/**
 * @file "native/src/misc/misc.h"
 * Declarations for miscellaneous functions.
 */

#ifndef H_WEBCARD__MISC
#define H_WEBCARD__MISC

#include "os_specific/os_specific.h"

#ifdef __cplusplus
  extern "C" {
#endif


/**************************************************************/
/* MISCELLANEOUS                                              */
/**************************************************************/

/**
 * @brief Get the number of elements in a multi-string list.
 *
 * Multi-string list contains some NULL-terminated strings one-after-another,
 * plus another NULL-terminator after the last entry in the list.
 *
 * @param[in] listHead The head (pointer to the first element)
 * of a multi-string list. This parameter can be `NULL`.
 * @return Element count.
 */
extern size_t
Misc_multiStringList_elementCount(
  _In_opt_ LPCTSTR listHead);

/**
 * @brief Get a number that is larger than given input
 * and a power of 2.
 *
 * @param[in] number The initial number.
 * @return Calculated power of 2.
 */
extern size_t
Misc_nextPowerOfTwo(
  _In_ size_t number);

/**
 * @brief Push an ASCII character into a local text buffer.
 *
 * This function validates if the buffer has still some space left
 * for more characters, and automatically updates the buffer-end pointer.
 *
 * @param[in] bufferStart Pointer to the first element of a local text buffer.
 * @param[in,out] bufferEndRef Pointer to a pointer where the next character
 * will be stored (first empty space in the buffer).
 * @param[in] size The total size of the local text buffer
 * (that should not be exceeded).
 * @param[in] character The ASCII character to put at the end of the buffer.
 * @return `TRUE` on success, `FALSE` on a buffer overflow.
 */
extern BOOL
Misc_pushToLocalBuffer(
  _In_ const LPCSTR bufferStart,
  _Inout_ LPSTR *bufferEndRef,
  _In_ const size_t size,
  _In_ const char character);


/**************************************************************/

#ifdef __cplusplus
  }
#endif

#endif  /* H_WEBCARD__MISC */
