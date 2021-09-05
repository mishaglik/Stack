#ifndef STACK_STACK_H
#define STACK_STACK_H
#include "stdio.h"

#ifdef STACK_USE_INT
typedef int stack_element_t;
#elifdef STACK_USE_DOUBLE
typedef double stack_element_t;
#elifdef STACK_USE_PTR
typedef void* stack_element_t;
#else
typedef int stack_element_t;
#endif

typedef unsigned int hash_t;

enum STACK_ERROR{
    STACK_ERRNO,                //No error
    STACK_NULL,                 //stack == NULL
    STACK_UNINITIALIZED,
    STACK_OVERFLOW,             //Writing to unexpanded stack
    STACK_DATA_CORRUPTED,       //
    STACK_SIZE_CORRUPTED,
    STACK_BAD_ALLOC,
    STACK_BAD_REALLOC,
    STACK_WRONG_REALLOC,
    STACK_WRONG_SHRINK,
    STACK_WRONG_EXPAND,
    STACK_BAD_STATUS,
    STACK_REINIT,
    STACK_EMPTY_POP,
    STACK_EMPTY_GET,
    STACK_INFO_CORRUPTED,
    STACK_REFREE,
};

struct Stack{
    stack_element_t* data = NULL;
    size_t capacity = 0;
    size_t size = 0;
    STACK_ERROR error = STACK_UNINITIALIZED;
    hash_t infoHash = 0;
    hash_t dataHash = 0;
};

/*!
 * Inits stack if it wasn't initialized before.
 * @param stack - stack to init
 */
void stack_init(Stack* stack);

/*!
 * Frees place taken by stack.
 * @param stack
 */
void stack_free(Stack* stack);

/*!
 * Pushes value to top of stack.
 * @param stack - stack
 * @param val - value to push
 */
void stack_push(Stack* stack, stack_element_t val);

/*!
 * Returns top element of stack.
 * @param stack
 * @return value of top element
 */
stack_element_t stack_top(Stack* stack);

/*!
 * Removes top element from stack.
 * @param stack
 */
void stack_pop(Stack* stack);

/*!
 * Expands stack's size in 2 time.
 * @param stack
 */
void stack_expand(Stack* stack);

/*!
 * Shrinks stack's size to free extra memory
 * @param stack
 */
void stack_shrink(Stack* stack);

/*!
 * Reallocs size to fit new capacity
 * @param stack
 * @param new_capacity
 */
void stack_realloc(Stack* stack, size_t new_capacity);

#endif //STACK_STACK_H
