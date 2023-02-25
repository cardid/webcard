/**
 * @file "native/src/json/json_value.c"
 * Simplified handling of the JSON data.
 */

#include "json/json.h"

/**************************************************************/

VOID
JsonValue_init(
  _Out_ JsonValue *value)
{
  value->type  = JSON_VALUE_TYPE__NULL;
  value->value = NULL;
}

/**************************************************************/

VOID
JsonValue_destroy(
  _Inout_ JsonValue *value)
{
  if (NULL != value->value)
  {
    switch (value->type)
    {
      case JSON_VALUE_TYPE__TRUE:
      case JSON_VALUE_TYPE__FALSE:
      case JSON_VALUE_TYPE__NULL:
      {
        return;
      }
      default:
      {
        /* Continue with dynamically allocated types */
      }
    }

    switch (value->type)
    {
      case JSON_VALUE_TYPE__STRING:
      {
        UTF8String_destroy(value->value);
        break;
      }
      case JSON_VALUE_TYPE__OBJECT:
      {
        JsonObject_destroy(value->value);
        break;
      }
      case JSON_VALUE_TYPE__ARRAY:
      {
        JsonArray_destroy(value->value);
        break;
      }
      default:
      {
        /* `JSON_VALUE_TYPE__NUMBER` is just a pointer to one `FLOAT` */
      }
    }

    free(value->value);
  }
}

/**************************************************************/

BOOL
JsonValue_copy(
  _Out_ JsonValue *destination,
  _In_ const JsonValue *source)
{
  size_t byteSize;

  /* Copy the JsonValue type field */

  destination->type = source->type;

  /* Check if the type points to a dynamic object */

  switch (source->type)
  {
    case JSON_VALUE_TYPE__STRING:
    {
      byteSize = sizeof(UTF8String);
      break;
    }
    case JSON_VALUE_TYPE__NUMBER:
    {
      byteSize = sizeof(FLOAT);
      break;
    }
    case JSON_VALUE_TYPE__OBJECT:
    {
      byteSize = sizeof(JsonObject);
      break;
    }
    case JSON_VALUE_TYPE__ARRAY:
    {
      byteSize = sizeof(JsonArray);
      break;
    }
    default:
    {
      destination->value = source->value;
      return TRUE;
    }
  }

  /* Allocate memory for an object to be cloned */

  destination->value = malloc(byteSize);
  if (NULL == destination->value) { return FALSE; }

  /* `destination->value` points to an UNINITIALIZED object of given type */

  switch (source->type)
  {
    case JSON_VALUE_TYPE__STRING:
    {
      return UTF8String_copy(destination->value, source->value);
    }
    case JSON_VALUE_TYPE__NUMBER:
    {
      ((FLOAT *) destination->value)[0] = ((FLOAT *) source->value)[0];
      return TRUE;
    }
    case JSON_VALUE_TYPE__OBJECT:
    {
      return JsonObject_copy(destination->value, source->value);
    }
    case JSON_VALUE_TYPE__ARRAY:
    {
      return JsonArray_copy(destination->value, source->value);
    }
    default:
    {
      return FALSE;
    }
  }
}

/**************************************************************/

