/**
 * @file "native/src/json/json_pair.c"
 * Simplified handling of the JSON data.
 */

#include "json/json.h"

/**************************************************************/

VOID
JsonPair_init(
  _Out_ JsonPair *pair)
{
  UTF8String_init(&(pair->key));
  JsonValue_init(&(pair->value));
}

/**************************************************************/

VOID
JsonPair_destroy(
  _Inout_ JsonPair *pair)
{
  UTF8String_destroy(&(pair->key));
  JsonValue_destroy(&(pair->value));
}

/**************************************************************/

BOOL
JsonPair_copy(
  _Out_ JsonPair *destination,
  _In_ const JsonPair *source)
{
  JsonPair_init(destination);

  if (!UTF8String_copy(&(destination->key), &(source->key)))
  {
    return FALSE;
  }

  return JsonValue_copy(&(destination->value), &(source->value));
}

/**************************************************************/

BOOL
JsonPair_parse(
  _Outptr_result_maybenull_ JsonPair **const result,
  _In_ BOOL allocate,
  _Inout_ JsonByteStream *stream)
{
  BYTE test_byte;

  if (allocate)
  {
    result[0] = malloc(sizeof(JsonPair));
    if (NULL == result[0]) { return FALSE; }
  }
  JsonPair_init(result[0]);

  if (!JsonByteStream_skipWhitespace(stream))
  {
    return FALSE;
  }

  UTF8String *key_pointer = &(result[0]->key);
  if (!JsonString_parse(&(key_pointer), FALSE, stream))
  {
    return FALSE;
  }

  if (!JsonByteStream_skipWhitespace(stream))
  {
    return FALSE;
  }

  if (!JsonByteStream_read(stream, &(test_byte), 1) || (':' != test_byte))
  {
    return FALSE;
  }

  JsonValue *value_pointer = &(result[0]->value);
  return JsonValue_parse(&(value_pointer), FALSE, stream);
}

/**************************************************************/

BOOL
JsonPair_toString(
  _In_ const JsonPair *pair,
  _Inout_ UTF8String *output)
{
  if (!JsonString_toString(&(pair->key), output))
  {
    return FALSE;
  }

  if (!UTF8String_pushByte(output, ':'))
  {
    return FALSE;
  }

  return JsonValue_toString(&(pair->value), output);
}

/**************************************************************/
