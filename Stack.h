#ifndef STACK_STACK_H
#define STACK_STACK_H
#include "stdio.h"

//#define STACK_NO_LOG
#ifndef STACK_NO_LOG
#include "Logger/Logger.h"
#endif

#define STACK_NO_CHECK      0x0
#define STACK_VALID_CHECK   0x1
#define STACK_HASH_CHECK    0x2
#define STACK_CANARY_CHECK  0x4

#define STACK_ALL_CHECK     0x7

#ifndef STACK_PROTECTION_LEVEL
#define STACK_PROTECTION_LEVEL STACK_ALL_CHECK
#endif

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
    STACK_UNINITIALIZED,        //Operating of uninitialized stack
    STACK_CANARY_DEATH,             //Writing to unexpanded stack
    STACK_DATA_CORRUPTED,       //Found data corruption
    STACK_SIZE_CORRUPTED,       //Found size > capacity
    STACK_BAD_ALLOC,            //Error during   allocation of memory
    STACK_BAD_REALLOC,          //Error during REallocation of memory
    STACK_WRONG_REALLOC,        //Inappropriate call of realloc.
    STACK_WRONG_SHRINK,         //Inappropriate call of shrink. Eg: Not enough space to shrink
    STACK_WRONG_EXPAND,         //Inappropriate call of expand. Eg: Expanding of empty stack
    STACK_VALID_FAIL,           //Failed stack_check()
    STACK_REINIT,               //Trying to reInit stack
    STACK_EMPTY_POP,            //Popping from empty stack
    STACK_EMPTY_GET,            //Getting from empty stack
    STACK_INFO_CORRUPTED,       //Found corruption of internal information
    STACK_REFREE,               //Freeing of uninitialized stack
};

struct Stack{
    stack_element_t* data = NULL;
    stack_element_t* raw_data = NULL;

    size_t capacity = 0;
    size_t size = 0;

#if STACK_PROTECTION_LEVEL & STACK_HASH_CHECK
    hash_t infoHash = 0;
    hash_t dataHash = 0;
#endif
};

/*!
 * Inits stack if it wasn't initialized before.
 * @param stack - stack to init
 */
STACK_ERROR stack_init(Stack* stack);

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
STACK_ERROR stack_push(Stack* stack, stack_element_t val);

/*!
 * Returns top element of stack.
 * @param stack
 * @return value of top element
 */
STACK_ERROR stack_get(Stack *stack, stack_element_t *value);

/*!
 * Removes top element from stack.
 * @param stack
 */
STACK_ERROR stack_pop(Stack* stack);

/*!
 * Expands stack's size in 2 time.
 * @param stack
 */
STACK_ERROR stack_expand(Stack* stack);

/*!
 * Shrinks stack's size to free extra memory
 * @param stack
 */
STACK_ERROR stack_shrink(Stack* stack);

/*!
 * Reallocs size to fit new capacity
 * @param stack
 * @param new_capacity
 */
STACK_ERROR stack_realloc(Stack* stack, size_t new_capacity);

/*!
 * Dumps stack info to log.
 * @param stack
 */
void stack_dump(const Stack* stack);
#endif //STACK_STACK_H