BOOL
JsonValue_parseNumber(
  _Inout_ JsonValue *value,
  _Inout_ JsonByteStream *stream)
{
  char buf[256];
  char *buf_end = buf;
  char *buf_end_dummy;
  FLOAT number;

  /* 'A': expecting a minus ('-') or a digit ('0'-'9') */
  /* 'B': expecting a digit ('0'-'9') */
  /* 'C': started with zero, expecting a dot ('.') or stream end */
  /* 'D': expecting digits, dot ('.'), 'e', 'E' or stream end */
  /* 'E': started fraction, expecting a digit */
  /* 'F': fraction, expecting digits, 'e', 'E' or stream end */
  /* 'G': started exp, expecting digits, '-' or '+' */
  /* 'H': started exp, expecting expecting a digit */
  /* 'I': exp, expecing digitsor stream end */
  /* 'J': parsing success (whitespace, comma or closing brackets) */
  /* any other state: parsing error (unexpected character) */
  int parser_state = 'A';
  BYTE test_byte;

  /* In case of parsing failure, dynamic value stays NULL */

  value->value = NULL;

  while ((parser_state >= 'A') && (parser_state <= 'I'))
  {
    if ('A' != parser_state)
    {
      if (!Misc_pushToLocalBuffer(buf, &(buf_end), sizeof(buf), test_byte))
      {
        #if defined(_DEBUG)
        OSSpecific_writeDebugMessage(
          "JSON number parsing failed: buffer overflow");
        #endif

        return FALSE;
      }

      JsonByteStream_skip(stream, 1);
    }

    if (!JsonByteStream_peek(stream, &(test_byte)))
    {
      return FALSE;
    }

    switch (parser_state)
    {
      case 'A':
      {
        switch (test_byte)
        {
          case '0':
          {
            parser_state = 'C';
            break;
          }
          case '1': case '2': case '3': case '4':
          case '5': case '6': case '7': case '8': case '9':
          {
            parser_state = 'D';
            break;
          }
          case '-':
          {
            parser_state = 'B';
            break;
          }
          default:
          {
            parser_state = 0;
          }
        }

        break;
      }
      case 'B':
      {
        switch (test_byte)
        {
          case '0':
          {
            parser_state = 'C';
            break;
          }
          case '1': case '2': case '3': case '4':
          case '5': case '6': case '7': case '8': case '9':
          {
            parser_state = 'D';
            break;
          }
          default:
          {
            parser_state = 0;
          }
        }

        break;
      }
      case 'C':
      {
        switch (test_byte)
        {
          case '.':
          {
            parser_state = 'E';
            break;
          }
          case ' ': case '\r': case '\n': case '\t':
          case ',': case ']': case '}':
          {
            parser_state = 'J';
            break;
          }
          default:
          {
            parser_state = 0;
          }
        }

        break;
      }
      case 'D':
      {
        switch (test_byte)
        {
          case '0': case '1': case '2': case '3': case '4':
          case '5': case '6': case '7': case '8': case '9':
          {
            break;
          }
          case '.':
          {
            parser_state = 'E';
            break;
          }
          case 'E': case 'e':
          {
            parser_state = 'G';
            break;
          }
          case ' ': case '\r': case '\n': case '\t':
          case ',': case ']': case '}':
          {
            parser_state = 'J';
            break;
          }
          default:
          {
            parser_state = 0;
          }
        }

        break;
      }
      case 'E':
      {
        switch (test_byte)
        {
          case '0': case '1': case '2': case '3': case '4':
          case '5': case '6': case '7': case '8': case '9':
          {
            parser_state = 'F';
            break;
          }
          default:
          {
            parser_state = 0;
          }
        }

        break;
      }
      case 'F':
      {
        switch (test_byte)
        {
          case '0': case '1': case '2': case '3': case '4':
          case '5': case '6': case '7': case '8': case '9':
          {
            break;
          }
          case 'E': case 'e':
          {
            parser_state = 'G';
            break;
          }
          case ' ': case '\r': case '\n': case '\t':
          case ',': case ']': case '}':
          {
            parser_state = 'J';
            break;
          }
          default:
          {
            parser_state = 0;
          }
        }

        break;
      }
      case 'G':
      {
        switch (test_byte)
        {
          case '0': case '1': case '2': case '3': case '4':
          case '5': case '6': case '7': case '8': case '9':
          {
            parser_state = 'I';
            break;
          }
          case '-': case '+':
          {
            parser_state = 'H';
            break;
          }
          default:
          {
            parser_state = 0;
          }
        }

        break;
      }
      case 'H':
      {
        switch (test_byte)
        {
          case '0': case '1': case '2': case '3': case '4':
          case '5': case '6': case '7': case '8': case '9':
          {
            parser_state = 'I';
            break;
          }
          default:
          {
            parser_state = 0;
          }
        }

        break;
      }
      case 'I':
      {
        switch (test_byte)
        {
          case '0': case '1': case '2': case '3': case '4':
          case '5': case '6': case '7': case '8': case '9':
          {
            break;
          }
          case ' ': case '\r': case '\n': case '\t':
          case ',': case ']': case '}':
          {
            parser_state = 'J';
            break;
          }
          default:
          {
            parser_state = 0;
          }
        }

        break;
      }
    }
  }

  /* Parsing completed, check if any errors occurred */

  if ('J' != parser_state)
  {
    #if defined(_DEBUG)
    OSSpecific_writeDebugMessage(
      "JSON number parsing failed: unexpected character 0x%02X",
      test_byte);
    #endif

    return FALSE;
  }

  /* Otherwise, try to parse a FLOAT */

  buf_end[0] = 0;

  errno = 0;
  number = strtof(buf, &(buf_end_dummy));
  if ((0 != errno) || (buf_end_dummy != buf_end))
  {
    #if defined(_DEBUG)
    OSSpecific_writeDebugMessage(
      "{strtof} failed: errno=0x%08X",
      errno);
    #endif

    return FALSE;
  }

  value->value = malloc(sizeof(FLOAT));
  if (NULL == value->value)
  {
    return FALSE;
  }

  ((FLOAT *) value->value)[0] = number;
  return TRUE;
}

/**************************************************************/

