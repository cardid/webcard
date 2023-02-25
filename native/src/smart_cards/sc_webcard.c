/**
 * @file "native/src/smart_cards/sc_webcard.c"
 * Communication with Smart Card Readers (physical or virtual peripherals).
 */

#include "smart_cards/smart_cards.h"

/**************************************************************/

#if defined(_DEBUG)

  LPCSTR
  WebCard_errorLookup(
    _In_ const PCSC_LONG errorCode)
  {
    switch (errorCode)
    {
      #if defined(_WIN32)
        case ERROR_BROKEN_PIPE:
          return "ERROR_BROKEN_PIPE";
        case SCARD_E_NO_PIN_CACHE:
          return "SCARD_E_NO_PIN_CACHE";
        case SCARD_E_PIN_CACHE_EXPIRED:
          return "SCARD_E_PIN_CACHE_EXPIRED";
        case SCARD_E_READ_ONLY_CARD:
          return "SCARD_E_READ_ONLY_CARD";
        case SCARD_E_UNEXPECTED:
          return "SCARD_E_UNEXPECTED";
        case SCARD_W_CACHE_ITEM_NOT_FOUND:
          return "SCARD_W_CACHE_ITEM_NOT_FOUND";
        case SCARD_W_CACHE_ITEM_STALE:
          return "SCARD_W_CACHE_ITEM_STALE";
        case SCARD_W_CACHE_ITEM_TOO_BIG:
          return "SCARD_W_CACHE_ITEM_TOO_BIG";
      #endif
      case SCARD_E_BAD_SEEK:
        return "SCARD_E_BAD_SEEK";
      case SCARD_E_CANCELLED:
        return "SCARD_E_CANCELLED";
      case SCARD_E_CANT_DISPOSE:
        return "SCARD_E_CANT_DISPOSE";
      case SCARD_E_CARD_UNSUPPORTED:
        return "SCARD_E_CARD_UNSUPPORTED";
      case SCARD_E_CERTIFICATE_UNAVAILABLE:
        return "SCARD_E_CERTIFICATE_UNAVAILABLE";
      case SCARD_E_COMM_DATA_LOST:
        return "SCARD_E_COMM_DATA_LOST";
      case SCARD_E_DIR_NOT_FOUND:
        return "SCARD_E_DIR_NOT_FOUND";
      case SCARD_E_DUPLICATE_READER:
        return "SCARD_E_DUPLICATE_READER";
      case SCARD_E_FILE_NOT_FOUND:
        return "SCARD_E_FILE_NOT_FOUND";
      case SCARD_E_ICC_CREATEORDER:
        return "SCARD_E_ICC_CREATEORDER";
      case SCARD_E_ICC_INSTALLATION:
        return "SCARD_E_ICC_INSTALLATION";
      case SCARD_E_INSUFFICIENT_BUFFER:
        return "SCARD_E_INSUFFICIENT_BUFFER";
      case SCARD_E_INVALID_ATR:
        return "SCARD_E_INVALID_ATR";
      case SCARD_E_INVALID_CHV:
        return "SCARD_E_INVALID_CHV";
      case SCARD_E_INVALID_HANDLE:
        return "SCARD_E_INVALID_HANDLE";
      case SCARD_E_INVALID_PARAMETER:
        return "SCARD_E_INVALID_PARAMETER";
      case SCARD_E_INVALID_TARGET:
        return "SCARD_E_INVALID_TARGET";
      case SCARD_E_INVALID_VALUE:
        return "SCARD_E_INVALID_VALUE";
      case SCARD_E_NO_ACCESS:
        return "SCARD_E_NO_ACCESS";
      case SCARD_E_NO_DIR:
        return "SCARD_E_NO_DIR";
      case SCARD_E_NO_FILE:
        return "SCARD_E_NO_FILE";
      case SCARD_E_NO_KEY_CONTAINER:
        return "SCARD_E_NO_KEY_CONTAINER";
      case SCARD_E_NO_MEMORY:
        return "SCARD_E_NO_MEMORY";
      case SCARD_E_NO_READERS_AVAILABLE:
        return "SCARD_E_NO_READERS_AVAILABLE";
      case SCARD_E_NO_SERVICE:
        return "SCARD_E_NO_SERVICE";
      case SCARD_E_NO_SMARTCARD:
        return "SCARD_E_NO_SMARTCARD";
      case SCARD_E_NO_SUCH_CERTIFICATE:
        return "SCARD_E_NO_SUCH_CERTIFICATE";
      case SCARD_E_NOT_READY:
        return "SCARD_E_NOT_READY";
      case SCARD_E_NOT_TRANSACTED:
        return "SCARD_E_NOT_TRANSACTED";
      case SCARD_E_PCI_TOO_SMALL:
        return "SCARD_E_PCI_TOO_SMALL";
      case SCARD_E_PROTO_MISMATCH:
        return "SCARD_E_PROTO_MISMATCH";
      case SCARD_E_READER_UNAVAILABLE:
        return "SCARD_E_READER_UNAVAILABLE";
      case SCARD_E_READER_UNSUPPORTED:
        return "SCARD_E_READER_UNSUPPORTED";
      case SCARD_E_SERVER_TOO_BUSY:
        return "SCARD_E_SERVER_TOO_BUSY";
      case SCARD_E_SERVICE_STOPPED:
        return "SCARD_E_SERVICE_STOPPED";
      case SCARD_E_SHARING_VIOLATION:
        return "SCARD_E_SHARING_VIOLATION";
      case SCARD_E_SYSTEM_CANCELLED:
        return "SCARD_E_SYSTEM_CANCELLED";
      case SCARD_E_TIMEOUT:
        return "SCARD_E_TIMEOUT";
      case SCARD_E_UNKNOWN_CARD:
        return "SCARD_E_UNKNOWN_CARD";
      case SCARD_E_UNKNOWN_READER:
        return "SCARD_E_UNKNOWN_READER";
      case SCARD_E_UNKNOWN_RES_MNG:
        return "SCARD_E_UNKNOWN_RES_MNG";
      case SCARD_E_UNSUPPORTED_FEATURE:
        return "SCARD_E_UNSUPPORTED_FEATURE";
      case SCARD_E_WRITE_TOO_MANY:
        return "SCARD_E_WRITE_TOO_MANY";
      case SCARD_F_COMM_ERROR:
        return "SCARD_F_COMM_ERROR";
      case SCARD_F_INTERNAL_ERROR:
        return "SCARD_F_INTERNAL_ERROR";
      case SCARD_F_UNKNOWN_ERROR:
        return "SCARD_F_UNKNOWN_ERROR";
      case SCARD_F_WAITED_TOO_LONG:
        return "SCARD_F_WAITED_TOO_LONG";
      case SCARD_P_SHUTDOWN:
        return "SCARD_P_SHUTDOWN";
      case SCARD_S_SUCCESS:
        return "SCARD_S_SUCCESS";
      case SCARD_W_CANCELLED_BY_USER:
        return "SCARD_W_CANCELLED_BY_USER";
      case SCARD_W_CARD_NOT_AUTHENTICATED:
        return "SCARD_W_CARD_NOT_AUTHENTICATED";
      case SCARD_W_CHV_BLOCKED:
        return "SCARD_W_CHV_BLOCKED";
      case SCARD_W_EOF:
        return "SCARD_W_EOF";
      case SCARD_W_REMOVED_CARD:
        return "SCARD_W_REMOVED_CARD";
      case SCARD_W_RESET_CARD:
        return "SCARD_W_RESET_CARD";
      case SCARD_W_SECURITY_VIOLATION:
        return "SCARD_W_SECURITY_VIOLATION";
      case SCARD_W_UNPOWERED_CARD:
        return "SCARD_W_UNPOWERED_CARD";
      case SCARD_W_UNRESPONSIVE_CARD:
        return "SCARD_W_UNRESPONSIVE_CARD";
      case SCARD_W_UNSUPPORTED_CARD:
        return "SCARD_W_UNSUPPORTED_CARD";
      case SCARD_W_WRONG_CHV:
        return "SCARD_W_WRONG_CHV";
      default:
        return "";
    }
  }

