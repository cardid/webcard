/**
 * @file "native/src/os_specific/wtypes_for_unix.h"
 * List of Windows Data Types.
 * `<stdint.h>` must be included before including this header file.
 */

/**
 * Same guard as in "PCSC Lite" library,
 * to avoid conflits and typedef redefinitions.
 */
#ifndef __wintypes_h__
#define __wintypes_h__

/**************************************************************/

/**
 * +--------------+--------------+
 * | Architecture | sizeof(long) |
 * +--------------+--------------+
 * | Windows  x86 |      4 Bytes |
 * +--------------+--------------+
 * | Windows  x64 |      4 Bytes |
 * +--------------+--------------+
 * | Linux    x86 |      4 Bytes |
 * +--------------+--------------+
 * | Linux    x64 |      8 Bytes |
 * +--------------+--------------+
 * | macOS    x64 |      8 Bytes |
 * +--------------+--------------+
 */

#if defined(__APPLE__)
  /** Windows-compliant (both 32-bit and 64-bit) */
  typedef int32_t  LONG;
  typedef uint32_t DWORD;

#elif defined (__linux__)
  /** Discrepancy on 64-bit architectures! */
  typedef          long LONG;
  typedef unsigned long DWORD;

#endif

/**
 * Marking these type-definitions as "non-reliable"
 * when the code is compiled for 64-bit Linux distributions
 */
typedef LONG  PCSC_LONG;
typedef DWORD PCSC_DWORD;

/**************************************************************/

#if defined(__linux__) || defined(__APPLE__)

  /** Values assignable to the `BOOL` data type */
  #define FALSE  0
  #define TRUE   1

  /** Basic sized types */
  typedef int      BOOL;
  typedef void     VOID;
  typedef uint8_t  BYTE;
  typedef float    FLOAT;

  /** Pointer-types required by the WinSCard */
  typedef       VOID  *LPVOID;
  typedef const VOID  *LPCVOID;
  typedef       BYTE  *LPBYTE;
  typedef const BYTE  *LPCBYTE;
  typedef       DWORD *LPDWORD;

  /**
   * Windows character type: "8-bit ASCII"
   * (UTF-8 compatible)
   */
  typedef char CHAR;

  /**
   * Windows character type: "UTF-16"
   * (incompatible with the 32-bit `wchar_t` on Linux!)
   */
  typedef uint16_t WCHAR;

  /**
   * Pointer-types for strings
   * (null-terminated character arrays)
   */
  typedef       CHAR *LPSTR;
  typedef const CHAR *LPCSTR;
  typedef       WCHAR *LPWSTR;
  typedef const WCHAR *LPCWSTR;

  /**
   * Generic text-types:
   * - `TCHAR`/`LPTSTR` use 16-bit characters on Windows
   *  (code-page independent, "Wide" prototypes)
   * - `TCHAR`/`LPTSTR` use 8-bit characters on Linux and macOS
   *  (UTF-8 compatible, "ANSI" prototypes)
   */
  typedef char   TCHAR;
  typedef LPSTR  LPTSTR;
  typedef LPCSTR LPCTSTR;
  #define _tcscmp  strcmp
  #define _tcslen  strlen

  /** Ignore "SAL" (Microsoft "Source Code Annotation") */
  #define _In_
  #define _Out_
  #define _Inout_
  #define _In_z_
  #define _In_opt_
  #define _Out_opt_
  #define _Outptr_result_maybenull_

#endif

/**************************************************************/

#endif  /* __wintypes_h__ */
