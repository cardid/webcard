/**
 * @file "native/src/utf.c"
 * Unicode Transformation Format text (UTF-8 and UTF-16)
 */

#include "utf/utf.h"
#include "misc/misc.h"

/**************************************************************/

VOID
UTF8String_init(
  _Out_ UTF8String *string)
{
  string->length   = 0;
  string->capacity = 0;
  string->text     = NULL;
}

/**************************************************************/

VOID
UTF8String_destroy(
  _Inout_ UTF8String *string)
{
  if (NULL != string->text)
  {
    free(string->text);
  }
}

/**************************************************************/

/**
 * @brief A private method for `UTF8String` object. Checks if the string
 * can hold `n` more characters, and if not, reallocates text buffer.
 *
 * @param[in,out] string Reference to a VALID and CONSTANT `UTF8String` object.
 * @param[in] additionalLength How many more characters you want to fit
 * into the dynamically allocated text buffer.
 * @return `TRUE` on success, `FALSE` on memory allocation failure.
 */
BOOL
UTF8String_assertCapacity(
  _Inout_ UTF8String *string,
  _In_ const size_t additionalLength)
{
  size_t newCapacity = string->length + additionalLength + 1;

  if (newCapacity > string->capacity)
  {
    newCapacity = Misc_nextPowerOfTwo(newCapacity - 1);

    const size_t newByteSize = sizeof(BYTE) * newCapacity;
    LPBYTE newText = realloc(string->text, newByteSize);
    if (NULL == newText) { return FALSE; }

    string->text = newText;
    string->capacity = newCapacity;
  }

  return TRUE;
}

/**************************************************************/

BOOL
UTF8String_copy(
  _Out_ UTF8String *destination,
  _In_ const UTF8String *source)
{
  if (0 == source->length)
  {
    UTF8String_init(destination);
    return TRUE;
  }

  const size_t capacity = Misc_nextPowerOfTwo(source->length);
  const size_t minByteSize = sizeof(BYTE) * (1 + source->length);
  const size_t maxByteSize = sizeof(BYTE) * capacity;

  destination->length = source->length;
  destination->capacity = capacity;

  destination->text = malloc(maxByteSize);
  if (NULL == destination->text) { return FALSE; }

  memcpy(destination->text, source->text, minByteSize);
  return TRUE;
}

/**************************************************************/

VOID
UTF8String_makeTemporary(
  _Out_ UTF8String *string,
  _In_ LPCSTR text)
{
  string->length = strlen(text);
  string->capacity = Misc_nextPowerOfTwo(string->length);
  string->text = (LPBYTE) text;
}

/**************************************************************/

BOOL
UTF8String_pushByte(
  _Inout_ UTF8String *string,
  _In_ const BYTE byte)
{
  if (!UTF8String_assertCapacity(string, 1))
  {
    return FALSE;
  }

  string->text[string->length] = byte;
  string->length += 1;
  string->text[string->length] = '\0';

  return TRUE;
}

/**************************************************************/

BOOL
UTF8String_pushBytesAsHex(
  _Inout_ UTF8String *string,
  _In_ size_t byteArraySize,
  _In_ const BYTE *bytes)
{
  if (!UTF8String_assertCapacity(string, (2 * byteArraySize)))
  {
    return FALSE;
  }

  LPBYTE text = string->text;

  for (size_t i = 0; i < byteArraySize; i++)
  {
    BYTE next_byte = bytes[i];

    for (size_t j = 0; j < 2; j++)
    {
      BYTE nibble = next_byte & 0x0F;
      next_byte >>= 4;

      BYTE hexcode = (nibble < 0x0A) ?
        ('0' + nibble) :
        ('A' + nibble - 0x0A);
      text[string->length + 1 - j] = hexcode;
    }

    string->length += 2;
  }

  text[string->length] = '\0';
  return TRUE;
}

/**************************************************************/

