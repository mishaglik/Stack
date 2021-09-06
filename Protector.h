#ifndef STACK_PROTECTOR_H
#define STACK_PROTECTOR_H
#include "stdio.h"
#include "Stack.h"
/*!
 * Raises found error. Log it's data and aborts if necessary
 * @param stack - stack, caused error
 * @param error - error to raise
 */
void stack_raise(Stack* stack, const STACK_ERROR error);

/*!
 * Checks if pointer to stack is not NULL; If stack is NULL generates FATAL.
 * @param stack --^
 */
void stack_checkNULL(const Stack *stack);

/*!
 * Checks stack on errors.
 * @param stack
 */
void stack_check(Stack *stack);

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
 * Checks bytes responsible for overflow. Causes error
 * @param stack
 */
void stack_check_overflow(Stack* stack);
#endif //STACK_PROTECTOR_H
