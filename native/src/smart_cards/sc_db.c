/**
 * @file "native/src/smart_cards/sc_db.c"
 * Communication with Smart Card Readers (physical or virtual peripherals).
 */

#include "smart_cards/smart_cards.h"

/**************************************************************/

VOID
SCardReaderDB_init(
  _Out_ SCardReaderDB *database)
{
  database->count = 0;
  database->states = NULL;
  database->connections = NULL;
}

/**************************************************************/

VOID
SCardReaderDB_destroy(
  _Inout_ SCardReaderDB *database)
{
  int i;

  if (NULL != database->states)
  {
    for (i = 0; i < database->count; i++)
    {
      if (NULL != database->states[i].szReader)
      {
        free((void *) database->states[i].szReader);
      }
    }

    free(database->states);
  }

  if (NULL != database->connections)
  {
    for (i = 0; i < database->count; i++)
    {
      SCardConnection_close(&(database->connections[i]));
    }

    free(database->connections);
  }
}

/**************************************************************/

BOOL
SCardReaderDB_load(
  _Out_ SCardReaderDB *database,
  _In_ LPCTSTR readerNames)
{
  size_t byteSize;
  size_t nameLength;

  LPCTSTR nextReader;
  SCARD_READERSTATE *readerStateRef;
  SCardConnection *testConnection;

  /* Initialize outgoing `SCardReaderDB` structure */

  SCardReaderDB_init(database);

  /* Iterate through every Smart Card Reader */

  nextReader = readerNames;

  while (nextReader[0])
  {
    /* Expand the "Smart Card Reader State" list */

    byteSize = sizeof(SCARD_READERSTATE) * (1 + database->count);
    readerStateRef = realloc(database->states, byteSize);
    if (NULL == readerStateRef) { return FALSE; }

    database->states = readerStateRef;
    readerStateRef = &(database->states[database->count]);

    /* Initialize "Smart Card Reader State" structure for current reader */

    readerStateRef->szReader = NULL;
    readerStateRef->dwCurrentState = SCARD_STATE_UNAWARE;
    readerStateRef->cbAtr = 0;

    /* Expand the "Smart Card Connection" list */

    byteSize = sizeof(SCardConnection) * (1 + database->count);
    testConnection = realloc(database->connections, byteSize);
    if (NULL == testConnection) { return FALSE; }

    database->connections = testConnection;
    testConnection = &(database->connections[database->count]);

    /* Initialize "Smart Card Connection" structure for current reader */

    SCardConnection_init(testConnection);

    /* Both lists have "+1" valid (initialized) structure */

    database->count += 1;

    /* Get Smart Card Reader name's length */

    nameLength = _tcslen(nextReader);

    /* Clone Smart Card Reader name */

    byteSize = sizeof(TCHAR) * (1 + nameLength);
    readerStateRef->szReader = malloc(byteSize);
    if (NULL == readerStateRef->szReader) { return FALSE; }

    memcpy((void *) readerStateRef->szReader, nextReader, byteSize);

    /* Move to the next entry in a multi-string list */

    nextReader = &(nextReader[1 + nameLength]);
  }

  return TRUE;
}

/**************************************************************/

BOOL
SCardReaderDB_hasReaderNamed(
  _In_ const SCardReaderDB *database,
  _In_ LPCTSTR readerName)
{
  int i;

  for (i = 0; i < database->count; i++)
  {
    if (0 == _tcscmp(database->states[i].szReader, readerName))
    {
      return TRUE;
    }
  }

  return FALSE;
}

/**************************************************************/