BOOL
UTF8String_hexToByteArray(
  _In_ const UTF8String *string,
  _Out_ size_t *byteArraySizeRef,
  _Outptr_result_maybenull_ BYTE **const result)
{
  const size_t output_size = string->length / 2;

  result[0] = malloc(sizeof(BYTE) * output_size);
  if (NULL == result[0]) { return FALSE; }

  byteArraySizeRef[0] = output_size;

  const BYTE *text = string->text;

  for (size_t i = 0; i < output_size; i++)
  {
    BYTE next_byte = 0;

    for (size_t j = 0; j < 2; j++)
    {
      BYTE nibble;
      BYTE codepoint = text[0];

      if (0 != (0xFF00 & codepoint))
      {
        return FALSE;
      }
      else if ((codepoint >= '0') && (codepoint <= '9'))
      {
        nibble = codepoint - '0';
      }
      else if ((codepoint >= 'A') && (codepoint <= 'F'))
      {
        nibble = codepoint - 'A' + 0x0A;
      }
      else if ((codepoint >= 'a') && (codepoint <= 'f'))
      {
        nibble = codepoint - 'a' + 0x0A;
      }
      else
      {
        return FALSE;
      }

      text++;
      next_byte = (next_byte << 4) | nibble;
    }

    result[0][i] = next_byte;
  }

  return TRUE;
}

/**************************************************************/

BOOL
UTF8String_pushText(
  _Inout_ UTF8String *string,
  _In_ LPCSTR rightText,
  _In_ size_t rightTextLength)
{
  if (0 == rightTextLength)
  {
    rightTextLength = strlen(rightText);

    if (0 == rightTextLength)
    {
      return TRUE;
    }
  }

  if (!UTF8String_assertCapacity(string, rightTextLength))
  {
    return FALSE;
  }

  const size_t byteSize = sizeof(BYTE) * rightTextLength;

  memcpy(&(string->text[string->length]), rightText, byteSize);
  string->length += rightTextLength;
  string->text[string->length] = '\0';

  return TRUE;
}

/**************************************************************/

BOOL
UTF8String_matches(
  _In_ const UTF8String *string,
  _In_z_ LPCSTR testedText)
{
  size_t i;

  for (i = 0; i < string->length; i++)
  {
    if (('\0' == testedText[i]) || (testedText[i] != string->text[i]))
    {
      return FALSE;
    }
  }

  return ('\0' == testedText[i]);
}

/**************************************************************/

BOOL
UTF8String_writeToStandardOutput(
  _In_ const UTF8String *string)
{
  os_specific_stream_t stdout_stream;

  #if defined(_WIN32)
  {
    stdout_stream = GetStdHandle(STD_OUTPUT_HANDLE);

    if (INVALID_HANDLE_VALUE == stdout_stream)
    {
      return FALSE;
    }
  }
  #elif defined(__linux__) || defined(__APPLE__)
  {
    stdout_stream = STDOUT_FILENO;
  }
  #endif

  uint32_t outgoing_length = string->length;

  BOOL test_bool = OSSpecific_writeBytesToStream(
    stdout_stream,
    &(outgoing_length),
    sizeof(uint32_t));

  if (!test_bool) { return FALSE; }

  return OSSpecific_writeBytesToStream(
    stdout_stream,
    string->text,
    string->length);
}

/**************************************************************/

VOID
UTF16String_init(
  _Out_ UTF16String *string)
{
  string->length   = 0;
  string->capacity = 0;
  string->text     = NULL;
}

/**************************************************************/

VOID
UTF16String_destroy(
  _Inout_ UTF16String *string)
{
  if (NULL != string->text)
  {
    free(string->text);
  }
}

/**************************************************************/

/**
 * @brief A private method for `UTF16String` object. Checks if the string
 * can hold `n` more characters, and if not, reallocates text buffer.
 *
 * @param[in,out] string Reference to a VALID and CONSTANT `UTF16String` object.
 * @param[in] additionalLength How many more characters you want to fit
 * into the dynamically allocated text buffer.
 * @return `TRUE` on success, `FALSE` on memory allocation failure.
 */
