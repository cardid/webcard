/**
 * @file "native/src/json/json.h"
 * Definitions and declarations for handling JSON data.
 */

#ifndef H_WEBCARD__JSON
#define H_WEBCARD__JSON

#include "os_specific/os_specific.h"
#include "misc/misc.h"
#include "utf/utf.h"

#ifdef __cplusplus
extern "C" {
#endif


/**************************************************************/
/* JSON BYTE STREAM                                           */
/**************************************************************/

/**
 * Possible return values for `JsonByteStream_loadFromStandardInput` function.
 */

  /** Bytes successfully loaded from STDIN stream. */
  #define JSON_STREAM_STATUS__VALID   0

  /** No data available on STDIN stream. */
  #define JSON_STREAM_STATUS__EMPTY   1

  /** No more bytes, loading error, memory alocation error. */
  #define JSON_STREAM_STATUS__NO_MORE  2

/**
 * `JsonByteStream` type definition.
 */
typedef struct JsonByteStream JsonByteStream;

/**
 * A stream of bytes (UTF-8 encoding), which are expected to
 * represent a valid stringified JSON.
 */
struct JsonByteStream
{
  /** Number of bytes present in the stream */
  size_t head_length;

  /** Bytes passed from Standard Input (dynamic allocation) */
  LPBYTE head;

  /** Number of incoming bytes */
  size_t tail_length;

  /** Bytes left to process */
  LPBYTE tail;
};

/**
 * @brief Prepares a `JsonByteStream` object to parse a stringified JSON.
 *
 * Checks if there is any data incoming on Standard Input. If so, attempts
 * to read the STDIN contents and store them for further processing.
 * @param[out] stream Reference to an UNINITIALIZED `JsonByteStream` object.
 * @return `JSON_STREAM_STATUS__VALID` if the stream is allocated and ready,
 * otherwise the object is left uninitialized.
 */
int
JsonByteStream_loadFromStandardInput(
  _Out_ JsonByteStream *stream);

/**
 * @brief `JsonByteStream` destructor.
 *
 * @param[in,out] stream Reference to a VALID `JsonByteStream` object.
 */
extern VOID
JsonByteStream_destroy(
  _Inout_ JsonByteStream *stream);

/**
 * @brief Peeks next byte without removing it from the stream.
 *
 * @param[in] source Reference to a VALID and CONSTANT `UTF8String` object
 * @param[out] byteRef Address that will hold the next byte.
 * @return `TRUE` on success, `FALSE` if there are no more bytes left to read.
 */
extern BOOL
JsonByteStream_peek(
  _In_ const JsonByteStream *stream,
  _Out_ BYTE *byteRef);

/**
 * @brief Removes (advances forward) `n` bytes from the stream.
 *
 * @param[in,out] stream Reference to a VALID `JsonByteStream` object.
 * @param[in] count Number of bytes to skip.
 */
extern VOID
JsonByteStream_skip(
  _Inout_ JsonByteStream *stream,
  _In_ const size_t count);

/**
 * @brief Copies `n` bytes from the stream, advancing the `tail` pointer.
 *
 * @param[in,out] stream Reference to a VALID `JsonByteStream` object.
 * @param[out] output Byte array in which the read bytes will be stored.
 * @param[in] count How many bytes should be read.
 * @return `TRUE` on success, `FALSE` if there are
 * not enough bytes left to read.
 */
extern BOOL
JsonByteStream_read(
  _Inout_ JsonByteStream *stream,
  _Out_ BYTE *output,
  _In_ const size_t count);

/**
 * @brief Removes any whitespace characters from the stream.
 *
 * Skips bytes in the stream, as long as the `tail` points
 * to a whitespace character.
 * @param[in,out] stream Reference to a VALID `JsonByteStream` object.
 * @param[out] output Byte array in which the read bytes will be stored.
 * @param[in] count How many bytes should be read.
 * @return `TRUE` if the `tail` points to a non-whitespace character,
 * `FALSE` if there are no more bytes left to read.
 */
extern BOOL
JsonByteStream_skipWhitespace(
  _Inout_ JsonByteStream *stream);


/**************************************************************/
/* JSON STRING                                                */
/**************************************************************/

/**
 * @brief Loads `UTF8String` object by parsing it's UTF-8
 * (stringified JSON) representation.
 *
 * Given stream should start with quotation mark (whitespace already skipped).
 * "JSON String" should be presented according to the JSON specification.
 * @param[out] result Points to a memory location that will hold
 * a new `UTF8String` object. `result` is always a VALID pointer,
 * while `result[0]` depends on the `allocate` argument.
 * @param[in] allocate Set to `TRUE` if a memory block should be allocated for
 * `UTF8String` object and that memory block should be placed at `result[0]`;
 * `FALSE` if `result[0]` already points to a valid (stack or allocated) memory.
 * @param[in,out] stream Reference to a VALID `JsonByteStream` object.
 * @return `TRUE` on success, `FALSE` on memory allocation failure OR
 * on any parsing error (incorrect `UTF8String` representation).
 *
 * @note After this call, `result[0]` will hold a VALID (at least initialized)
 * `UTF8String` object. If the function returned `FALSE`, `result[0]` should be
 * checked for `NULL` and the object shall be destroyed.
 */
extern BOOL
JsonString_parse(
  _Outptr_result_maybenull_ UTF8String **const result,
  _In_ const BOOL allocate,
  _Inout_ JsonByteStream *stream);

/**
 * @brief Saves `UTS8String` object to it's UTF-8
 * (stringified JSON) representation.
 *
 * The string representation of `UTF8String` object under the `string` argument
 * is appended to the `UTF8String` object under the `output` argument.
 * @param[in] string Reference to a VALID and CONSTANT `UTF8String` object.
 * @param[in,out] output Reference to a VALID `UTF8String` object.
 * @return `TRUE` on success, `FALSE` on memory allocation failure.
 */
extern BOOL
JsonString_toString(
  _In_ const UTF8String *string,
  _Inout_ UTF8String *output);


/**************************************************************/
/* JSON VALUE                                                 */
/**************************************************************/

/**
 * All possible "JSON Value" types.
 */

  #define JSON_VALUE_TYPE__NULL    0
  #define JSON_VALUE_TYPE__STRING  1
  #define JSON_VALUE_TYPE__NUMBER  2
  #define JSON_VALUE_TYPE__FALSE   3
  #define JSON_VALUE_TYPE__TRUE    4
  #define JSON_VALUE_TYPE__ARRAY   5
  #define JSON_VALUE_TYPE__OBJECT  6

/**
 * `JsonValue` type definition.
 */
typedef struct JsonValue JsonValue;

/**
 * Represents a dynamically-allocated JSON Value (unnamed JSON Object,
 * unnamed JSON Array, unnamed JSON String, unnamed JSON Number,
 * unnamed JSON Boolean).
 */
struct JsonValue
{
  /** Type of this value (determines which functions to use). */
  int type;

  /** Pointer to some data block (depends on the type). */
  void *value;
};

/**
 * @brief `JsonValue` constructor.
 *
 * @param[out] value Reference to an UNINITIALIZED `JsonValue` object.
 */
extern VOID
JsonValue_init(
  _Out_ JsonValue *value);

/**
 * @brief `JsonValue` destructor.
 *
 * @param[in,out] value Reference to a VALID `JsonValue` object.
 *
 * @note After this call, `value` should not be used (unless re-initialized).
 */
extern VOID
JsonValue_destroy(
  _Inout_ JsonValue *value);

/**
 * @brief Creates a deep-copy (independent memory allocation)
 * of the `JsonValue` object.
 *
 * @param[out] destination Reference to an UNINITIALIZED
 * `JsonValue` object (copy destination).
 * @param[in] source Reference to a VALID and CONSTANT `JsonValue` object
 * (copy source).
 * @return `TRUE` on success, `FALSE` on memory allocation failure.
 *
 * @note After this call, `destination` will hold a VALID (at least initialized)
 * `JsonValue` object. If the function returned `FALSE`,
 * `destination` shall be destroyed.
 */
extern BOOL
JsonValue_copy(
  _Out_ JsonValue *destination,
  _In_ const JsonValue *source);

/**
 * @brief Loads JSON Number by parsing it's stringified representation.
 *
 * Validates the string representation of a floating-point number.
 * Then, uses `strtof()` function to convert extracted text to a number.
 * @param[in,out] value Reference to an UNINITIALIZED `JsonValue` object.
 * @param[in,out] stream Reference to a VALID `JsonByteStream` object.
 * @return `TRUE` on success, `FALSE` on memory allocation failure OR
 * on invalid floating-point number text format.
 *
 * @note As for now, the stringified JSON Number cannot exists solely
 * at the end of `JsonByteStream`. The string representation must end on
 * a whitespace, a comma, a closing curly brace, or a closing square bracket.
 * @note After this call, `destination` will hold a VALID (at least initialized)
 * `JsonValue` object. If the function returned `FALSE`,
 * `destination` shall be destroyed.
 */
extern BOOL
JsonValue_parseNumber(
  _Inout_ JsonValue *value,
  _Inout_ JsonByteStream *stream);

/**
 * @brief Loads `JsonValue` object by parsing it's UTF-8
 * (stringified JSON) representation.
 *
 * Given stream can start with whitespace characters. Then, the first
 * non-whitespace character is tested to determine "JSON Value" type.
 * "JSON Value" should be presented according to the JSON specification.
 * @param[out] result Points to a memory location that will hold
 * a new `JsonValue` object. `result` is always a VALID pointer,
 * while `result[0]` depends on the `allocate` argument.
 * @param[in] allocate Set to `TRUE` if a memory block should be allocated for
 * `JsonValue` object and that memory block should be placed at `result[0]`;
 * `FALSE` if `result[0]` already points to a valid (stack or allocated) memory.
 * @param[in,out] stream Reference to a VALID `JsonByteStream` object.
 * @return `TRUE` on success, `FALSE` on memory allocation failure OR
 * on any parsing error (incorrect `JsonValue` representation).
 *
 * @note After this call, `result[0]` will hold a VALID (at least initialized)
 * `JsonValue` object. If the function returned `FALSE`, `result[0]` should be
 * checked for `NULL` and the object shall be destroyed.
 */
extern BOOL
JsonValue_parse(
  _Outptr_result_maybenull_ JsonValue **const result,
  _In_ const BOOL allocate,
  _Inout_ JsonByteStream *stream);

/**
 * @brief Saves `JsonValue` object to it's UTF-8
 * (stringified JSON) representation.
 *
 * The string representation of `JsonValue` object under the `value` argument
 * is appended to the `UTF8String` object under the `output` argument.
 * @param[in] value Reference to a VALID and CONSTANT `JsonValue` object.
 * @param[in,out] output Reference to a VALID `UTF8String` object.
 * @return `TRUE` on success, `FALSE` on memory allocation failure.
 */
extern BOOL
JsonValue_toString(
  _In_ const JsonValue *value,
  _Inout_ UTF8String *output);


/**************************************************************/
/* JSON ARRAY                                                 */
/**************************************************************/

/**
 * `JsonArray` type definition.
 */
typedef struct JsonArray JsonArray;

/**
 * JSON Array represents an array of ordered JSON Values.
 */
struct JsonArray
{
  /** Number of valid elements (`JsonValue` structures). */
  size_t count;

  /** Total number of allocated elements (`JsonValue` structures). */
  size_t capacity;

  /**
   * Contiguous (dynamically allocated) memory block of
   * `JsonValue` structures.
   */
  JsonValue *values;
};

/**
 * @brief `JsonArray` constructor.
 *
 * @param[out] value Reference to an UNINITIALIZED `JsonArray` object.
 */
extern VOID
JsonArray_init(
  _Out_ JsonArray *array);

/**
 * @brief `JsonArray` destructor.
 *
 * @param[in,out] value Reference to a VALID `JsonArray` object.
 *
 * @note After this call, `value` should not be used (unless re-initialized).
 */
extern VOID
JsonArray_destroy(
  _Inout_ JsonArray *array);

/**
 * @brief Creates a deep-copy (independent memory allocations)
 * of the `JsonArray` object.
 *
 * @param[out] destination Reference to an UNINITIALIZED
 * `JsonArray` object (copy destination).
 * @param[in] source Reference to a VALID and CONSTANT `JsonArray` object
 * (copy source).
 * @return `TRUE` on success, `FALSE` on memory allocation failure.
 *
 * @note After this call, `destination` will hold a VALID (at least initialized)
 * `JsonArray` object. If the function returned `FALSE`,
 * `destination` shall be destroyed.
 */
extern BOOL
JsonArray_copy(
  _Out_ JsonArray *destination,
  _In_ const JsonArray *source);

/**
 * @brief Appends `JsonValue` to `JsonArray` by performing a deep-copy.
 *
 * @param[in,out] array Reference to a VALID `JsonArray` object.
 * @param[in] value Reference to a VALID and CONSTANT `JsonValue` object.
 * @return `TRUE` on success, `FALSE` on memory allocation failure.
 *
 * @note Contents of the `value` argument can be temporary
 * (eg. prepared on the stack), because `JsonValue_copy` is called.
 */
extern BOOL
JsonArray_append(
  _Inout_ JsonArray *array,
  _In_ const JsonValue *value);

/**
 * @brief Loads `JsonArray` object by parsing it's UTF-8
 * (stringified JSON) representation.
 *
 * Given stream should start with a opening square bracket (whitespace
 * already skipped). "JSON Array" should be presented according
 * to the JSON specification.
 * @param[out] result Points to a memory location that will hold
 * a new `JsonArray` object. `result` is always a VALID pointer,
 * while `result[0]` depends on the `allocate` argument.
 * @param[in] allocate Set to `TRUE` if a memory block should be allocated for
 * `JsonArray` object and that memory block should be placed at `result[0]`;
 * `FALSE` if `result[0]` already points to a valid (stack or allocated) memory.
 * @param[in,out] stream Reference to a VALID `JsonByteStream` object.
 * @return `TRUE` on success, `FALSE` on memory allocation failure OR
 * on any parsing error (incorrect `JsonArray` representation).
 *
 * @note After this call, `result[0]` will hold a VALID (at least initialized)
 * `JsonArray` object. If the function returned `FALSE`, `result[0]` should be
 * checked for `NULL` and the object shall be destroyed.
 */
extern BOOL
JsonArray_parse(
  _Outptr_result_maybenull_ JsonArray **const result,
  _Inout_ JsonByteStream *stream);

/**
 * @brief Saves `JsonArray` object to it's UTF-8
 * (stringified JSON) representation.
 *
 * The string representation of `JsonArray` object under the `array` argument
 * is appended to the `UTF8String` object under the `output` argument.
 * @param[in] array Reference to a VALID and CONSTANT `JsonArray` object.
 * @param[in,out] output Reference to a VALID `UTF8String` object.
 * @return `TRUE` on success, `FALSE` on memory allocation failure.
 */
extern BOOL
JsonArray_toString(
  _In_ const JsonArray *array,
  _Inout_ UTF8String *output);


/**************************************************************/
/* JSON PAIR                                                  */
/**************************************************************/

/**
 * `JsonPair` type definition.
 */
typedef struct JsonPair JsonPair;

/**
 * Represents a dynamically-allocated Key-Value Pair
 * (JSON String with a JSON Value).
 */
struct JsonPair
{
  /** Key (pair identifier, should be unique within a single JSON Object). */
  UTF8String key;

  /** Value (what the key points to). */
  JsonValue value;
};

/**
 * @brief `JsonPair` constructor.
 *
 * @param[out] pair Reference to an UNINITIALIZED `JsonPair` object.
 */
extern VOID
JsonPair_init(
  _Out_ JsonPair *pair);

/**
 * @brief `JsonPair` destructor.
 *
 * @param[in,out] pair Reference to a VALID `JsonPair` object.
 *
 * @note After this call, `pair` should not be used (unless re-initialized).
 */
extern VOID
JsonPair_destroy(
  _Inout_ JsonPair *pair);

/**
 * @brief Creates a deep-copy (independent memory allocation)
 * of the `JsonPair` object.
 *
 * @param[out] destination Reference to an UNINITIALIZED
 * `JsonPair` object (copy destination).
 * @param[in] source Reference to a VALID and CONSTANT `JsonPair` object
 * (copy source).
 * @return `TRUE` on success, `FALSE` on memory allocation failure.
 *
 * @note After this call, `destination` will hold a VALID (at least initialized)
 * `JsonPair` object. If the function returned `FALSE`,
 * `destination` shall be destroyed.
 */
extern BOOL
JsonPair_copy(
  _Out_ JsonPair *destination,
  _In_ const JsonPair *source);

/**
 * @brief Loads `JsonPair` object by parsing it's UTF-8
 * (stringified JSON) representation.
 *
 * Given stream can start with whitespace characters. Then, there should be
 * a "JSON String", followed by a whitespace, then a colon, a whitespace, and
 * finally a "JSON Value", all presented according to the JSON specification.
 * @param[out] result Points to a memory location that will hold
 * a new `JsonPair` object. `result` is always a VALID pointer,
 * while `result[0]` depends on the `allocate` argument.
 * @param[in] allocate Set to `TRUE` if a memory block should be allocated for
 * `JsonPair` object and that memory block should be placed at `result[0]`;
 * `FALSE` if `result[0]` already points to a valid (stack or allocated) memory.
 * @param[in,out] stream Reference to a VALID `JsonByteStream` object.
 * @return `TRUE` on success, `FALSE` on memory allocation failure OR
 * on any parsing error (incorrect `JsonPair` representation).
 *
 * @note After this call, `result[0]` will hold a VALID (at least initialized)
 * `JsonPair` object. If the function returned `FALSE`, `result[0]` should be
 * checked for `NULL` and the object shall be destroyed.
 */
extern BOOL
JsonPair_parse(
  _Outptr_result_maybenull_ JsonPair **const result,
  _In_ const BOOL allocate,
  _Inout_ JsonByteStream *stream);

/**
 * @brief Saves `JsonPair` object to it's UTF-8
 * (stringified JSON) representation.
 *
 * The string representation of `JsonPair` object under the `pair` argument
 * is appended to the `UTF8String` object under the `output` argument.
 * @param[in] pair Reference to a VALID and CONSTANT `JsonPair` object.
 * @param[in,out] output Reference to a VALID `UTF8String` object.
 * @return `TRUE` on success, `FALSE` on memory allocation failure.
 */
extern BOOL
JsonPair_toString(
  _In_ const JsonPair *pair,
  _Inout_ UTF8String *output);


/**************************************************************/
/* JSON OBJECT                                                */
/**************************************************************/

/**
 * `JsonObject` type definition.
 */
typedef struct JsonObject JsonObject;

/**
 * JSON Object represents an object which contains
 * unordered JSON Key-Value pairs.
 */
struct JsonObject
{
  /** Number of valid elements (`JsonPair` structures). */
  size_t count;

  /** Total number of allocated elements (`JsonPair` structures). */
  size_t capacity;

  /**
   * Contiguous (dynamically allocated) memory block of
   * `JsonPair` structures.
   */
  JsonPair *pairs;
};

/**
 * @brief `JsonObject` constructor.
 *
 * @param[out] object Reference to an UNINITIALIZED `JsonObject` object.
 */
extern VOID
JsonObject_init(
  _Out_ JsonObject *object);

/**
 * @brief `JsonObject` destructor.
 *
 * @param[in,out] object Reference to a VALID `JsonObject` object.
 *
 * @note After this call, `object` should not be used (unless re-initialized).
 */
extern VOID
JsonObject_destroy(
  _Inout_ JsonObject *object);

/**
 * @brief Creates a deep-copy (independent memory allocation)
 * of the `JsonObject` object.
 *
 * @param[out] destination Reference to an UNINITIALIZED
 * `JsonObject` object (copy destination).
 * @param[in] source Reference to a VALID and CONSTANT `JsonObject` object
 * (copy source).
 * @return `TRUE` on success, `FALSE` on memory allocation failure.
 *
 * @note After this call, `destination` will hold a VALID (at least initialized)
 * `JsonObject` object. If the function returned `FALSE`,
 * `destination` shall be destroyed.
 */
extern BOOL
JsonObject_copy(
  _Out_ JsonObject *destination,
  _In_ const JsonObject *source);

/**
 * @brief Appends `JsonPair` to `JsonObject` by performing a deep-copy.
 *
 * @param[in,out] object Reference to a VALID `JsonObject` object.
 * @param[in] pair Reference to a VALID and CONSTANT `JsonPair` object.
 * @return `TRUE` on success, `FALSE` on memory allocation failure.
 *
 * @note Contents of the `pair` argument can be temporary
 * (eg. prepared on the stack), because `JsonPair_copy` is called.
 */
extern BOOL
JsonObject_appendPair(
  _Inout_ JsonObject *object,
  _In_ const JsonPair *pair);

/**
 * @brief Appends a Key-Value pair to `JsonObject` by performing a deep-copy.
 *
 * @param[in,out] object Reference to a VALID `JsonObject` object.
 * @param[in] key Case-sensitive and read-only UTF-8 text,
 * that describes some key. Must end with a NULL-terminator!
 * @param[in] value Reference to a VALID and CONSTANT `JsonValue` object.
 * @return `TRUE` on success, `FALSE` on memory allocation failure.
 *
 * @note Contents of the `value` argument can be temporary
 * (eg. prepared on the stack), because a temporary `JsonPair`
 * is passed to `JsonObject_appendPair` function.
 */
extern BOOL
JsonObject_appendKeyValue(
  _Inout_ JsonObject *object,
  _In_ LPCSTR key,
  _In_ const JsonValue *value);

/**
 * @brief Loads `JsonObject` object by parsing it's UTF-8
 * (stringified JSON) representation.
 *
 * Given stream should start with an opening curly brace (whitespace
 * already skipped). "JSON Object" should be presented
 * according to the JSON specification.
 * @param[out] result Points to a memory location that will hold
 * a new `JsonObject` object. `result` is always a VALID pointer,
 * while `result[0]` depends on the `allocate` argument.
 * @param[in] allocate Set to `TRUE` if a memory block should be allocated for
 * `JsonObject` object and that memory block should be placed at `result[0]`;
 * `FALSE` if `result[0]` already points to a valid (stack or allocated) memory.
 * @param[in,out] stream Reference to a VALID `JsonByteStream` object.
 * @return `TRUE` on success, `FALSE` on memory allocation failure OR
 * on any parsing error (incorrect `JsonObject` representation).
 *
 * @note After this call, `result[0]` will hold a VALID (at least initialized)
 * `JsonObject` object. If the function returned `FALSE`, `result[0]` should be
 * checked for `NULL` and the object shall be destroyed.
 */
extern BOOL
JsonObject_parse(
  _Outptr_result_maybenull_ JsonObject **const result,
  _In_ const BOOL allocate,
  _Inout_ JsonByteStream *stream);

/**
 * @brief Saves `JsonObject` object to it's UTF-8
 * (stringified JSON) representation.
 *
 * The string representation of `JsonObject` object under the `object` argument
 * is appended to the `UTF8String` object under the `output` argument.
 * @param[in] object Reference to a VALID and CONSTANT `JsonObject` object.
 * @param[in,out] output Reference to a VALID `UTF8String` object.
 * @return `TRUE` on success, `FALSE` on memory allocation failure.
 */
extern BOOL
JsonObject_toString(
  _In_ const JsonObject *object,
  _Inout_ UTF8String *output);

/**
 * Searches for a value tied with a given key in given JSON Object.
 *
 * @param[in] object Reference to a VALID and CONSTANT `JsonObject` object.
 * @param[out] result Reference to an UNINITIALIZED
 * (discardable) `JsonValue` object.
 * @param[in] key Case-sensitive and read-only UTF-8 text,
 * that describes some key. Must end with a NULL-terminator!
 * @return `TRUE` if given key was found (and `result` was updated),
 * otherwise `FALSE`.
 *
 * @note Structure pointed to by `result` is OVERWRITTEN on success and contains
 * only a READ-ONLY copy, that should not be directly modified / destructed.
 */
extern BOOL
JsonObject_getValue(
  _In_ const JsonObject *object,
  _Out_ JsonValue *result,
  _In_ LPCSTR key);


/**************************************************************/

#ifdef __cplusplus
}
#endif

#endif  /* H_WEBCARD__JSON */
