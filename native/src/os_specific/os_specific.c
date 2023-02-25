/**
 * @file "native/src/os_specific/os_specific.c"
 * Operating-System-specific functions
 */

#include "os_specific/os_specific.h"

/**************************************************************/

#if defined(__linux__) || defined(__APPLE__)

  size_t
  OSSpecific_wideStrLen(_In_z_ LPCWSTR string)
  {
    size_t size = 0;

    while ('\0' != string[size])
    {
      size++;
    }

    return size;
  }

#endif

/**************************************************************/

BOOL
OSSpecific_validateTypesOfStreams(
  _In_ const os_specific_stream_t inputStream,
  _In_ const os_specific_stream_t outputStream)
{
  #if defined(_WIN32)
  {
    if (FILE_TYPE_PIPE != GetFileType(inputStream))
    {
      #if defined(_DEBUG)
      OSSpecific_writeDebugMessage(
        "Expected {Standard Input} type to be a pipe");
      #endif

      return FALSE;
    }

    if (FILE_TYPE_PIPE != GetFileType(outputStream))
    {
      #if defined(_DEBUG)
      OSSpecific_writeDebugMessage(
        "Expected {Standard Output} type to be a pipe");
      #endif

      return FALSE;
    }

    return TRUE;
  }
  #elif defined(__linux__) || defined(__APPLE__)
  {
    struct stat file_status;

    if (0 != fstat(inputStream, &(file_status)))
    {
      #if defined(_DEBUG)
      OSSpecific_writeDebugMessage(
        "{fstat} failed: errno=0x%08X",
        errno);
      #endif

      return FALSE;
    }

    if (!S_ISFIFO(file_status.st_mode))
    {
      #if defined(_DEBUG)
      OSSpecific_writeDebugMessage(
        "Expected {Standard Input} type to be a pipe");
      #endif

      return FALSE;
    }

    if (0 != fstat(outputStream, &(file_status)))
    {
      #if defined(_DEBUG)
      OSSpecific_writeDebugMessage(
        "{fstat} failed: errno=0x%08X",
        errno);
      #endif

      return FALSE;
    }

    if (!S_ISFIFO(file_status.st_mode))
    {
      #if defined(_DEBUG)
      OSSpecific_writeDebugMessage(
        "Expected {Standard Output} type to be a pipe");
      #endif

      return FALSE;
    }

    return TRUE;
  }
  #else
  {
    return FALSE;
  }
  #endif
}

/**************************************************************/

BOOL
OSSpecific_peekStream(
  _In_ const os_specific_stream_t stream,
  _Out_ uint32_t *streamSizeRef)
{
  #if defined(_WIN32)
  {
    BOOL test_bool;
    DWORD pipe_length;

    test_bool = PeekNamedPipe(
      stream,
      NULL,
      0,
      NULL,
      &(pipe_length),
      NULL);

    /* If function fails, `GetLastError()` returns: */
    /* - `ERROR_INVALID_FUNCTION` when STDIN is a console or a file */
    /* - `ERROR_BROKEN_PIPE` when other end of the pipe was closed (e.g.: */
    /*  extension was disabled by a Web Browser; parent process was closed) */

    if (test_bool)
    {
      streamSizeRef[0] = pipe_length;
    }
    #if defined(_DEBUG)
    else
    {
      OSSpecific_writeDebugMessage(
        "{PeekNamedPipe} failed: 0x%08X",
        GetLastError());
    }
    #endif

    return test_bool;
  }
  #elif defined(__linux__) || defined(__APPLE__)
  {
    int32_t result;

    struct pollfd fds =
    {
      .fd = stream,
      .events = POLLIN
    };

    result = poll(&(fds), 1, 0);

    if ((-1) == result)
    {
      #if defined(_DEBUG)
      OSSpecific_writeDebugMessage(
        "{poll} failed: errno=0x%08X",
        errno);
      #endif

      return FALSE;
    }
    else if (0 == result)
    {
      /* Polling timeout - ignore */

      streamSizeRef[0] = 0;
      return TRUE;
    }
    else
    {
      #if defined(_DEBUG)
      OSSpecific_writeDebugMessage(
        "{poll} POLLIN=%d POLLHUP=%d",
        !!(POLLIN & fds.revents),
        !!(POLLHUP & fds.revents));
      #endif

      if (POLLHUP & fds.revents)
      {
        /* Writing end of the pipe was closed */

        /* "macOS" always sends both `(POLLIN | POLLHUP)`,      */
        /*  so we must prioritize `POLLHUP` check, even if      */
        /*  that would drop one or two final WebCard responses. */

        return FALSE;
      }
      else if (POLLIN & fds.revents)
      {
        /* There is data to read - peek amount of available bytes */

        if ((-1) == ioctl(stream, FIONREAD, &(result)))
        {
          #if defined(_DEBUG)
          OSSpecific_writeDebugMessage(
            "{ioctl} failed: errno=0x%08X",
            errno);
          #endif

          return FALSE;
        }

        streamSizeRef[0] = (uint32_t) result;

        return TRUE;
      }
    }

    return FALSE;
  }
  #else
  {
    return FALSE;
  }
  #endif
}