BOOL
UTF16String_assertCapacity(
  _Inout_ UTF16String *string,
  _In_ const size_t additionalLength)
{
  size_t newCapacity = string->length + additionalLength + 1;

  if (newCapacity > string->capacity)
  {
    newCapacity = Misc_nextPowerOfTwo(newCapacity - 1);

    const size_t newByteSize = sizeof(WCHAR) * newCapacity;
    LPWSTR newText = realloc(string->text, newByteSize);
    if (NULL == newText) { return FALSE; }

    string->text = newText;
    string->capacity = newCapacity;
  }

  return TRUE;
}

/**************************************************************/

BOOL
UTF16String_copy(
  _Out_ UTF16String *destination,
  _In_ const UTF16String *source)
{
  if (0 == source->length)
  {
    UTF16String_init(destination);
    return TRUE;
  }

  const size_t capacity = Misc_nextPowerOfTwo(source->length);
  const size_t minByteSize = sizeof(WCHAR) * (1 + source->length);
  const size_t maxByteSize = sizeof(WCHAR) * capacity;

  destination->length = source->length;
  destination->capacity = capacity;

  destination->text = malloc(maxByteSize);
  if (NULL == destination->text) { return FALSE; }

  memcpy(destination->text, source->text, minByteSize);
  return TRUE;
}

/**************************************************************/

BOOL
UTF16String_pushWideChar(
  _Inout_ UTF16String *string,
  _In_ const WCHAR code)
{
  if (!UTF16String_assertCapacity(string, 1))
  {
    return FALSE;
  }

  string->text[string->length] = code;
  string->length += 1;
  string->text[string->length] = '\0';

  return TRUE;
}

/**************************************************************/

BOOL
UTF16String_pushBytesAsHex(
  _Inout_ UTF16String *string,
  _In_ size_t byteArraySize,
  _In_ const BYTE *bytes)
{
  if (!UTF16String_assertCapacity(string, (2 * byteArraySize)))
  {
    return FALSE;
  }

  LPWSTR text = string->text;

  for (size_t i = 0; i < byteArraySize; i++)
  {
    BYTE next_byte = bytes[i];

    for (size_t j = 0; j < 2; j++)
    {
      BYTE nibble = next_byte & 0x0F;
      next_byte >>= 4;

      WCHAR hexcode = (nibble < 0x0A) ?
        ('0' + nibble) :
        ('A' + nibble - 0x0A);
      text[string->length + 1 - j] = hexcode;
    }

    string->length += 2;
  }

  text[string->length] = '\0';
  return TRUE;
}

/**************************************************************/

BOOL
UTF16String_hexToByteArray(
  _In_ const UTF16String *string,
  _Out_ size_t *byteArraySizeRef,
  _Outptr_result_maybenull_ BYTE **const result)
{
  const size_t output_size = string->length / 2;

  result[0] = malloc(sizeof(BYTE) * output_size);
  if (NULL == result[0]) { return FALSE; }

  byteArraySizeRef[0] = output_size;

  LPCWSTR text = string->text;

  for (size_t i = 0; i < output_size; i++)
  {
    BYTE next_byte = 0;

    for (size_t j = 0; j < 2; j++)
    {
      BYTE nibble;
      WCHAR codepoint = text[0];

      if (0 != (0xFF00 & codepoint))
      {
        return FALSE;
      }
      else if ((codepoint >= '0') && (codepoint <= '9'))
      {
        nibble = codepoint - '0';
      }
      else if ((codepoint >= 'A') && (codepoint <= 'F'))
      {
        nibble = codepoint - 'A' + 0x0A;
      }
      else if ((codepoint >= 'a') && (codepoint <= 'f'))
      {
        nibble = codepoint - 'a' + 0x0A;
      }
      else
      {
        return FALSE;
      }

      text++;
      next_byte = (next_byte << 4) | nibble;
    }

    result[0][i] = next_byte;
  }

  return TRUE;
}

/**************************************************************/