#endif

/**************************************************************/

BOOL
WebCard_establishContext(
  _Out_ LPSCARDCONTEXT resultContext)
{
  resultContext[0] = 0;

  PCSC_LONG pcscResult = SCardEstablishContext(
    SCARD_SCOPE_USER,
    NULL,
    NULL,
    resultContext);

  if (SCARD_S_SUCCESS != pcscResult)
  {
    #if defined(_DEBUG)
    {
      OSSpecific_writeDebugMessage(
        "{SCardEstablishContext} failed: 0x%08X (%s)",
        (uint32_t) pcscResult,
        WebCard_errorLookup(pcscResult));
    }
    #endif

    return FALSE;
  }

  #if defined(_DEBUG)
  {
    UTF8String hexdump;
    UTF8String_init(&(hexdump));
    UTF8String_pushBytesAsHex(
      &(hexdump),
      sizeof(SCARDCONTEXT),
      (const BYTE *) &(resultContext[0]));

    OSSpecific_writeDebugMessage(
      "{SCardEstablishContext} success: %s",
      (LPCSTR) hexdump.text);

    UTF8String_destroy(&(hexdump));
  }
  #endif

  return TRUE;
}

/**************************************************************/

BOOL
WebCard_init(
  _Out_ SCardReaderDB *resultDatabase,
  _Out_ SCARDCONTEXT *resultContext)
{
  int fetch_result;
  BOOL should_fetch;

  SCardReaderDB_init(resultDatabase);

  if (!WebCard_establishContext(resultContext))
  {
    return FALSE;
  }

  should_fetch = TRUE;
  while (should_fetch)
  {
    should_fetch = FALSE;

    fetch_result = SCardReaderDB_fetch(
      resultDatabase,
      NULL,
      resultContext[0],
      TRUE);

    if (WEBCARD_FETCH_READERS__FAIL == fetch_result)
    {
      #if defined(_DEBUG)
      {
        OSSpecific_writeDebugMessage(
          "{SCardReaderDB::fetch} failed");
      }
      #endif

      return FALSE;
    }
    else if (WEBCARD_FETCH_READERS__SERVICE_STOPPED == fetch_result)
    {
      SCardReleaseContext(resultContext[0]);

      if (!WebCard_establishContext(resultContext))
      {
        return FALSE;
      }

      should_fetch = TRUE;
    }
  }

  return TRUE;
}