/**************************************************************/

BOOL
OSSpecific_readBytesFromStream(
  _In_ const os_specific_stream_t stream,
  _Out_ void *output,
  _In_ const size_t size)
{
  #if defined(_WIN32)
  {
    BOOL test_bool;
    DWORD test_dword;

    test_bool = ReadFile(
      stream,
      output,
      size,
      &(test_dword),
      NULL);

    #if defined(_DEBUG)
    {
      if (test_bool && (size == test_dword)) { return TRUE; }

      OSSpecific_writeDebugMessage(
        "{ReadFile} failed: 0x%08X",
        GetLastError());

      return FALSE;
    }
    #else
    {
      return (test_bool && (size == test_dword));
    }
    #endif
  }
  #elif defined(__linux__) || defined(__APPLE__)
  {
    #if defined(_DEBUG)
    {
      ssize_t result = read(stream, output, size);
      if (size == result) { return TRUE; }

      OSSpecific_writeDebugMessage(
        "{read} failed: errno=0x%08X",
        errno);

      return FALSE;
    }
    #else
    {
      return (size == read(stream, output, size));
    }
    #endif
  }
  #else
  {
    return FALSE;
  }
  #endif
}

/**************************************************************/

BOOL
OSSpecific_writeBytesToStream(
  _In_ const os_specific_stream_t stream,
  _In_ const void *input,
  _In_ const size_t size)
{
  #if defined(_WIN32)
  {
    BOOL test_bool;
    DWORD test_dword;

    test_bool = WriteFile(
      stream,
      input,
      size,
      &(test_dword),
      NULL);

    if (test_bool && (size == test_dword))
    {
      #if defined(_DEBUG)
      {
        test_bool = FlushFileBuffers(stream);

        if (!test_bool)
        {
          test_dword = GetLastError();

          if (ERROR_INVALID_HANDLE == test_dword)
          {
            return TRUE;
          }

          OSSpecific_writeDebugMessage(
            "{FlushFileBuffers} failed: 0x%08X",
            GetLastError());
        }

        return test_bool;
      }
      #else
      {
        return FlushFileBuffers(stream);
      }
      #endif
    }

    #if defined(_DEBUG)
    {
      OSSpecific_writeDebugMessage(
        "{WriteFile} failed: 0x%08X",
        GetLastError());
    }
    #endif

    return FALSE;
  }
  #elif defined(__linux__) || defined(__APPLE__)
  {
    #if defined(_DEBUG)
    {
      ssize_t result = write(stream, input, size);
      if (size == result) { return TRUE; }

      OSSpecific_writeDebugMessage(
        "{write} failed");

      return FALSE;
    }
    #else
    {
      return (size == write(stream, input, size));
    }
    #endif
  }
  #else
  {
    return FALSE;
  }
  #endif
}

/**************************************************************/

#if defined(_DEBUG)

  VOID
  OSSpecific_writeDebugMessage(
    _In_z_ LPCSTR message,
    ...)
  {
    CHAR buffer[512];
    va_list args;
    os_specific_stream_t stderr_stream;

    va_start(args, message);
    vsnprintf(buffer, sizeof(buffer), message, args);
    va_end(args);

    #if defined(_WIN32)
    {
      stderr_stream = GetStdHandle(STD_ERROR_HANDLE);

      if (INVALID_HANDLE_VALUE == stderr_stream)
      {
        return;
      }
    }
    #elif defined(__linux__) || defined(__APPLE__)
    {
      stderr_stream = STDERR_FILENO;
    }
    #endif

    OSSpecific_writeBytesToStream(
      stderr_stream,
      DEBUG_MESSAGE_START,
      sizeof(CHAR) * strlen(DEBUG_MESSAGE_START));

    OSSpecific_writeBytesToStream(
      stderr_stream,
      buffer,
      sizeof(CHAR) * strlen(buffer));

    OSSpecific_writeBytesToStream(
      stderr_stream,
      DEBUG_MESSAGE_END,
      sizeof(CHAR) * strlen(DEBUG_MESSAGE_END));

    #if defined(_WIN32)
    {
      OutputDebugStringA(buffer);
    }
    #endif
  }

#endif

/**************************************************************/