BOOL
UTF16String_pushText(
  _Inout_ UTF16String *string,
  _In_ LPCWSTR rightText,
  _In_ size_t rightTextLength)
{
  if (0 == rightTextLength)
  {
    rightTextLength = OSSpecific_wideStrLen(rightText);

    if (0 == rightTextLength)
    {
      return TRUE;
    }
  }

  if (!UTF16String_assertCapacity(string, rightTextLength))
  {
    return FALSE;
  }

  const size_t byteSize = sizeof(WCHAR) * rightTextLength;

  memcpy(&(string->text[string->length]), rightText, byteSize);
  string->length += rightTextLength;
  string->text[string->length] = '\0';

  return TRUE;
}

/**************************************************************/

BOOL
UTF16String_matches(
  _In_ const UTF16String *string,
  _In_z_ LPCWSTR testedText)
{
  size_t i;

  for (i = 0; i < string->length; i++)
  {
    if (('\0' == testedText[i]) || (testedText[i] != string->text[i]))
    {
      return FALSE;
    }
  }

  return ('\0' == testedText[i]);
}

/**************************************************************/

BOOL
UTF8_validateTransformation(
  _In_ const BYTE *bytes,
  _Inout_ size_t *lengthRef,
  _Out_opt_ uint32_t *codePointRef)
{
  uint32_t codepoint;

  if (lengthRef[0] < 2)
  {
    return FALSE;
  }

  if (0xF0 == (0xF8 & bytes[0]))
  {
    /* bitmask:    11111000 */
    /* first byte: 11110xxx */

    if (lengthRef[0] < 4)
    {
      return FALSE;
    }

    lengthRef[0] = 4;

    /* bitmask: 00000111 */
    codepoint = bytes[0] & 0x07;
  }
  else if (0xE0 == (0xF0 & bytes[0]))
  {
    /* bitmask:    11110000 */
    /* first byte: 1110xxxx */

    if (lengthRef[0] < 3)
    {
      return FALSE;
    }

    lengthRef[0] = 3;

    /* bitmask: 00001111 */
    codepoint = bytes[0] & 0x0F;
  }
  else if (0xC0 == (0xE0 & bytes[0]))
  {
    /* bitmask:    11100000 */
    /* first byte: 110xxxxx */

    lengthRef[0] = 2;

    /* bitmask: 00011111 */
    codepoint = bytes[0] & 0x1F;
  }
  else
  {
    return FALSE;
  }

  /* Append the bits from subsequent bytes */

  for (size_t i = 0; i < lengthRef[0]; i++)
  {
    /* bitmask:          11000000 */
    /* subsequent bytes: 10xxxxxx */

    if (0x80 != (0xC0 & bytes[1 + i]))
    {
      return FALSE;
    }

    /* bitmask: 00111111 */
    codepoint = (codepoint << 6) | (bytes[1 + i] & 0x3F);
  }

  /* Validate the "minimum number of bytes" rule */

  if ((4 == lengthRef[0]) && (codepoint < 0x00010000))
  {
    return FALSE;
  }
  else if ((3 == lengthRef[0]) && (codepoint < 0x00000800))
  {
    return FALSE;
  }
  else if (codepoint < 0x00000080)
  {
    return FALSE;
  }

  /* Store the decoded codepoint if needed */

  if (NULL != codePointRef)
  {
    codePointRef[0] = codepoint;
  }

  return TRUE;
}

/**************************************************************/

BOOL
UTF16String_toUTF8(
  _In_ const UTF16String *string,
  _Inout_ UTF8String *output)
{
  for (size_t i = 0; i < string->length; i++)
  {
    WCHAR codepoint = string->text[i];

    if (0 != (0xFF80 & codepoint))
    {
      /** TODO: utf16 to utf8 conversion */
      /** TODO: safe check (minimal possible value range) */
      return FALSE;
    }

    if (!UTF8String_pushByte(output, (BYTE) codepoint))
    {
      return FALSE;
    }
  }

  return TRUE;
}

/**************************************************************/