/**************************************************************/

VOID
WebCard_run(void)
{
  SCARDCONTEXT context;
  SCardReaderDB database;
  int byte_stream_status;
  int fetch_result;

  JsonByteStream json_stream;
  JsonObject json_request;
  JsonObject json_response;
  JsonArray json_reader_names;

  clock_t cpu_time_start = clock();
  clock_t cpu_time_end;
  double cpu_time_elapsed;

  BOOL should_fetch;
  BOOL active = WebCard_init(&(database), &(context));

  while (active)
  {
    cpu_time_end = clock();
    cpu_time_elapsed =
      ((double)(cpu_time_end - cpu_time_start)) / FIXED_CLOCKS_PER_SEC;

    /* Do the fetching every 1.0 second(s) */

    if (cpu_time_elapsed >= 1.0)
    {
      cpu_time_start = cpu_time_end;

      /* 1) Fetch list of Smart Card Readers */
      /* (detecting plugging and unplugging) */

      should_fetch = TRUE;
      while (should_fetch)
      {
        should_fetch = FALSE;

        fetch_result = SCardReaderDB_fetch(
          &(database),
          &(json_reader_names),
          context,
          FALSE);

        if ((WEBCARD_FETCH_READERS__FAIL != fetch_result) &&
          (WEBCARD_FETCH_READERS__IGNORE != fetch_result))
        {
          if (WEBCARD_FETCH_READERS__SERVICE_STOPPED == fetch_result)
          {
            SCardReleaseContext(context);

            if (WebCard_establishContext(&(context)))
            {
              /* Context re-established (Smart Card Service re-launched), */
              /* now try fetching the list of readers again! */
              should_fetch = TRUE;
            }
            else
            {
              active = FALSE;
            }
          }
          else
          {
            WebCard_sendReaderEvent(
              NULL,
              0,
              (WEBCARD_FETCH_READERS__MORE_READERS == fetch_result) ?
                WEBCARD_READER_EVENT__READERS_MORE :
                WEBCARD_READER_EVENT__READERS_LESS,
              &(json_response),
              &(json_reader_names));

            JsonObject_destroy(&(json_response));
          }
        }

        JsonArray_destroy(&(json_reader_names));
      }
    }

    /* Smart Card Service Context might be lost */
    /* when the last reader is unplugged */

    if (active)
    {
      /* 2) Update Smart Card Reader Status list */
      /* (detecting existence of smart cards) */

      WebCard_handleStatusChange(&(database), context);

      /* 3) Parse commands from Standard Input */

      byte_stream_status = JsonByteStream_loadFromStandardInput(&(json_stream));

      if (JSON_STREAM_STATUS__VALID == byte_stream_status)
      {
        WebCard_handleRequest(
          &(json_stream),
          &(json_request),
          &(json_response),
          &(database),
          context);

        JsonObject_destroy(&(json_request));
        JsonObject_destroy(&(json_response));
      }
      else if (JSON_STREAM_STATUS__NO_MORE == byte_stream_status)
      {
        active = FALSE;
      }
    }
  }

  WebCard_close(&(database), context);
}

