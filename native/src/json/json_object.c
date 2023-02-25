/**
 * @file "native/src/json/json_object.c"
 * Simplified handling of the JSON data.
 */

#include "json/json.h"

/**************************************************************/

VOID
JsonObject_init(
  _Out_ JsonObject *object)
{
  object->count    = 0;
  object->capacity = 0;
  object->pairs    = NULL;
}

/**************************************************************/

VOID
JsonObject_destroy(
  _Inout_ JsonObject *object)
{
  if (NULL != object->pairs)
  {
    for (size_t i = 0; i < object->count; i++)
    {
      JsonPair_destroy(&(object->pairs[i]));
    }

    free(object->pairs);
  }
}

/**************************************************************/

BOOL
JsonObject_copy(
  _Out_ JsonObject *destination,
  _In_ const JsonObject *source)
{
  if (0 == source->count)
  {
    JsonObject_init(destination);
    return TRUE;
  }

  const size_t capacity = Misc_nextPowerOfTwo(source->count - 1);
  const size_t byteSize = sizeof(JsonPair) * capacity;

  destination->count = 0;
  destination->capacity = capacity;

  destination->pairs = malloc(byteSize);
  if (NULL == destination->pairs) { return FALSE; }

  for (size_t i = 0; i < source->count; i++)
  {
    BOOL test_bool = JsonPair_copy(&(destination->pairs[i]), &(source->pairs[i]));
    destination->count += 1;
    if (!test_bool) { return FALSE; }
  }

  return TRUE;
}

/**************************************************************/

BOOL
JsonObject_appendPair(
  _Inout_ JsonObject *object,
  _In_ const JsonPair *pair)
{
  size_t newCapacity = (object->count + 1);

  if (newCapacity > object->capacity)
  {
    newCapacity = Misc_nextPowerOfTwo(newCapacity - 1);

    const size_t newByteSize = sizeof(JsonPair) * newCapacity;
    JsonPair *newPairs = realloc(object->pairs, newByteSize);
    if (NULL == newPairs) { return FALSE; }

    object->pairs = newPairs;
    object->capacity = newCapacity;
  }

  BOOL test_bool = JsonPair_copy(
    &(object->pairs[object->count]),
    pair);

  object->count += 1;
  return test_bool;
}

/**************************************************************/

BOOL
JsonObject_appendKeyValue(
  _Inout_ JsonObject *object,
  _In_ LPCSTR key,
  _In_ const JsonValue *value)
{
  JsonPair json_pair;

  UTF8String_makeTemporary(&(json_pair.key), key);

  json_pair.value = value[0];

  return JsonObject_appendPair(object, &(json_pair));
}

/**************************************************************/

BOOL
JsonObject_parse(
  _Outptr_result_maybenull_ JsonObject **const result,
  _In_ BOOL allocate,
  _Inout_ JsonByteStream *stream)
{
  BOOL test_bool;
  BYTE test_byte;
  JsonPair json_pair;
  JsonPair *json_pair_ptr = &(json_pair);

  if (allocate)
  {
    result[0] = malloc(sizeof(JsonObject));
    if (NULL == result[0]) { return FALSE; }
  }
  JsonObject_init(result[0]);

  /* Object starts with '{' */

  if (!JsonByteStream_read(stream, &(test_byte), 1) || ('{' != test_byte))
  {
    #if defined(_DEBUG)
    OSSpecific_writeDebugMessage(
      "JSON object, parsing failed: expected an opening curly bracket");
    #endif

    return FALSE;
  }

  if (!JsonByteStream_skipWhitespace(stream))
  {
    return FALSE;
  }

  while (JsonByteStream_peek(stream, &(test_byte)))
  {
    if ('}' == test_byte)  /* Object ends with '}' */
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
          "JSON object, parsing failed: unexpected comma");
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
          "JSON object, parsing failed: expected a comma");
        #endif

        return FALSE;
      }
    }

    test_bool = JsonPair_parse(&(json_pair_ptr), FALSE, stream);

    if (test_bool)
    {
      test_bool = JsonObject_appendPair(result[0], json_pair_ptr);
    }

    JsonPair_destroy(json_pair_ptr);

    if (!test_bool)
    {
      return FALSE;
    }
  }

  return FALSE;
}

/**************************************************************/

BOOL
JsonObject_toString(
  _In_ const JsonObject *object,
  _Inout_ UTF8String *output)
{
  if (!UTF8String_pushByte(output, '{'))
  {
    return FALSE;
  }

  for (size_t i = 0; i < object->count; i++)
  {
    if (!JsonPair_toString(&(object->pairs[i]), output))
    {
      return FALSE;
    }

    if (i < (object->count - 1))
    {
      if (!UTF8String_pushByte(output, ','))
      {
        return FALSE;
      }
    }
  }

  return UTF8String_pushByte(output, '}');
}

/**************************************************************/

BOOL
JsonObject_getValue(
  _In_ const JsonObject *object,
  _Out_ JsonValue *result,
  _In_ LPCSTR key)
{
  for (size_t i = 0; i < object->count; i++)
  {
    if (UTF8String_matches(&(object->pairs[i].key), key))
    {
      result[0] = object->pairs[i].value;
      return TRUE;
    }
  }

  return FALSE;
}

/**************************************************************/
