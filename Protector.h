#ifndef STACK_PROTECTOR_H
#define STACK_PROTECTOR_H
#include "stdio.h"
#include "Stack.h"


#ifndef STACK_NO_LOG
    //Checking log inited
    #if !(defined(LOG_INFO) && defined(LOG_WARNING) && defined(LOG_ERROR) && defined(LOG_FATAL) && defined(LOG_ASSERT))
    #error No logger defined
    #endif

#else
#define LOG_INFO
#define LOG_WARNING
#define LOG_ERROR
#define LOG_FATAL(x) abort();
#include "assert.h"
#define LOG_ASSERT(x) assert(x);
#define LOG_DEBUG
#define LOG_DEBUG_F
#define LOG_DEBUG_F2
#endif

#define STACK_RAISE(error) stack_raise(error); return error //Logs error and returns error code;
#define STACK_WARN(error)  stack_raise(error)               //Only logs error (usually warning)

#if STACK_PROTECTION_LEVEL & STACK_CANARY_CHECK
const size_t STACK_CANARY_SZ = 4;    //Amount of canary values.
#else`
const size_t STACK_CANARY_SZ = 0;
#endif

/*!
 * Raises found error. Log it's data and aborts if necessary
 * @param error - error to raise
 */
void stack_raise(const STACK_ERROR error);

/*!
 * Checks if pointer to stack is not NULL; If stack is NULL generates FATAL.
 * @param stack --^
 */
void stack_checkNULL(const Stack *stack);

/*!
 * Checks stack on errors.
 * @param stack
 */
STACK_ERROR stack_check(Stack *stack);

/*!
 * Checks if stack is initialized;
 * @param stack
 * @return 1 if init, 0 otherwise
 */
int stack_is_init(const Stack* stack);

/*!
 * Checks stack's initialization. Raises FATAL in case of uninitialization.
 * @param stack
 */
void stack_check_init(Stack* stack);

/*!
 * Counts hash of stack's data.
 * @param stack
 * @return hash
 */
hash_t stack_data_hash(const Stack* stack);

/*!
 * Counts hash of stack's internal information.
 * @param stack
 * @return hash
 */
hash_t stack_info_hash(const Stack* stack);

/*!
 * Counts hash of array of data using algorithm. Uses algorithm ROT13
 * @param array - array to hash
 * @param size - size of array
 * @return
 */
hash_t hashROT13(const unsigned char *array, const size_t size);

/*!
 * Updates stack's hashes.
 * @param stack
 */
void stack_reHash(Stack* stack);

/*!
 * Checks if canary is alive. Causes error.
 * @param stack
 */
int stack_check_canary(Stack* stack);

/*!
 * Places canary in stack.
 * @param stack
 */
void stack_place_canary(Stack* stack);

/*!
 * Dumps stack data to log. !Works if data is correct!.
 * @param stack
 */
void stack_dump_data(const Stack *stack);

#endif //STACK_PROTECTOR_H