/**************************************************************/

VOID
WebCard_close(
  _Inout_ SCardReaderDB *database,
  _In_ const SCARDCONTEXT context)
{
  SCardReaderDB_destroy(database);

  if (0 != context)
  {
    SCardReleaseContext(context);
  }
}

/**************************************************************/

VOID
WebCard_handleRequest(
  _Inout_ JsonByteStream *jsonStream,
  _Out_ JsonObject *jsonRequest,
  _Out_ JsonObject *jsonResponse,
  _In_ const SCardReaderDB *database,
  _In_ const SCARDCONTEXT context)
{
  BOOL test_bool;
  JsonValue json_value;
  UTF8String utf8_string;
  size_t command;

  /* Initialize JSON response object */
  /* (it will be destroyed by caller) */

  JsonObject_init(jsonResponse);

  /* Initialize and load JSON request object */
  /* Destroy `jsonStream` after parsing the JSON object */

  test_bool = JsonObject_parse(
    &(jsonRequest),
    FALSE,
    jsonStream);

  JsonByteStream_destroy(jsonStream);

  if (!test_bool)
  {
    #if defined(_DEBUG)
      OSSpecific_writeDebugMessage(
        "{JSON Request} parsing error!");
    #endif

    return;
  }

  /* Try to find the "i" key (unique message identifier) */

  test_bool = JsonObject_getValue(
    jsonRequest,
    &(json_value),
    "i");

  if (!test_bool || (JSON_VALUE_TYPE__STRING != json_value.type))
  {
    return;
  }

  /* Try to append the "i" key to JSON response */

  test_bool = JsonObject_appendKeyValue(
    jsonResponse,
    "i",
    &(json_value));

  if (!test_bool) { return; }

  /* Try to find the "c" key (request command) */

  test_bool = JsonObject_getValue(
    jsonRequest,
    &(json_value),
    "c");

  if (!test_bool || (JSON_VALUE_TYPE__NUMBER != json_value.type))
  {
    return;
  }

  /* Handle requested command */

  command = (size_t) (((FLOAT *) json_value.value)[0]);

  switch (command)
  {
    case WEBCARD_COMMAND__LIST_READERS:
    {
      test_bool = WebCard_pushReadersListToJsonResponse(
        jsonResponse,
        database);

      break;
    }

    case WEBCARD_COMMAND__CONNECT:
    {
      test_bool = WebCard_tryConnectingToReader(
        jsonRequest,
        jsonResponse,
        database,
        context);

      break;
    }

    case WEBCARD_COMMAND__DISCONNECT:
    {
      test_bool = WebCard_tryDisconnectingFromReader(
        jsonRequest,
        database);

      /* "Empty" response (JSON object containing the "i" key only) */
      /* will be required to resolve a "JavaScript Promise" */
      break;
    }

    case WEBCARD_COMMAND__TRANSCEIVE:
    {
      test_bool = WebCard_transmitAndReceive(
        jsonRequest,
        jsonResponse,
        database);

      break;
    }

    case WEBCARD_COMMAND__GET_VERSION:
    {
      UTF8String_makeTemporary(&(utf8_string), WEBCARD_VERSION);

      json_value.type = JSON_VALUE_TYPE__STRING;
      json_value.value = &(utf8_string);

      test_bool = JsonObject_appendKeyValue(
        jsonResponse,
        "verNat",
        &(json_value));

      break;
    }

    default:
    {
      test_bool = TRUE;
    }
  }

  /* Try to always send a JSON Response (so that a JavaScript Promise */
  /* won't hang), even if a WebCard's command-handling function has failed */

  if (!test_bool)
  {
    /* Append an optional key-value "incomplete=true" */

    json_value.type = JSON_VALUE_TYPE__TRUE;
    json_value.value = NULL;

    JsonObject_appendKeyValue(jsonResponse, "incomplete", &(json_value));
  }

  /* Stringify JSON response and send it through the STDOUT stream */

  UTF8String_init(&(utf8_string));

  test_bool = JsonObject_toString(jsonResponse, &(utf8_string));

  if (test_bool)
  {
    UTF8String_writeToStandardOutput(&(utf8_string));
  }

  UTF8String_destroy(&(utf8_string));
}

