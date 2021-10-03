#ifndef STACK_STACK_H
#define STACK_STACK_H
#include "config.h"
#include "Loglib/Logger.h"

#ifdef STACK_NO_LOG
#undef LOG_MESSAGE
#define LOG_MESSAGE(...)
#endif
#if defined(STACK_NO_CHECK) || defined(STACK_VALID_CHECK) || defined(STACK_STACK_HASH_CHECKNO_CHECK) || defined(STACK_ALL_CHECK) 
    #error Define collision. Unable to compile.
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

#if defined(STACK_DUMP)
    #error Define collision. Unable to compile.
#endif
#define STACK_DUMP(stack) stack_dump(stack,LOCATION(stack))

typedef unsigned int hash_t;
#if STACK_PROTECTION_LEVEL & STACK_CANARY_CHECK
typedef u_int64_t canary_t;
#else
typedef void canary_t;
#endif

#ifdef STACK_META_INFORMATION
struct Location{
    const char* var_name;
    const char* func;
    const char* filename;
    int         line;
};
#define LOCATION(x) {#x, __func__, __FILE__, __LINE__}
#endif

struct Stack{
#if STACK_PROTECTION_LEVEL & STACK_CANARY_CHECK
    canary_t canary_beg = 0;
#endif
    stack_element_t* data     = NULL;
    void*            raw_data = NULL;

    size_t capacity = 0;
    size_t size     = 0;
    size_t reserved = 0;

#if STACK_PROTECTION_LEVEL & STACK_HASH_CHECK
    hash_t infoHash = 0;
    hash_t dataHash = 0;
#endif
#ifdef STACK_META_INFORMATION
    Location location  = {};
#endif
#if STACK_PROTECTION_LEVEL & STACK_CANARY_CHECK
    canary_t canary_end = 0;
#endif
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

/*!
 * Inits stack if it wasn't initialized before.
 * @param stack - stack to init
 */
#ifdef STACK_META_INFORMATION
#define stack_init(stack) stack_init_meta(stack, LOCATION(stack))
STACK_ERROR stack_init_meta(Stack* stack, Location location);
#else
STACK_ERROR stack_init(Stack* stack);
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
void stack_dump(const Stack *stack, Location location);
#endif //STACK_STACK_H
