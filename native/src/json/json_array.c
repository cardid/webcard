/**
 * @file "native/src/json/json_array.c"
 * Simplified handling of the JSON data.
 */

#include "json/json.h"

/**************************************************************/

VOID
JsonArray_init(
  _Out_ JsonArray *array)
{
  array->count    = 0;
  array->capacity = 0;
  array->values   = NULL;
}

/**************************************************************/

VOID
JsonArray_destroy(
  _Inout_ JsonArray *array)
{
  if (NULL != array->values)
  {
    for (size_t i = 0; i < array->count; i++)
    {
      JsonValue_destroy(&(array->values[i]));
    }

    free(array->values);
  }
}

/**************************************************************/

BOOL
JsonArray_copy(
  _Out_ JsonArray *destination,
  _In_ const JsonArray *source)
{
  if (0 == source->count)
  {
    JsonArray_init(destination);
    return TRUE;
  }

  const size_t capacity = Misc_nextPowerOfTwo(source->count - 1);
  const size_t byteSize = sizeof(JsonValue) * capacity;

  destination->count = 0;
  destination->capacity = capacity;

  destination->values = malloc(byteSize);
  if (NULL == destination->values) { return FALSE; }

  for (size_t i = 0; i < source->count; i++)
  {
    BOOL test_bool = JsonValue_copy(&(destination->values[i]), &(source->values[i]));
    destination->count += 1;
    if (!test_bool) { return FALSE; }
  }

  return TRUE;
}

/**************************************************************/

BOOL
JsonArray_append(
  _Inout_ JsonArray *array,
  _In_ const JsonValue *value)
{
  size_t newCapacity = (array->count + 1);

  if (newCapacity > array->capacity)
  {
    newCapacity = Misc_nextPowerOfTwo(newCapacity - 1);

    const size_t newByteSize = sizeof(JsonValue) * newCapacity;
    JsonValue *newValues = realloc(array->values, newByteSize);
    if (NULL == newValues) { return FALSE; }

    array->values = newValues;
    array->capacity = newCapacity;
  }

  BOOL test_bool = JsonValue_copy(
    &(array->values[array->count]),
    value);

  array->count += 1;
  return test_bool;
}

/**************************************************************/

BOOL
JsonArray_parse(
  _Outptr_result_maybenull_ JsonArray **const result,
  _Inout_ JsonByteStream *stream)
{
  BOOL test_bool;
  BYTE test_byte;
  JsonValue json_value;
  JsonValue *json_value_ptr = &(json_value);

  result[0] = malloc(sizeof(JsonArray));
  if (NULL == result[0]) { return FALSE; }
  JsonArray_init(result[0]);

  /* Array starts with '[' */

  if (!JsonByteStream_read(stream, &(test_byte), 1) || ('[' != test_byte))
  {
    #if defined(_DEBUG)
    OSSpecific_writeDebugMessage(
      "JSON array, parsing failed: expected an opening square bracket");
    #endif

    return FALSE;
  }

  if (!JsonByteStream_skipWhitespace(stream))
  {
    return FALSE;
  }

  while (JsonByteStream_peek(stream, &(test_byte)))
  {
    if (']' == test_byte)  /* Array ends with ']' */
    {
      JsonByteStream_skip(stream, 1);
      return TRUE;
    }

    if (',' == test_byte)
    {
      if (0 == result[0]->count)
      {
        #if defined(_DEBUG)
        OSSpecific_writeDebugMessage(
          "JSON array, parsing failed: unexpected comma");
        #endif

        return FALSE;
      }

      JsonByteStream_skip(stream, 1);
    }
    else
    {
      if (0 != result[0]->count)
      {
        #if defined(_DEBUG)
        OSSpecific_writeDebugMessage(
          "JSON value, parsing failed: expected a comma");
        #endif

        return FALSE;
      }
    }

    test_bool = JsonValue_parse(&(json_value_ptr), FALSE, stream);

    if (test_bool)
    {
      test_bool = JsonArray_append(result[0], json_value_ptr);
    }

    JsonValue_destroy(json_value_ptr);

    if (!test_bool)
    {
      return FALSE;
    }
  }

  return FALSE;
}

/**************************************************************/

BOOL
JsonArray_toString(
  _In_ const JsonArray *array,
  _Inout_ UTF8String *output)
{
  if (!UTF8String_pushByte(output, '['))
  {
    return FALSE;
  }

  for (size_t i = 0; i < array->count; i++)
  {
    if (!JsonValue_toString(&(array->values[i]), output))
    {
      return FALSE;
    }

    if (i < (array->count - 1))
    {
      if (!UTF8String_pushByte(output, ','))
      {
        return FALSE;
      }
    }
  }

  return UTF8String_pushByte(output, ']');
}

/**************************************************************/