/**************************************************************/

BOOL
WebCard_pushReaderNameToJsonString(
  _In_ const SCARD_READERSTATE *readerState,
  _Out_ UTF8String *resultReaderName)
{
  BOOL test_bool;

  #ifdef _UNICODE
    UTF16String utf16_reader_name;
  #endif

  UTF8String_init(resultReaderName);

  #ifdef _UNICODE
  {
    UTF16String_init(&(utf16_reader_name));

    test_bool = UTF16String_pushText(
      &(utf16_reader_name),
      readerState->szReader,
      0);

    if (test_bool)
    {
      test_bool = UTF16String_toUTF8(
        &(utf16_reader_name),
        resultReaderName);
    }

    UTF16String_destroy(&(utf16_reader_name));
  }
  #else
  {
    test_bool = UTF8String_pushText(
      resultReaderName,
      readerState->szReader,
      0);
  }
  #endif

  return test_bool;
}

/**************************************************************/

BOOL
WebCard_pushReaderNameToJsonArray(
  _In_ const SCARD_READERSTATE *readerState,
  _Inout_ JsonArray *jsonArray)
{
  BOOL test_bool;
  JsonValue json_value;
  UTF8String utf8_reader_name;

  test_bool = WebCard_pushReaderNameToJsonString(
    readerState,
    &(utf8_reader_name));

  if (!test_bool)
  {
    UTF8String_destroy(&(utf8_reader_name));
    return FALSE;
  }

  /* Put "Reader Name" at the end of given array */

  json_value.type = JSON_VALUE_TYPE__STRING;
  json_value.value = &(utf8_reader_name);

  test_bool = JsonArray_append(jsonArray, &(json_value));

  UTF8String_destroy(&(utf8_reader_name));

  return test_bool;
}

/**************************************************************/

BOOL
WebCard_pushReaderNameToJsonObject(
  _In_ const SCARD_READERSTATE *readerState,
  _Inout_ JsonObject *jsonObject,
  _In_ LPCSTR key)
{
  BOOL test_bool;
  JsonValue json_value;
  UTF8String utf8_reader_name;

  test_bool = WebCard_pushReaderNameToJsonString(
    readerState,
    &(utf8_reader_name));

  if (!test_bool)
  {
    UTF8String_destroy(&(utf8_reader_name));
    return FALSE;
  }

  /* Add "Reader Name" under a specified key */

  json_value.type = JSON_VALUE_TYPE__STRING;
  json_value.value = &(utf8_reader_name);

  test_bool = JsonObject_appendKeyValue(
    jsonObject,
    key,
    &(json_value));

  UTF8String_destroy(&(utf8_reader_name));

  return test_bool;
}

/**************************************************************/

BOOL
WebCard_convertReaderStateToJsonObject(
  _In_ const SCARD_READERSTATE *readerState,
  _Out_ JsonObject *jsonReaderObject)
{
  BOOL test_bool;

  /* Initialize JSON reader object */
  /* (it will be destroyed by caller) */

  JsonObject_init(jsonReaderObject);

  /* Add key "n" (Reader Name) */

  test_bool = WebCard_pushReaderNameToJsonObject(
    readerState,
    jsonReaderObject,
    "n");

  if (!test_bool) { return FALSE; }

  /* Add key "a" (card Answer To Reset) */

  return WebCard_pushReaderAtrToJsonObject(
    readerState,
    jsonReaderObject,
    "a");
}

/**************************************************************/

BOOL
WebCard_pushReaderAtrToJsonObject(
  _In_ const SCARD_READERSTATE *readerState,
  _Inout_ JsonObject *jsonObject,
  _In_ LPCSTR key)
{
  BOOL test_bool;
  JsonValue json_value;
  UTF8String utf8_string;

  /* Copy Smart Card "ATR" identifier (bytearray to text) */

  UTF8String_init(&(utf8_string));

  test_bool = UTF8String_pushBytesAsHex(
    &(utf8_string),
    readerState->cbAtr,
    readerState->rgbAtr);

  if (!test_bool)
  {
    UTF8String_destroy(&(utf8_string));
    return FALSE;
  }

  /* Add "card Answer To Reset" under a specified key */

  json_value.type = JSON_VALUE_TYPE__STRING;
  json_value.value = &(utf8_string);

  test_bool = JsonObject_appendKeyValue(
    jsonObject,
    key,
    &(json_value));

  UTF8String_destroy(&(utf8_string));

  return test_bool;
}

