/**
 * @file "native/src/os_specific/os_specific.h"
 * Operating-System-specific definitions and declarations
 */

#ifndef H_WEBCARD__OS_SPECIFIC
#define H_WEBCARD__OS_SPECIFIC

#ifdef __cplusplus
  extern "C" {
#endif


/**************************************************************/
/* HEADER FILES FOR SPECIFIC TARGETS (OPERATING SYSTEMS)      */
/**************************************************************/

#if defined(_WIN32)

  /** UTF-16 encoding + generic-type text support. */
  #define _UNICODE 1
  #define UNICODE 1
  #include <tchar.h>

  /** Windows API */
  #include <windows.h>

#elif defined(__linux__) || defined(__APPLE__)

  /**
   * Standard C library, includes:
   *  `EXIT_SUCCESS`, `EXIT_FAILURE`,
   *  `malloc()`, `realloc()`, `free()`.
   */
  #include <stdlib.h>

  /** Linux File operations */
  #include <fcntl.h>
  #include <sys/stat.h>
  #include <sys/ioctl.h>
  #include <poll.h>
  #include <unistd.h>

  /**
   * C library for strings, includes:
   *  `strlen()`, `memcpy()`.
   */
  #include <string.h>

#else
  #error("Unsupported Operating System, sorry!")
  #pragma GCC error "Unsupported Operating System, sorry!"
#endif


/**************************************************************/
/* MORE STANDARD LIBRARIES, TYPE DEFINITIONS                  */
/**************************************************************/

/** Timing */
#include <time.h>

#if defined(__APPLE__)
  #define FIXED_CLOCKS_PER_SEC  (CLOCKS_PER_SEC / 100)
#else
  #define FIXED_CLOCKS_PER_SEC  CLOCKS_PER_SEC
#endif

/** Fixed-width integer type definitions */
#include <stdint.h>

/**
 * Standard C Input-output declarations, includes:
 *  `snprintf()`, `vsnprintf()`.
 */
#include <stdio.h>

#if defined(_DEBUG)

  /** Accessing variable-argument lists. */
  #include <stdarg.h>

#endif

/** Checking error codes when standard functions fail. */
#include <errno.h>

/** Windows Data Types. */
#include "wtypes_for_unix.h"

#if defined(_WIN32)
  typedef HANDLE os_specific_stream_t;

#elif defined(__linux__) || defined(__APPLE__)
  typedef int os_specific_stream_t;

#endif


/**************************************************************/
/* CUSTOM WIDE CHARS (exactly 16-bit wide)                    */
/**************************************************************/

#if defined(_WIN32)
  #define OSSpecific_wideStrLen wcslen

#elif defined(__linux__) || defined(__APPLE__)
  extern size_t
  OSSpecific_wideStrLen(_In_z_ LPCWSTR string);

#endif


/**************************************************************/
/* PIPE OPERATIONS                                            */
/**************************************************************/

/**
 * @brief Validates if the process was launched with correct
 * types of standard streams.
 *
 * Both the Input Stream and the Output Stream should be PIPEs
 * (not disk files, nor keyboard/console).
 * @param[in] inputStream OS-specific stream that corresponds to `STDIN`.
 * @param[in] outputStream OS-specific stream that corresponds to `STDOUT`.
 * @return `TRUE` on successful validation, otherwise `FALSE`.
 */
extern BOOL
OSSpecific_validateTypesOfStreams(
  _In_ const os_specific_stream_t inputStream,
  _In_ const os_specific_stream_t outputStream);

/**
 * @brief Peeks how many bytes are available in a given stream.
 *
 * @param[in] stream OS-specific stream descriptor, open for reading.
 * @param[out] streamSize pointer to a location that receives number
 * of pending bytes.
 * @return `TRUE` on successful peek, `FALSE` on pipe-access errors.
 */
extern BOOL
OSSpecific_peekStream(
  _In_ const os_specific_stream_t stream,
  _Out_ uint32_t *streamSizeRef);

/**
 * @brief Reads bytes from a stream.
 *
 * @param[in] stream OS-specific stream descriptor, open for reading.
 * @param[out] output Memory location where `size` of bytes will be stored.
 * @param[in] size Constant number of bytes to read from the stream.
 * @return `TRUE` on success, `FALSE` on any stream error.
 */
extern BOOL
OSSpecific_readBytesFromStream(
  _In_ const os_specific_stream_t stream,
  _Out_ void *output,
  _In_ const size_t size);

/**
 * @brief Writes bytes to a stream.
 *
 * @param[in] stream OS-specific stream descriptor, open for writing.
 * @param[in] input Memory location from where `size` of bytes will be loaded.
 * @param[in] size Constant number of bytes to write to the stream.
 * @return `TRUE` on success, `FALSE` on any stream error.
 */
extern BOOL
OSSpecific_writeBytesToStream(
  _In_ const os_specific_stream_t stream,
  _In_ const void *input,
  _In_ const size_t size);


/**************************************************************/
/* DEBUG DEFINITIONS AND DECLARATIONS                         */
/**************************************************************/

#if defined(_DEBUG)

  #define DEBUG_MESSAGE_START "-- "
  #define DEBUG_MESSAGE_END " --\n"

  /**
   * @brief Outputs a debug message (info or error).
   *
   * On all operating systems, the message is passed to `STDERR` stream.
   * On Windows, the message is also passed to `OutputDebugStringA` function.
   * @param[in] message A NULL-terminated UTF-8 string
   * that acts as a message format.
   * @param[in] ... variable arguments for message formatting.
   */
  extern VOID
  OSSpecific_writeDebugMessage(
    _In_z_ LPCSTR message,
    ...);

#endif

/**************************************************************/

#ifdef __cplusplus
  }
#endif

#endif  /* H_WEBCARD__OS_SPECIFIC */
