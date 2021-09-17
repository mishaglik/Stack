#ifndef STACK_STACK_H
#define STACK_STACK_H
#include "config.h"

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
const char* const stack_element_format = "%i";
#else
#ifdef STACK_USE_DOUBLE
typedef double stack_element_t;
const char* const stack_element_format = "%f";
#else
#ifdef STACK_USE_PTR
typedef void* stack_element_t;
const char* const stack_element_format = "%p";
#else
typedef int stack_element_t;
const char* const stack_element_format = "%i";
#endif
#endif
#endif

typedef unsigned int hash_t;

struct Stack{
    unsigned int canary_beg = 0;
#ifdef STACK_EXTRA_INFO
    const char* init_var_name = NULL;
    const char* init_file = NULL;
    int init_line = 0;
#endif
    stack_element_t* data = NULL;
    stack_element_t* raw_data = NULL;

    size_t capacity = 0;
    size_t size = 0;
    size_t reserved = 0;

#if (STACK_PROTECTION_LEVEL) & STACK_HASH_CHECK
    hash_t infoHash = 0;
    hash_t dataHash = 0;
#endif
    unsigned int canary_end = 0;
};

enum STACK_ERROR{
    STACK_ERRNO,                //No error

    STACK_ANY_WARNING,
    //Warnings goes here:
    STACK_REINIT,               //Trying to reInit stack
    STACK_EMPTY_POP,            //Popping from empty stack
    STACK_EMPTY_GET,            //Getting from empty stack
    STACK_WRONG_REALLOC,        //Inappropriate call of realloc.
    STACK_REFREE,               //Freeing of uninitialized stack

    STACK_ANY_ERROR,
    //Errors goes here:
    STACK_SIZE_CORRUPTED,       //Found size > capacity
    STACK_DATA_CORRUPTED,       //Found data corruption
    STACK_BAD_ALLOC,            //Error during   allocation of memory
    STACK_BAD_REALLOC,          //Error during REallocation of memory
    STACK_VALID_FAIL,           //Failed stack_check()

    STACK_ANY_FATAL,
    //Fatals goes here:
    STACK_NULL,                 //stack == NULL
    STACK_UNINITIALIZED,        //Operating of uninitialized stack
    STACK_CANARY_DEATH,         //Canary check failed
    STACK_INFO_CORRUPTED,       //Found corruption of internal information

};


#ifdef STACK_EXTRA_INFO
#define STACK_INIT(stack) stack_extra_init(stack, #stack, __LINE__, __FILE__)
#else
#define STACK_INIT(stack) stack_init(stack)
#endif

/*!
 * Inits stack if it wasn't initialized before.
 * @param stack - stack to init
 */
STACK_ERROR stack_init(Stack* stack);

#ifdef STACK_EXTRA_INFO
/*!
 * Inits stack with extra information
 * @param stack
 * @return Error occurred during initialization
 */
STACK_ERROR stack_extra_init(Stack* stack, const char* init_var_name, int line, const char* file);
#endif

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
 * Preserves stack capacity to [to_reserve]
 * @param stack
 * @param to_reserve
 * @return Error during preservation
 */
STACK_ERROR stack_reserve(Stack *stack, size_t to_reserve);

/*!
 * Dumps stack info to log.
 * @param stack
 */
void stack_dump(const Stack* stack);
#endif //STACK_STACK_H