/**************************************************************/

BOOL
WebCard_convertReaderStatesToJsonArray(
  _In_ const SCardReaderDB *database,
  _Out_ JsonArray *jsonReadersArray)
{
  BOOL test_bool;
  JsonValue json_value;
  JsonObject json_reader_object;

  json_value.type = JSON_VALUE_TYPE__OBJECT;
  json_value.value = &(json_reader_object);

  /* Initialize JSON reader object */
  /* (it will be destroyed by caller) */

  JsonArray_init(jsonReadersArray);

  /* Enumerate Smart Card Readers */

  for (size_t i = 0; i < database->count; i++)
  {
    test_bool = WebCard_convertReaderStateToJsonObject(
      &(database->states[i]),
      &(json_reader_object));

    if (test_bool)
    {
      test_bool = JsonArray_append(
        jsonReadersArray,
        &(json_value));
    }

    JsonObject_destroy(&(json_reader_object));

    if (!test_bool)
    {
      return FALSE;
    }
  }

  return TRUE;
}

/**************************************************************/

BOOL
WebCard_pushReadersListToJsonResponse(
  _Inout_ JsonObject *jsonResponse,
  _In_ const SCardReaderDB *database)
{
  BOOL test_bool;
  JsonArray json_readers_array;
  JsonValue json_value;

  test_bool = WebCard_convertReaderStatesToJsonArray(
    database,
    &(json_readers_array));

  if (!test_bool)
  {
    JsonArray_destroy(&(json_readers_array));
    return FALSE;
  }

  json_value.type = JSON_VALUE_TYPE__ARRAY;
  json_value.value = &(json_readers_array);

  test_bool = JsonObject_appendKeyValue(
    jsonResponse,
    "d",
    &(json_value));

  JsonArray_destroy(&(json_readers_array));
  return test_bool;
}

/**************************************************************/

BOOL
WebCard_tryConnectingToReader(
  _In_ const JsonObject *jsonRequest,
  _Inout_ JsonObject *jsonResponse,
  _In_ const SCardReaderDB *database,
  _In_ const SCARDCONTEXT context)
{
  BOOL test_bool;
  size_t reader_index;
  const SCARD_READERSTATE *readerState;
  PCSC_DWORD share_mode = SCARD_SHARE_SHARED;
  JsonValue json_value;

  /* Try to find the "r" key (reader index) */

  test_bool = JsonObject_getValue(
    jsonRequest,
    &(json_value),
    "r");

  if (!test_bool || (JSON_VALUE_TYPE__NUMBER != json_value.type))
  {
    #if defined(_DEBUG)
    {
      OSSpecific_writeDebugMessage(
        "{WebCard::tryConnectingToReader} failed: " \
        "missing \"r\" key!"
      );
    }
    #endif

    return FALSE;
  }

  reader_index = (size_t) (((FLOAT *) json_value.value)[0]);

  if (reader_index >= database->count)
  {
    #if defined(_DEBUG)
    {
      OSSpecific_writeDebugMessage(
        "{WebCard::tryConnectingToReader} failed: " \
        "invalid reader index!"
      );
    }
    #endif

    return FALSE;
  }

  /* Try to find the "p" key (optional share mode param) */

  test_bool = JsonObject_getValue(
    jsonRequest,
    &(json_value),
    "p");

  if (test_bool && (JSON_VALUE_TYPE__NUMBER == json_value.type))
  {
    share_mode = (PCSC_DWORD) (((FLOAT *) json_value.value)[0]);
  }

  /* Try to open a connection to active Smart Card */

  readerState = &(database->states[reader_index]);

  test_bool = SCardConnection_open(
    &(database->connections[reader_index]),
    context,
    readerState->szReader,
    share_mode);

  if (!test_bool) { return FALSE; }

  /* Add key "d" (card Answer To Reset) */

  return WebCard_pushReaderAtrToJsonObject(
    readerState,
    jsonResponse,
    "d");
}

