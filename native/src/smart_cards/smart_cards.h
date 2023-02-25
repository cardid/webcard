/**
 * @file "native/src/smart_cards/smart_cards.h"
 * Communication with Smart Card Readers (physical or virtual peripherals).
 */

#ifndef H_WEBCARD__SMART_CARDS
#define H_WEBCARD__SMART_CARDS

#include "os_specific/os_specific.h"
#include "misc/misc.h"
#include "utf/utf.h"
#include "json/json.h"

#if defined(_WIN32)
  #include <winscard.h>

#elif defined(__linux__)
  /* Included "os_specific.h" => "wtypes_for_unix.h" */
  #include <PCSC/winscard.h>

#elif defined(__APPLE__)
  /* Included "os_specific.h" => "wtypes_for_unix.h" */
  #include <PCSC/musclecard.h>
  #define SCARD_READERSTATE SCARD_READERSTATE_A
  #define LPSCARD_READERSTATE LPSCARD_READERSTATE_A

#endif

#ifdef __cplusplus
  extern "C" {
#endif


/**************************************************************/
/* WEBCARD DEFINITIONS                                        */
/**************************************************************/

#define WEBCARD_VERSION  "0.4.0"

#define MAX_APDU_SIZE  0x7FFF

/**
 * Possible "Reader Event" values.
 */

  #define WEBCARD_READER_EVENT__NONE            0
  #define WEBCARD_READER_EVENT__CARD_INSERTION  1
  #define WEBCARD_READER_EVENT__CARD_REMOVAL    2
  #define WEBCARD_READER_EVENT__READERS_MORE    3
  #define WEBCARD_READER_EVENT__READERS_LESS    4

/**
 * Possible "Webcard Command" values.
 */

  #define WEBCARD_COMMAND__NONE           0
  #define WEBCARD_COMMAND__LIST_READERS   1
  #define WEBCARD_COMMAND__CONNECT        2
  #define WEBCARD_COMMAND__DISCONNECT     3
  #define WEBCARD_COMMAND__TRANSCEIVE     4
  #define WEBCARD_COMMAND__GET_VERSION   10

/**
 * Possible return values for `SCardReaderDB_fetch` function.
 */

  #define WEBCARD_FETCH_READERS__FAIL             0
  #define WEBCARD_FETCH_READERS__SERVICE_STOPPED  1
  #define WEBCARD_FETCH_READERS__IGNORE           2
  #define WEBCARD_FETCH_READERS__MORE_READERS     3
  #define WEBCARD_FETCH_READERS__LESS_READERS     4


/**************************************************************/
/* SMART CARD CONNECTION                                      */
/**************************************************************/

/**
 * `SCardConnection` type definition.
 */
typedef struct SCardConnection SCardConnection;

/**
 * Connection parameters for disgnated Smart Card reader.
 */
struct SCardConnection
{
  /** A handle that identifies the connection to the Smart Card in the designated reader. */
  SCARDHANDLE handle;

  /** A flag that indicates the established active protocol. */
  PCSC_DWORD activeProtocol;

  /** How many incoming Reader State Changes should be ignored. */
  DWORD ignoreCounter;
};

/**
 * @brief `SCardConnection` constructor.
 *
 * @param[out] connection Reference to an UNINITIALIZED
 * `SCardConnection` object.
 */
extern VOID
SCardConnection_init(
  _Out_ SCardConnection *connection);

/**
 * @brief Opens connection to a Smart Card Reader.
 *
 * @param[in,out] connection Reference to a VALID `SCardConnection` object.
 * @param[in] context A handle that identifies the resource manager context.
 * @param[in] readerName The name of the reader that contains the target card.
 * @param[in] shareMode A flag that indicates whether other applications
 * may form connections to the card.
 * @return `TRUE` on success (or if the connection is already open),
 * `FALSE` if the connection was NOT established.
 */
extern BOOL
SCardConnection_open(
  _Inout_ SCardConnection *connection,
  _In_ const SCARDCONTEXT context,
  _In_ LPCTSTR readerName,
  _In_ const PCSC_DWORD shareMode);

/**
 * @brief Closes connection to a Smart Card Reader.
 *
 * @param[in,out] connection Reference to a VALID `SCardConnection` object.
 * @return `TRUE` on success (or if the connection is already closed),
 * `FALSE` if any Smart Card error has occurred.
 */
extern BOOL
SCardConnection_close(
  _Inout_ SCardConnection *connection);

/**
 * @brief Sends a service request to the smart card
 * and expects to receive data back from the card.
 *
 * @param[in] connection Reference to a VALID and CONSTANT
 * `SCardConnection` object.
 * @param[in] input Data to be written to the card.
 * @param[in] inputLength The length of `input` buffer, in bytes.
 * @param[out] output Data returned from the card.
 * @param[in,out] outputLengthRef Supplies the length, in bytes,
 * of the `output` buffer, and receives the actual number of bytes
 * received from the smart card.
 * @return `TRUE` on success (one APDU sent and one APDU received),
 * `FALSE` if any Smart Card error has occurred.
 */
extern BOOL
SCardConnection_transceiveSingle(
  _In_ const SCardConnection *connection,
  _In_ const BYTE *input,
  _In_ const PCSC_DWORD inputLength,
  _Out_ BYTE *output,
  _Inout_ PCSC_DWORD *outputLengthRef);

/**
 * @brief Sends a large APDU to the smart card
 * and concatenates response to a one large string of data.
 *
 * @param[in] connection Reference to a VALID and CONSTANT
 * `SCardConnection` object.
 * @param[in,out] hexStringResult Refernce to a VALID `UTF8String` object.
 * Reponse in form of hex-string will be appended at the end of this param.
 * @param[in] input Data to be written to the card.
 * @param[in] inputLength The length of `input` buffer, in bytes.
 * @param[out] output Buffer that can be used to collect reponse data.
 * @param[in] outputLength The length of `output` buffer, in bytes.
 * @return `TRUE` on success (one APDU sent and multiple APDUs received),
 * `FALSE` if any Smart Card error has occurred.
 */
extern BOOL
SCardConnection_transceiveMultiple(
  _In_ const SCardConnection *connection,
  _Inout_ UTF8String *hexStringResult,
  _In_ const BYTE *input,
  _In_ const PCSC_DWORD inputLength,
  _Out_ LPBYTE output,
  _In_ const PCSC_DWORD outputLength);


/**************************************************************/
/* SMART CARD READER DATABASE                                 */
/**************************************************************/

/**
 * `SCardReaderDB` type definition.
 */
typedef struct SCardReaderDB SCardReaderDB;

/**
 * Database of Smart Card Readers.
 */
struct SCardReaderDB
{
  /** Number of allocated Smart Card Readers. */
  int count;

  /**
   * Array of `SCARD_READERSTATE` structures, needed for
   * `SCardGetStatusChange()` function.
   */
  SCARD_READERSTATE *states;

  /**
   * Array of `SCardConnection` structures, needed for
   * establishing connections and for data transmission.
   */
  SCardConnection *connections;
};

/**
 * @brief `SCardReaderDB` constructor.
 *
 * @param[out] database Reference to an UNINITIALIZED `SCardReaderDB` object.
 */
extern VOID
SCardReaderDB_init(
  _Out_ SCardReaderDB *database);

/**
 * @brief `SCardReaderDB` destructor.
 *
 * @param[in,out] database Reference to a VALID `SCardReaderDB` object.
 *
 * @note After this call, `database` should not be used (unless re-initialized).
 */
extern VOID
SCardReaderDB_destroy(
  _Inout_ SCardReaderDB *database);

/**
 * @brief Prepares a Smart Card Reader Database (list od states
 * and list of connections) from given reader names.
 *
 * @param[out] database Reference to an UNINITIALIZED `SCardReaderDB` object.
 * @param[in] readerNames The head (pointer to the first element)
 * of a multi-string list. This parameter should NOT be `NULL`.
 * @return `TRUE` on successful load, `FALSE` on memory allocation errors.
 *
 * @note After this call, `database` will hold a VALID (at least initialized)
 * `SCardReaderDB` object. If the function returned `FALSE`,
 * `database` shall be destroyed.
 */
extern BOOL
SCardReaderDB_load(
  _Out_ SCardReaderDB *database,
  LPCTSTR readerNames);

/**
 * @brief Checks if given Smart Card Reader exists in a given Database.
 *
 * @param[in] database Reference to a VALID and CONSTANT `SCardReaderDB` object.
 * @param[in] readerName Name of the queried Smart Card Reader.
 * @return `TRUE` if an exact name exists in `states` list, otherwise `FALSE`.
 */
extern BOOL
SCardReaderDB_hasReaderNamed(
  _In_ const SCardReaderDB *database,
  _In_ LPCTSTR readerName);

/**
 * @brief Fetches the list of currently connected Smart Card Readers
 * and replaces given Database if the number is different.
 *
 * @param[in] database Reference to a VALID `SCardReaderDB` object.
 * @param[out] jsonReaderNames An UNITIALIZED `JsonArray` object,
 * to which the names of changed (plugged in or out) readers will be appended.
 * @param[in] context A handle that identifies the resource manager context.
 * @param[in] firstFetch Should the initial contents of `database` be ignored?
 * @return `WEBCARD_FETCH_READERS__IGNORE` if no changes were detected;
 * `WEBCARD_FETCH_READERS__LESS_READERS` if some reades were disconnected
 * (and `jsonReaderNames` will hold the names of now-missing readers);
 * `WEBCARD_FETCH_READERS__MORE_READERS` if some readers were connected
 * (and `jsonReaderNames` will hold the names of just-added readers);
 * `WEBCARD_FETCH_READERS__SERVICE_STOPPED` if last reader was disconnected,
 * the Smart Card Service has stopped and must be re-established;
 * `WEBCARD_FETCH_READERS__FAIL` on any error (and the Database doesn't change).
 *
 * @note After this call, `jsonReaderNames` will hold a VALID
 * (at least initialized) `JsonArray` object.
 * It must be released by the caller.
 */
extern int
SCardReaderDB_fetch(
  _Inout_ SCardReaderDB *database,
  _Out_ JsonArray *jsonReaderNames,
  _In_ const SCARDCONTEXT context,
  _In_ const BOOL firstFetch);


/**************************************************************/
/* WEBCARD OPERATIONS                                         */
/**************************************************************/

#if defined(_DEBUG)

  /**
   * @brief Returns a string representation of a `WinSCard` Error Code.
   *
   * @param[in] errorCode Error Code returned by any of the `WinSCard` functions.
   * @return Contant string (mapping number to a name).
   */
  extern LPCSTR
  WebCard_errorLookup(
    _In_ const PCSC_LONG errorCode);

#endif

/**
 * @brief (Re)establishes the Smart Card Context.
 * (Required for any `WinSCard` function calls)
 *
 * @param[out] resultContext Pointer to a `SCARDCONTEXT` variable
 * (pointer to a handle that identifies the resource manager context).
 * @return `TRUE` on success, `FALSE` if any Smart Card error has occurred.
 */
extern BOOL
WebCard_establishContext(
  _Out_ LPSCARDCONTEXT resultContext);

/**
 * @brief Initializes the `WebCard` NativeApp (Readers Database,
 * Smart Card Context) before entering the main application loop.
 *
 * @param[out] resultDatabase Reference to an UNINITIALIZED
 * `SCardReaderDB` object.
 * @param[out] resultContext Pointer to an UNITIALIZED variable
 * of `SCARDCONTEXT` type (pointer to a handle that identifies
 * the resource manager context).
 * @return `TRUE` on successful initialization,
 * `FALSE` if any Smart Card error has occurred.
 */
extern BOOL
WebCard_init(
  _Out_ SCardReaderDB *resultDatabase,
  _Out_ SCARDCONTEXT *resultContext);

/**
 * @brief Enters the `WebCard` main loop.
 *
 * This function takes care of `WebCard_init` and `WebCard_close`.
 */
extern VOID
WebCard_run(void);

/**
 * @brief Shuts down the `WebCard` NativeApp.
 *
 * @param[in,out] database Reference to a VALID `SCardReaderDB` object.
 * @param[out] context Previously established `SCARDCONTEXT` variable.
 * (handle that identifies the resource manager context).
 */
extern VOID
WebCard_close(
  _Inout_ SCardReaderDB *database,
  _In_ const SCARDCONTEXT context);

/**
 * @brief Takes a stream of bytes, tries to create a JSON Object from it,
 * and then chooses appropriate path based on the JSON Request.
 *
 * @param[in,out] jsonStream Reference to a valid (preloaded) stream of bytes,
 * from which the `jsonRequest` is constructed.
 * @param[out] jsonRequest Reference to an UNITIALIZED `JsonObject` variable
 * that will hold the JSON Request (input command).
 * @param[out] jsonResponse Reference to an UNITIALIZED `JsonObject` variable
 * that will hold the JSON Response (output).
 * @param[in] database Reference to a VALID and CONSTANT `SCardReaderDB` object
 * that holds the states of plugged-in Smart Card Readers.
 * @param[in] context A handle that identifies the resource manager context.
 * @note After this call, `jsonRequest` and `jsonResponse` will be initialized
 * and they must be released by the caller.
 */
extern VOID
WebCard_handleRequest(
  _Inout_ JsonByteStream *jsonStream,
  _Out_ JsonObject *jsonRequest,
  _Out_ JsonObject *jsonResponse,
  _In_ const SCardReaderDB *database,
  _In_ const SCARDCONTEXT context);

/**
 * @brief Extracts UTF-8 name from given Smart Card Reader State.
 *
 * @param[in] reader Reference to a read-only Reader State,
 * that contains the reader name property (`->szReader`).
 * @param[out] resultReaderName Reference to an UNINITIALIZED `UTF8String`
 * variable, which will hold the Reader Name.
 * @return `TRUE` on successful copy-conversion,
 * `FALSE` on memory allocation failure.
 *
 * @note On Windows, the names will be stored in UTF-16 (UNICODE) format.
 * We are not choosing the ANSI formatting (and UTF-8 is not a part of ANSI).
 * So, this requires a quick conversion from UTF-16 to UTF-8.
 * On Linux however, the names are already stored in UTF-8 format.
 * @note `resultReaderName` must be released by the caller.
 */
extern BOOL
WebCard_pushReaderNameToJsonString(
  _In_ const SCARD_READERSTATE *readerState,
  _Out_ UTF8String *resultReaderName);

/**
 * @brief Appends selected Reader's name to a given JSON Array.
 *
 * @param[in] readerState Reference to a read-only Reader State,
 * that contains the reader name property (`->szReader`).
 * @param[in,out] jsonArray Reference to a VALID `JsonArray` object,
 * to which the Reader's name will be appended.
 * @return `TRUE` on success, `FALSE` on memory allocation failure.
 */
extern BOOL
WebCard_pushReaderNameToJsonArray(
  _In_ const SCARD_READERSTATE *readerState,
  _Inout_ JsonArray *jsonArray);

/**
 * @brief Appends selected Reader's name to a given JSON Object under some key.
 *
 * @param[in] readerState Reference to a read-only Reader State,
 * that contains the reader name property (`->szReader`).
 * @param[in,out] jsonObject Reference to a VALID `JsonObject` object,
 * under which the Reader's name will be appended.
 * @param[in] key Case-sensitive and read-only UTF-8 text, that describes
 * the key under which Reader's name will be stored.
 * The key must end with a NULL-terminator!
 * @return `TRUE` on success, `FALSE` on memory allocation failure.
 */
extern BOOL
WebCard_pushReaderNameToJsonObject(
  _In_ const SCARD_READERSTATE *readerState,
  _Inout_ JsonObject *jsonObject,
  _In_ LPCSTR key);

/**
 * @brief Gathers basic info about selected Smart Card Reader
 * into a JSON Object.
 *
 * Appends selected Reader's name ("n") and Reader's ATR ("a")
 * to a given JSON Object under predefined keys.
 * @param[in] readerState Reference to a read-only Reader State,
 * that contains the reader name property (`->szReader`)
 * and the "Answer To Reset" property (`->rgbAtr`).
 * @param[out] jsonReaderObject Reference to a VALID `JsonObject` object
 * (presumably empty, only initialized), that will hold the basic info
 * that uniquely identifies a Smart Card Reader connected to the OS.
 * @return `TRUE` on success, `FALSE` on memory allocation failure.
 *
 * @note `jsonReaderObject` must be released by the caller.
 */
extern BOOL
WebCard_convertReaderStateToJsonObject(
  _In_ const SCARD_READERSTATE *readerState,
  _Out_ JsonObject *jsonReaderObject);

/**
 * @brief Appends selected Reader's ATR to a given JSON Object under some key.
 *
 * @param[in] readerState Reference to a read-only Reader State,
 * that contains the "Answer To Reset" property (`->rgbAtr`).
 * @param[in,out] jsonObject Reference to a VALID `JsonObject` object,
 * under which the Reader's ATR will be appended.
 * @param[in] key Case-sensitive and read-only UTF-8 text, that describes
 * the key under which Reader's ATR will be stored.
 * The key must end with a NULL-terminator!
 * @return `TRUE` on success, `FALSE` on memory allocation failure.
 */
extern BOOL
WebCard_pushReaderAtrToJsonObject(
  _In_ const SCARD_READERSTATE *readerState,
  _Inout_ JsonObject *jsonObject,
  _In_ LPCSTR key);

/**
 * @brief Gathers JSON Objects from each connected Smart Card Reader
 * and generates a JSON Array (list of readers and their states).
 *
 * @param[in] database Reference to a VALID and CONSTANT `SCardReaderDB` object
 * that holds the states of plugged-in Smart Card Readers.
 * @param[out] jsonReadersArray Reference to an UNITIALIZED `JsonArray`
 * variable that will hold the reader states (each reader's name and ATR).
 * @return `TRUE` on success, `FALSE` on memory allocation failure.
 *
 * @note `jsonReadersArray` must be released by the caller.
 */
extern BOOL
WebCard_convertReaderStatesToJsonArray(
  _In_ const SCardReaderDB *database,
  _Out_ JsonArray *jsonReadersArray);

/**
 * @brief Executes one of the main WebCard commands, which gathers
 * the list of all plugged-in Smart Card Readers.
 *
 * @param[in,out] jsonResponse Reference to a VALID `JsonObject` object
 * that will hold the list of Smart Card Reader states,
 * under the predefined "d" (data) key.
 * @param[in] database Reference to a VALID and CONSTANT `SCardReaderDB` object
 * that holds the states of plugged-in Smart Card Readers.
 * @return `TRUE` on success, `FALSE` on memory allocation failure.
 */
extern BOOL
WebCard_pushReadersListToJsonResponse(
  _Inout_ JsonObject *jsonResponse,
  _In_ const SCardReaderDB *database);

/**
 * @brief Executes one of the main WebCard commands, which attempts
 * to establish a connection from OS to the selected Smart Card Reader.
 *
 * @param[in] jsonRequest Reference to a VALID and CONSTANT `JsonObject` object
 * that contains the Smart Card Reader Index ("r") key
 * and the optional Share Mode parameter ("p") key.
 * @param[in,out] jsonResponse Reference to a VALID `JsonObject` object
 * that will hold the reader's ATR attribute (if any card is inserted,
 * otherwise empty text) under the predefined "d" (data) key.
 * @param[in] database Reference to a VALID and CONSTANT `SCardReaderDB` object
 * that holds the states of plugged-in Smart Card Readers.
 * @param[in] context A handle that identifies the resource manager context.
 * @return `TRUE` when a connection was successfully established,
 * `FALSE` on invalid parameters OR on any internal Smart Card error.
 */
extern BOOL
WebCard_tryConnectingToReader(
  _In_ const JsonObject *jsonRequest,
  _Inout_ JsonObject *jsonResponse,
  _In_ const SCardReaderDB *database,
  _In_ const SCARDCONTEXT context);

/**
 * @brief Executes one of the main WebCard commands, which attempts
 * to close the connection from OS to the selected Smart Card Reader.
 *
 * @param[in] jsonRequest Reference to a VALID and CONSTANT `JsonObject` object
 * that contains the Smart Card Reader Index ("r") key.
 * @param[in] database Reference to a VALID and CONSTANT `SCardReaderDB` object
 * that holds the states of plugged-in Smart Card Readers.
 * @return `TRUE` when the connection was closed,
 * `FALSE` on invalid parameters.
 */
extern BOOL
WebCard_tryDisconnectingFromReader(
  _In_ const JsonObject *jsonRequest,
  _In_ const SCardReaderDB *database);

/**
 * @brief Executes one of the main WebCard commands, which attempts to transmit
 * and receive APDUs between the OS and the selected Smart Card Reader.
 *
 * @param[in] jsonRequest Reference to a VALID and CONSTANT `JsonObject` object
 * that contains the Smart Card Reader Index ("r") key, and the
 * Application Prodotol Data Unit ("APDU") hex-string under the "a" key.
 * @param[in,out] jsonResponse Reference to a VALID `JsonObject` object
 * that will hold the Smart Card's APDU response under the "d" (data) key.
 * @param[in] database Reference to a VALID and CONSTANT `SCardReaderDB` object
 * that holds the states of plugged-in Smart Card Readers.
 * @return `TRUE` on success, `FALSE` on invalid parameters
 * OR on memory allocation error OR on any internal Smart Card error.
 */
extern BOOL
WebCard_transmitAndReceive(
  _In_ const JsonObject *jsonRequest,
  _Inout_ JsonObject *jsonResponse,
  _In_ const SCardReaderDB *database);

/**
 * @brief Sends selected Reader Event to the Standard Output.
 *
 * The Reader Event can contain information about:
 * -> new card connected to some given reader (index, ATR);
 * -> card disconnected from some given reader (index);
 * -> list of freshly connected readers (usually just one name, because
 *  readers-list fetching happens constantly with short intervals);
 * -> list of freshly disconnected readers (usually one name);
 *
 * @param[in] readerState Reference to a read-only Reader State,
 * that contains the "Answer To Reset" property (`->rgbAtr`).
 * This parameter is optional (can be `NULL`) for reader events
 * other than "Card Insertion".
 * @param[in] readerIndex Zero-based index that identifies Smart Card Reader
 * in current database. This parameter has no meaning for events other than
 * "Card Insertion" and "Card Removal". It is ignored if `reader` is `NULL`.
 * @param[in] readerEvent Type of the event fired from WebCard
 * to the Standard Output;
 * @param[out] jsonResponse Reference to an UNITIALIZED `JsonObject` variable
 * that will hold the JSON Response (output).
 * @param[in] jsonEventDetails Reference to a VALID and CONSTANT `JsonArray`
 * object, that holds the names of affected Smard Card Readers. It has
 * no meaning for events other than "More Readers" and "Less Readers".
 * This parameter is optional (can be `NULL`).
 *
 * @note `jsonResponse` must be released by the caller.
 */
extern VOID
WebCard_sendReaderEvent(
  _In_opt_ const SCARD_READERSTATE *readerState,
  _In_ const size_t readerIndex,
  _In_ const int readerEvent,
  _Out_ JsonObject *jsonResponse,
  _In_opt_ const JsonArray *jsonEventDetails);

/**
 * @brief Checks if any Reader changed status (ICC connected/disconnected),
 * then sends a Reader Event to Standard Output.
 *
 * @param[in,out] database Reference to a VALID `SCardReaderDB` object.
 * @param[in] context A handle that identifies the resource manager context.
 */
extern VOID
WebCard_handleStatusChange(
  _Inout_ SCardReaderDB *database,
  _In_ const SCARDCONTEXT context);


/**************************************************************/

#ifdef __cplusplus
  }
#endif

#endif  /* H_WEBCARD__SMART_CARDS */
