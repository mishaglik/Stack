#ifndef STACK_STACK_PRIVATE_H
#define STACK_STACK_PRIVATE_H
#include "stdio.h"
#include "Stack.h"


#ifndef STACK_NO_LOG
    //Checking log inited
    #if !(defined(LOG_MESSAGE) && defined(LOG_ASSERT))
    #error No logger defined
    #endif
#else
#define LOG_MESSAGE
#include "assert.h"
#define LOG_ASSERT(x) assert(x);
#endif

#define STACK_RAISE(error) stack_log_error(error); return error //Logs error and returns error code;
#define STACK_WARN(error)  stack_log_error(error)               //Only logs error (usually warning)

#define STACK_CHECK_NULL(stack) if(stack == NULL){stack_log_error(STACK_NULL); LOG_RAISE(FATAL) ;return STACK_NULL;}

#if (STACK_PROTECTION_LEVEL) & STACK_CANARY_CHECK
const size_t STACK_CANARY_SZ = 4;    //Amount of canary values.
#else
const size_t STACK_CANARY_SZ = 0;
#endif

/*!
 * Raises found error. Log it's data and aborts if necessary
 * @param error - error to raise
 */
void stack_log_error(const STACK_ERROR error);

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

#if STACK_PROTECTION_LEVEL & STACK_HASH_CHECK
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
#endif

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
 * Reallocs size to fit new capacity
 * @param stack
 * @param new_capacity
 */
STACK_ERROR stack_realloc(Stack* stack, size_t new_capacity);

/*!
 * Return level of error;
 * @param error
 * @return
 */
ErrorLevel stack_get_ErrorLevel(STACK_ERROR error);
#endif //STACK_STACK_PRIVATE_H