/**************************************************************/

BOOL
WebCard_tryDisconnectingFromReader(
  _In_ const JsonObject *jsonRequest,
  _In_ const SCardReaderDB *database)
{
  BOOL test_bool;
  size_t reader_index;
  JsonValue json_value;

  /* Try to find the "r" key (reader index) */

  test_bool = JsonObject_getValue(
    jsonRequest,
    &(json_value),
    "r");

  if (!test_bool || (JSON_VALUE_TYPE__NUMBER != json_value.type))
  {
    #if defined(_DEBUG)
    {
      OSSpecific_writeDebugMessage(
        "{WebCard::tryDisconnectingFromReader} failed: " \
        "missing \"r\" key!"
      );
    }
    #endif

    return FALSE;
  }

  reader_index = (size_t) (((FLOAT *) json_value.value)[0]);

  if (reader_index >= database->count)
  {
    #if defined(_DEBUG)
    {
      OSSpecific_writeDebugMessage(
        "{WebCard::tryDisconnectingFromReader} failed: " \
        "invalid reader index!"
      );
    }
    #endif

    return FALSE;
  }

  /* Try to close a connection to active Smart Card */

  return SCardConnection_close(
    &(database->connections[reader_index]));
}

/**************************************************************/

BOOL
WebCard_transmitAndReceive(
  _In_ const JsonObject *jsonRequest,
  _Inout_ JsonObject *jsonResponse,
  _In_ const SCardReaderDB *database)
{
  BOOL test_bool;
  size_t reader_index;
  LPBYTE input_bytes;
  size_t input_bytes_length;
  LPBYTE output_bytes;
  JsonValue json_value;
  UTF8String utf8_hex_apdu_response;
  SCardConnection *connection;

  /* Try to find the "r" key (reader index) */

  test_bool = JsonObject_getValue(
    jsonRequest,
    &(json_value),
    "r");

  if (!test_bool || (JSON_VALUE_TYPE__NUMBER != json_value.type))
  {
    #if defined(_DEBUG)
    {
      OSSpecific_writeDebugMessage(
        "{WebCard::transmitAndReceive} failed: " \
        "missing \"r\" key!"
      );
    }
    #endif

    return FALSE;
  }

  reader_index = (size_t) (((FLOAT *) json_value.value)[0]);

  if (reader_index >= database->count)
  {
    #if defined(_DEBUG)
    {
      OSSpecific_writeDebugMessage(
        "{WebCard::transmitAndReceive} failed: " \
        "invalid reader index!"
      );
    }
    #endif

    return FALSE;
  }

  /* Make sure that a connection to the Smart Card is still active */

  connection = &(database->connections[reader_index]);

  if (0 == connection->handle)
  {
    #if defined(_DEBUG)
    {
      OSSpecific_writeDebugMessage(
        "{WebCard::transmitAndReceive} failed: " \
        "no connection!"
      );
    }
    #endif

    return FALSE;
  }

  /* Try to find the "a" key (Application Protocol Data Unit) */

  test_bool = JsonObject_getValue(
    jsonRequest,
    &(json_value),
    "a");

  if (!test_bool || (JSON_VALUE_TYPE__STRING != json_value.type))
  {
    return FALSE;
  }

  /* Prepare input and output byte buffers */

  test_bool = UTF8String_hexToByteArray(
    json_value.value,
    &(input_bytes_length),
    &(input_bytes));

  if (!test_bool)
  {
    if (NULL != input_bytes)
    {
      free(input_bytes);
    }
    return FALSE;
  }

  output_bytes = malloc(sizeof(BYTE) * MAX_APDU_SIZE);
  if (NULL == output_bytes)
  {
    free(input_bytes);
    return FALSE;
  }

  /* Transmit and receive */

  UTF8String_init(&(utf8_hex_apdu_response));

  test_bool = SCardConnection_transceiveMultiple(
    connection,
    &(utf8_hex_apdu_response),
    input_bytes,
    input_bytes_length,
    output_bytes,
    MAX_APDU_SIZE);

  if (test_bool)
  {
    /* Add key "d" (Smart Card APDU response) */

    json_value.type = JSON_VALUE_TYPE__STRING;
    json_value.value = &(utf8_hex_apdu_response);

    test_bool = JsonObject_appendKeyValue(
      jsonResponse,
      "d",
      &(json_value));
  }

  UTF8String_destroy(&(utf8_hex_apdu_response));

  return test_bool;
}

