/**
 * @file "native/src/misc/misc.c"
 * Miscellaneous functions.
 */

#include "misc/misc.h"

/**************************************************************/

size_t
Misc_multiStringList_elementCount(
  _In_opt_ LPCTSTR listHead)
{
  if (NULL == listHead) { return 0; }

  size_t count = 0;
  while (listHead[0])
  {
    size_t elementLength = _tcslen(listHead);
    listHead = &(listHead[1 + elementLength]);
    count += 1;
  }

  return count;
}

/**************************************************************/

size_t
Misc_nextPowerOfTwo(
  _In_ size_t number)
{
  if (0 == number)
  {
    return 1;
  }

  if (number <= 2)
  {
    return (2 * number);
  }

  do
  {
    number++;
  }
  while (0 != ((number - 1) & number));

  return number;
}

/**************************************************************/

BOOL
Misc_pushToLocalBuffer(
  _In_ const LPCSTR bufferStart,
  _Inout_ LPSTR *bufferEndRef,
  _In_ const size_t size,
  _In_ const char character)
{
  /* Last space reserved for NULL terminator */
  if ((bufferEndRef[0] - bufferStart) >= (size - 1))
  {
    return FALSE;
  }

  bufferEndRef[0][0] = character;
  bufferEndRef[0] += 1;

  return TRUE;
}


/**************************************************************/
