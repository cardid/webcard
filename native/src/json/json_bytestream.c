/**
 * @file "native/src/json/json_bytestream.c"
 * Simplified handling of the JSON data.
 */

#include "json/json.h"

/**************************************************************/

int
JsonByteStream_loadFromStandardInput(
  _Out_ JsonByteStream *stream)
{
  BOOL test_bool;
  os_specific_stream_t stdin_stream;
  uint32_t pipe_length;
  uint32_t json_length;

  /* Get Standard Input stream identifier */

  #if defined(_WIN32)
  {
    stdin_stream = GetStdHandle(STD_INPUT_HANDLE);

    if (INVALID_HANDLE_VALUE == stdin_stream)
    {
      return JSON_STREAM_STATUS__NO_MORE;
    }
  }
  #elif defined(__linux__) || defined(__APPLE__)
  {
    stdin_stream = STDIN_FILENO;
  }
  #endif

  /* Check if the pipe is not broken */
  /* and if any message is pending to be read */

  test_bool = OSSpecific_peekStream(
    stdin_stream,
    &(pipe_length));

  if (!test_bool)
  {
    return JSON_STREAM_STATUS__NO_MORE;
  }

  if (0 == pipe_length)
  {
    /* Go back to the program's main loop */
    return JSON_STREAM_STATUS__EMPTY;
  }

  /* At least one byte available: in theory this could stop the thread, but */
  /* usually STDIN is flushed only after sending a complete stringified JSON*/

  /* Read the first four bytes (INT32) */
  /* ("native byte order", no need to check for endianness) */

  test_bool = OSSpecific_readBytesFromStream(
    stdin_stream,
    &(json_length),
    sizeof(uint32_t));

  if (!test_bool)
  {
    return JSON_STREAM_STATUS__NO_MORE;
  }

  #if defined(_DEBUG)
    OSSpecific_writeDebugMessage(
      "{JsonByteStream} 0x%04X/0x%04X bytes on STDIN",
      json_length,
      pipe_length);
  #endif

  /* Validate given text length */

  test_bool = (json_length <= (pipe_length - sizeof(uint32_t)));

  if ((0 == json_length) || (UINT32_MAX == json_length) || (!test_bool))
  {
    if (sizeof(uint32_t) == pipe_length)
    {
      /* In rare cases, only 4 bytes (JSON size info) were streamed first */

      do
      {
        test_bool = OSSpecific_peekStream(
          stdin_stream,
          &(pipe_length));

        if (!test_bool)
        {
          return JSON_STREAM_STATUS__NO_MORE;
        }
      }
      while (0 == pipe_length);

      #if defined(_DEBUG)
        OSSpecific_writeDebugMessage(
          "{JsonByteStream} 0x%04X bytes on STDIN",
          pipe_length);
      #endif

      test_bool = (json_length <= pipe_length);
    }

    if (!test_bool)
    {
      #if defined(_DEBUG)
        OSSpecific_writeDebugMessage(
          "{JsonByteStream::loadFromStandardInput} invalid stream length!");
      #endif

      return JSON_STREAM_STATUS__NO_MORE;
    }
  }

  /* Initialize "JsonByteStream" object */

  stream->head = malloc(sizeof(BYTE) * json_length);
  if (NULL == (stream->head))
  {
    #if defined(_DEBUG)
      OSSpecific_writeDebugMessage(
        "{JsonByteStream::loadFromStandardInput} memory allocation failed!");
    #endif

    return JSON_STREAM_STATUS__NO_MORE;
  }

  stream->head_length = json_length;

  /* Read the rest of the STDIN stream (UTF-8 text) */

  test_bool = OSSpecific_readBytesFromStream(
    stdin_stream,
    &(stream->head[0]),
    json_length);

  if (!test_bool)
  {
    free(stream->head);
    return JSON_STREAM_STATUS__NO_MORE;
  }

  #if defined(_DEBUG)
    UTF8String hexdump;
    UTF8String_init(&(hexdump));
    UTF8String_pushBytesAsHex(&(hexdump), json_length, stream->head);
    OSSpecific_writeDebugMessage("{JsonByteStream} hex-dump: %s", (LPCSTR) hexdump.text);
    UTF8String_destroy(&(hexdump));
  #endif

  /* "JsonByteStream" object is now ready to be parsed */

  stream->tail = stream->head;
  stream->tail_length = json_length;

  return JSON_STREAM_STATUS__VALID;
}

/**************************************************************/

VOID
JsonByteStream_destroy(
  _Inout_ JsonByteStream *stream)
{
  if (NULL != stream->head)
  {
    free(stream->head);
    stream->head = NULL;
  }
}

/**************************************************************/

BOOL
JsonByteStream_peek(
  _In_ const JsonByteStream *stream,
  _Out_ BYTE *byte)
{
  if (stream->tail_length > 0)
  {
    byte[0] = stream->tail[0];
    return TRUE;
  }

  #if defined(_DEBUG)
  OSSpecific_writeDebugMessage(
    "{JsonByteStream} peek failed: no more bytes");
  #endif

  return FALSE;
}

/**************************************************************/

VOID
JsonByteStream_skip(
  _Inout_ JsonByteStream *stream,
  _In_ const size_t count)
{
  stream->tail += count;
  stream->tail_length -= count;
}

/**************************************************************/

BOOL
JsonByteStream_read(
  _Inout_ JsonByteStream *stream,
  _Out_ BYTE *output,
  _In_ const size_t count)
{
  if (stream->tail_length < count)
  {
    #if defined(_DEBUG)
    OSSpecific_writeDebugMessage(
      "{JsonByteStream} read failed: no more bytes");
    #endif

    return FALSE;
  }

  memcpy(output, stream->tail, count);
  JsonByteStream_skip(stream, count);
  return TRUE;
}

/**************************************************************/

BOOL
JsonByteStream_skipWhitespace(
  _Inout_ JsonByteStream *stream)
{
  BYTE test_byte;

  while (JsonByteStream_peek(stream, &(test_byte)))
  {
    switch (test_byte)
    {
      case ' ':  /* space */
      case '\n': /* line feed */
      case '\r': /* carriage return */
      case '\t': /* horizontal tab */
      {
        break;
      }

      default:
      {
        return TRUE;
      }
    }

    JsonByteStream_skip(stream, 1);
  }

  return FALSE;
}

/**************************************************************/