/**************************************************************/

VOID
WebCard_sendReaderEvent(
  _In_opt_ const SCARD_READERSTATE *readerState,
  _In_ const size_t readerIndex,
  _In_ const int readerEvent,
  _Out_ JsonObject *jsonResponse,
  _In_opt_ const JsonArray *jsonEventDetails)
{
  BOOL test_bool;
  FLOAT test_float;
  JsonValue json_value;
  UTF8String utf8_string;

  #if defined(_DEBUG)
  {
    OSSpecific_writeDebugMessage(
      "Sending ReaderEvent '%d' (ReaderIndex '%d')",
      readerEvent,
      readerIndex);
  }
  #endif

  /* Initialize JSON response object */
  /* (it will be destroyed by caller) */

  JsonObject_init(jsonResponse);

  json_value.type = JSON_VALUE_TYPE__NUMBER;
  json_value.value = &(test_float);

  /* Add key "e" (reader event) */

  test_float = (FLOAT) readerEvent;

  test_bool = JsonObject_appendKeyValue(
    jsonResponse,
    "e",
    &(json_value));

  if (!test_bool) { return; }

  if (NULL != readerState)
  {
    /* Add key "r" (reader index for reader events) */

    test_float = (FLOAT) readerIndex;

    test_bool = JsonObject_appendKeyValue(
      jsonResponse,
      "r",
      &(json_value));

    if (!test_bool) { return; }

    /* Add key "d" (card Answer To Reset) on CARD INSERT event */

    if (WEBCARD_READER_EVENT__CARD_INSERTION == readerEvent)
    {
      test_bool = WebCard_pushReaderAtrToJsonObject(
        readerState,
        jsonResponse,
        "d");

      if (!test_bool) { return; }
    }
  }
  else
  {
    if (NULL != jsonEventDetails)
    {
      /* Add key "n" (optional reader names) */

      json_value.type = JSON_VALUE_TYPE__ARRAY;
      json_value.value = (void *) jsonEventDetails;

      test_bool = JsonObject_appendKeyValue(
        jsonResponse,
        "n",
        &(json_value));

      if (!test_bool) { return; }
    }
  }

  /* Stringify JSON response and send it through the STDOUT stream */

  UTF8String_init(&(utf8_string));

  test_bool = JsonObject_toString(jsonResponse, &(utf8_string));

  if (test_bool)
  {
    UTF8String_writeToStandardOutput(&(utf8_string));
  }

  UTF8String_destroy(&(utf8_string));
}

/**************************************************************/

VOID
WebCard_handleStatusChange(
  _Inout_ SCardReaderDB *database,
  _In_ const SCARDCONTEXT context)
{
  JsonObject json_response;

  PCSC_LONG pcscResult = SCardGetStatusChange(
    context,
    0,
    database->states,
    database->count);

  if (SCARD_S_SUCCESS != pcscResult) { return; }

  /* Enumerate Smart Card Readers */

  for (size_t i = 0; i < database->count; i++)
  {
    SCARD_READERSTATE *readerState = &(database->states[i]);
    SCardConnection *connection = &(database->connections[i]);

    if (readerState->dwEventState & SCARD_STATE_CHANGED)
    {
      if (connection->ignoreCounter > 0)
      {
        connection->ignoreCounter -= 1;
      }
      else
      {
        int reader_event = WEBCARD_READER_EVENT__NONE;

        if ((readerState->dwCurrentState & SCARD_STATE_EMPTY) &&
          (readerState->dwEventState & SCARD_STATE_PRESENT))
        {
          reader_event = WEBCARD_READER_EVENT__CARD_INSERTION;
        }
        else if ((readerState->dwCurrentState & SCARD_STATE_PRESENT) &&
          (readerState->dwEventState & SCARD_STATE_EMPTY))
        {
          reader_event = WEBCARD_READER_EVENT__CARD_REMOVAL;

          /* Invalidate connection */
          connection->handle = 0;
        }

        if (WEBCARD_READER_EVENT__NONE != reader_event)
        {
          WebCard_sendReaderEvent(
            readerState,
            i,
            reader_event,
            &(json_response),
            NULL);

          JsonObject_destroy(&(json_response));
        }
      }

      readerState->dwCurrentState = (readerState->dwEventState & (~SCARD_STATE_CHANGED));
    }
  }
}

/**************************************************************/