BOOL
JsonValue_parse(
  _Outptr_result_maybenull_ JsonValue **const result,
  _In_ BOOL allocate,
  _Inout_ JsonByteStream *stream)
{
  BYTE test_bytes[2][4];

  if (allocate)
  {
    result[0] = malloc(sizeof(JsonValue));
    if (NULL == result[0]) { return FALSE; }
  }
  JsonValue_init(result[0]);

  if (!JsonByteStream_skipWhitespace(stream))
  {
    return FALSE;
  }

  if (!JsonByteStream_peek(stream, &(test_bytes[0][0])))
  {
    return FALSE;
  }

  switch (test_bytes[0][0])
  {
    case '"':
    {
      /* string value */
      result[0]->type = JSON_VALUE_TYPE__STRING;

      if (!JsonString_parse((UTF8String **) &(result[0]->value), TRUE, stream))
      {
        return FALSE;
      }

      break;
    }
    case '-':
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
    {
      /* number value */
      result[0]->type = JSON_VALUE_TYPE__NUMBER;

      if (!JsonValue_parseNumber(result[0], stream))
      {
        return FALSE;
      }

      break;
    }
    case '{':
    {
      /* object value */
      result[0]->type = JSON_VALUE_TYPE__OBJECT;

      if (!JsonObject_parse((JsonObject **) &(result[0]->value), TRUE, stream))
      {
        return FALSE;
      }

      break;
    }
    case '[':
    {
      /* array value */
      result[0]->type = JSON_VALUE_TYPE__ARRAY;

      if (!JsonArray_parse((JsonArray **) &(result[0]->value), stream))
      {
        return FALSE;
      }

      break;
    }
    case 't':
    {
      /* true value */
      result[0]->type = JSON_VALUE_TYPE__TRUE;

      if (!JsonByteStream_read(stream, test_bytes[0], 3))
      {
        return FALSE;
      }

      test_bytes[1][0] = 'r';
      test_bytes[1][1] = 'u';
      test_bytes[1][2] = 'e';

      if (0 != memcmp(test_bytes[0], test_bytes[1], 3))
      {
        #if defined(_DEBUG)
        OSSpecific_writeDebugMessage(
          "JSON value, parsing failed: expected literal 'true'");
        #endif

        return FALSE;
      }

      break;
    }
    case 'f':
    {
      /* false value */
      result[0]->type = JSON_VALUE_TYPE__FALSE;

      if (!JsonByteStream_read(stream, test_bytes[0], 4))
      {
        return FALSE;
      }

      test_bytes[1][0] = 'a';
      test_bytes[1][1] = 'l';
      test_bytes[1][2] = 's';
      test_bytes[1][3] = 'e';

      if (0 != memcmp(test_bytes[0], test_bytes[1], 4))
      {
        #if defined(_DEBUG)
        OSSpecific_writeDebugMessage(
          "JSON value, parsing failed: expected literal 'false'");
        #endif

        return FALSE;
      }

      break;
    }
    case 'n':
    {
      /* null value */
      result[0]->type = JSON_VALUE_TYPE__NULL;

      if (!JsonByteStream_read(stream, test_bytes[0], 3))
      {
        return FALSE;
      }

      test_bytes[1][0] = 'u';
      test_bytes[1][1] = 'l';
      test_bytes[1][2] = 'l';

      if (0 != memcmp(test_bytes[0], test_bytes[1], 3))
      {
        #if defined(_DEBUG)
        OSSpecific_writeDebugMessage(
          "JSON value, parsing failed: expected literal 'null'");
        #endif

        return FALSE;
      }

      break;
    }
    default:
    {
      return FALSE;
    }
  }

  if (!JsonByteStream_skipWhitespace(stream))
  {
    return FALSE;
  }

  if (!JsonByteStream_peek(stream, &(test_bytes[0][0])))
  {
    return FALSE;
  }

  switch (test_bytes[0][0])
  {
    case ' ': case '\r': case '\n': case '\t':
    case ',': case ']': case '}':
    {
      return TRUE;
    }
    default:
    {
      #if defined(_DEBUG)
      OSSpecific_writeDebugMessage(
        "JSON value, parsing failed: unexpected character 0x%02X",
        test_bytes[0][0]);
      #endif

      return FALSE;
    }
  }
}

/**************************************************************/

BOOL
JsonValue_toString(
  _In_ const JsonValue *value,
  _Inout_ UTF8String *output)
{
  char number_buffer[64];
  FLOAT value_number;

  switch (value->type)
  {
    case JSON_VALUE_TYPE__STRING:
    {
      return JsonString_toString(value->value, output);
    }
    case JSON_VALUE_TYPE__NUMBER:
    {
      value_number = ((FLOAT *) value->value)[0];
      snprintf(number_buffer, 64, "%.f", value_number);
      return UTF8String_pushText(output, number_buffer, 0);
    }
    case JSON_VALUE_TYPE__OBJECT:
    {
      return JsonObject_toString(value->value, output);
    }
    case JSON_VALUE_TYPE__ARRAY:
    {
      return JsonArray_toString(value->value, output);
    }
    case JSON_VALUE_TYPE__TRUE:
    {
      return UTF8String_pushText(output, "true", 4);
    }
    case JSON_VALUE_TYPE__FALSE:
    {
      return UTF8String_pushText(output, "false", 5);
    }
    default:
    {
      return UTF8String_pushText(output, "null", 4);
    }
  }
}

/**************************************************************/