int
SCardReaderDB_fetch(
  _Inout_ SCardReaderDB *database,
  _Out_ JsonArray *jsonReaderNames,
  _In_ const SCARDCONTEXT context,
  _In_ const BOOL firstFetch)
{
  int fetchResult;
  PCSC_LONG pcscResult;
  size_t byteSize;
  PCSC_DWORD testLength;
  BOOL testBool;

  int i;
  LPTSTR readerNames;
  SCardReaderDB testDatabase;

  if (NULL != jsonReaderNames)
  {
    JsonArray_init(jsonReaderNames);
  }

  /* Get total length of the multi-string list */

  testLength = 0;

  pcscResult = SCardListReaders(
    context,
    NULL,
    NULL,
    &(testLength));

  if (SCARD_S_SUCCESS != pcscResult)
  {
    if (SCARD_E_SERVICE_STOPPED == pcscResult)
    {
      /* Last reader unplugged */
      return WEBCARD_FETCH_READERS__SERVICE_STOPPED;
    }
    else if (SCARD_E_NO_READERS_AVAILABLE == pcscResult)
    {
      /* Ignore this error, as readers can be plugged-in */
      /* later while the Native App is still running */
      testLength = 0;
    }
    else
    {
      #if defined(_DEBUG)
      {
        OSSpecific_writeDebugMessage(
          "{SCardListReaders} failed: 0x%08X (%s)",
          (uint32_t) pcscResult,
          WebCard_errorLookup(pcscResult));
      }
      #endif

      return WEBCARD_FETCH_READERS__FAIL;
    }
  }

  /* Are there any Smart Card Readers even available? */

  if (0 == testLength)
  {
    if (firstFetch)
    {
      /* Signal no errors on the first database look-up */
      return WEBCARD_FETCH_READERS__IGNORE;
    }

    if (0 != database->count)
    {
      /* Some readers were connected before */
      if (NULL != jsonReaderNames)
      {
        for (i = 0; i < database->count; i++)
        {
          WebCard_pushReaderNameToJsonArray(
            &(database->states[i]),
            jsonReaderNames);
        }
      }
      SCardReaderDB_destroy(database);
      SCardReaderDB_init(database);

      return WEBCARD_FETCH_READERS__LESS_READERS;
    }

    /* No readers connected before or after the fetching */
    return WEBCARD_FETCH_READERS__IGNORE;
  }

  /* Allocate memory for the multi-string list */

  byteSize = sizeof(TCHAR) * testLength;
  readerNames = malloc(byteSize);
  if (NULL == readerNames) { return WEBCARD_FETCH_READERS__FAIL; }

  /* Get Smart Card Reader names */

  pcscResult = SCardListReaders(
    context,
    NULL,
    readerNames,
    &(testLength));

  if (SCARD_S_SUCCESS != pcscResult)
  {
    free(readerNames);
    return WEBCARD_FETCH_READERS__FAIL;
  }

  /* Check if the array should be refreshed */

  if (firstFetch)
  {
    testBool = TRUE;
    fetchResult = WEBCARD_FETCH_READERS__MORE_READERS;
  }
  else
  {
    testLength = Misc_multiStringList_elementCount(readerNames);

    if (testLength != database->count)
    {
      testBool = TRUE;
      fetchResult = (testLength > database->count) ?
        WEBCARD_FETCH_READERS__MORE_READERS :
        WEBCARD_FETCH_READERS__LESS_READERS;
    }
    else
    {
      testBool = FALSE;
    }
  }

  if (!testBool)
  {
    free(readerNames);
    return WEBCARD_FETCH_READERS__IGNORE;
  }

  /* Prepare local Smart Card Readers array */

  testBool = SCardReaderDB_load(&(testDatabase), readerNames);

  if (NULL != readerNames)
  {
    free(readerNames);
  }

  if (!testBool)
  {
    SCardReaderDB_destroy(&(testDatabase));
    return WEBCARD_FETCH_READERS__FAIL;
  }

  if (NULL != jsonReaderNames)
  {
    /* Find differences in two databases */

    if (WEBCARD_FETCH_READERS__MORE_READERS == fetchResult)
    {
      /* Look for the names in OLD (SMALLER) database */
      /* that are not found in the NEW (LARGER) database */
      for (i = 0; i < testDatabase.count; i++)
      {
        testBool = SCardReaderDB_hasReaderNamed(
          database,
          testDatabase.states[i].szReader);

        if (!testBool)
        {
          WebCard_pushReaderNameToJsonArray(
            &(testDatabase.states[i]),
            jsonReaderNames);
        }
      }
    }
    else
    {
      /* Look for the names in NEW (SMALLER) database */
      /* that are not found in the OLD (LARGER) database */
      for (i = 0; i < database->count; i++)
      {
        testBool = SCardReaderDB_hasReaderNamed(
          &(testDatabase),
          database->states[i].szReader);

        if (!testBool)
        {
          WebCard_pushReaderNameToJsonArray(
            &(database->states[i]),
            jsonReaderNames);
        }
      }
    }
  }

  /* Destroy previous Smart Card Readers array */

  SCardReaderDB_destroy(database);

  /* Replace outgoing array with the local array */
  /* (direct assignment: local destructor should not be called) */

  database[0] = testDatabase;

  return fetchResult;
}

/**************************************************************/
