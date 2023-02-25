/**
 * @file "native/src/json/json_string.c"
 * Simplified handling of the JSON data.
 */

#include "json/json.h"

/**************************************************************/

BOOL
JsonString_parse(
  _Outptr_result_maybenull_ UTF8String **const result,
  _In_ const BOOL allocate,
  _Inout_ JsonByteStream *stream)
{
  BYTE test_byte;
  BOOL test_bool;

  if (allocate)
  {
    result[0] = malloc(sizeof(UTF8String));
    if (NULL == result[0]) { return FALSE; }
  }
  UTF8String_init(result[0]);

  /* String starts with '"' */

  if (!JsonByteStream_read(stream, &(test_byte), 1) || ('"' != test_byte))
  {
    #if defined(_DEBUG)
    OSSpecific_writeDebugMessage(
      "JSON string, parsing failed: expected an opening quote");
    #endif

    return FALSE;
  }

  while (JsonByteStream_read(stream, &(test_byte), 1))
  {
    if (test_byte < ' ')
    {
      #if defined(_DEBUG)
      OSSpecific_writeDebugMessage(
        "JSON string, parsing failed: unexpected character 0x%02X",
        test_byte);
      #endif

      return FALSE;
    }
    else if (0x80 & test_byte)
    {
      /* Multibyte codepoint */

      size_t remaining_bytes = stream->tail_length;

      test_bool = UTF8_validateTransformation(
        &(stream->tail[-1]),
        &(remaining_bytes),
        NULL);

      if (!test_bool)
      {
        #if defined(_DEBUG)
        OSSpecific_writeDebugMessage(
          "JsonString::parse(): not a valid UTF-8 representation!");
        #endif

        return FALSE;
      }

      test_bool = UTF8String_pushText(
        result[0],
        (LPCSTR) &(stream->tail[-1]),
        remaining_bytes);

      JsonByteStream_skip(stream, (remaining_bytes - 1));

      if (!test_bool) { return FALSE; }
    }
    else if ('"' == test_byte)
    {
      /* String ends with '"' */
      return TRUE;
    }
    else
    {
      if ('\\' == test_byte)
      {
        if (!JsonByteStream_read(stream, &(test_byte), 1))
        {
          return FALSE;
        }

        switch (test_byte)
        {
          case '"':
          case '\\':
          case '/':
            break;
          case 'b':
            test_byte = '\b';
            break;
          case 'f':
            test_byte = '\f';
            break;
          case 'n':
            test_byte = '\n';
            break;
          case 'r':
            test_byte = '\r';
            break;
          case 't':
            test_byte = '\t';
            break;
          default:
            #if defined(_DEBUG)
            OSSpecific_writeDebugMessage(
              "JSON stream, parsing failed: unknown escape sequence 0x%02X",
              test_byte);
            #endif
            return FALSE;
        }
      }

      if (!UTF8String_pushByte(result[0], test_byte))
      {
        return FALSE;
      }
    }
  }

  return FALSE;
}

/**************************************************************/

BOOL
JsonString_toString(
  _In_ const UTF8String *string,
  _Inout_ UTF8String *output)
{
  BYTE test_bytes[2] = { 0x00 };
  BOOL test_bool;

  if (!UTF8String_pushByte(output, '"'))
  {
    return FALSE;
  }

  for (size_t i = 0; i < string->length;  i++)
  {
    test_bytes[1] = string->text[i];

    test_bool = TRUE;

    switch (test_bytes[1])
    {
      case '"':
      case '\\':
      {
        break;
      }
      case '\b':
      {
        test_bytes[1] = 'b';
        break;
      }
      case '\f':
      {
        test_bytes[1] = 'f';
        break;
      }
      case '\n':
      {
        test_bytes[1] = 'n';
        break;
      }
      case '\r':
      {
        test_bytes[1] = 'r';
        break;
      }
      case '\t':
      {
        test_bytes[1] = 't';
        break;
      }
      default:
      {
        test_bool = FALSE;
      }
    }

    if (test_bool)
    {
      /* Character escaped in a short form */

      if (!UTF8String_pushByte(output, '\\'))
      {
        return FALSE;
      }

      if (!UTF8String_pushByte(output, test_bytes[1]))
      {
        return FALSE;
      }
    }
    else if (0x80 & test_bytes[1])
    {
      /* Multibyte codepoint */

      size_t remaining_bytes = (string->length - i);

      test_bool = UTF8_validateTransformation(
        &(string->text[i]),
        &(remaining_bytes),
        NULL);

      if (!test_bool)
      {
        #if defined(_DEBUG)
        OSSpecific_writeDebugMessage(
          "JsonString::toString(): not a valid UTF-8 representation!");
        #endif

        return FALSE;
      }

      test_bool = UTF8String_pushText(
        output,
        (LPCSTR) &(string->text[i]),
        remaining_bytes);

      if (!test_bool) { return FALSE; }
    }
    else if (test_bytes[1] < ' ')
    {
      /* Control character escaped in a long form */

      if (!UTF8String_pushText(output, "\\u", 2))
      {
        return FALSE;
      }

      if (!UTF8String_pushBytesAsHex(output, 2, test_bytes))
      {
        return FALSE;
      }
    }
    else
    {
      /* Regular ASCII printable character */

      if (!UTF8String_pushByte(output, test_bytes[1]))
      {
        return FALSE;
      }
    }
  }

  return UTF8String_pushByte(output, '"');
}

/**************************************************************/
