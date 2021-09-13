#include "Protector.h"
#include "Stack.h"
#include "string.h"

const stack_element_t STACK_CANARY_VALUE = 0x12345;

void stack_checkNULL(const Stack *stack){
    if(stack == NULL){
        stack_raise(STACK_NULL);
    }
}

//----------------------------------------------------------------------------------------------------------------------

void stack_raise(const STACK_ERROR error){
    switch(error){
    case STACK_ERRNO:
        break;
    case STACK_NULL:
        LOG_FATAL("Trying to access with NULL");
        break;
    case STACK_UNINITIALIZED:
        LOG_FATAL("Trying to use without init");
        break;
    case STACK_SIZE_CORRUPTED:
        LOG_FATAL("Stack size got inappropriate value");
        break;
    case STACK_INFO_CORRUPTED:
        LOG_FATAL("Internal stack information is corrupted");
        break;
    case STACK_CANARY_DEATH:
        LOG_ERROR("Stackoverflow - data goes over allowed");
        break;
    case STACK_DATA_CORRUPTED:
        LOG_ERROR("Found memory leak. Data probably corrupted");
        break;
    case STACK_BAD_ALLOC:
        LOG_ERROR("Initial memory allocation is unsuccessful");
        break;
    case STACK_EMPTY_GET:
        LOG_ERROR("Getting element from empty stack");
        break;
    case STACK_BAD_REALLOC:
        LOG_WARNING("Reallocation is unsuccessful");
        break;
    case STACK_WRONG_REALLOC:
        LOG_WARNING("Inappropriate use of realloc. Size is more than new capacity");
        break;
    case STACK_VALID_FAIL:
        LOG_WARNING("stack_check() failed. See log before.");
        break;
    case STACK_REINIT:
        LOG_WARNING("Reinitializing of stack");
        break;
    case STACK_WRONG_SHRINK:
        LOG_WARNING("Wrong use of stack_shrink");
        break;
    case STACK_WRONG_EXPAND:
        LOG_WARNING("Wrong use of stack_expand");
        break;
    case STACK_EMPTY_POP:
        LOG_WARNING("Called pop to empty stack");
        break;
    case STACK_REFREE:
        LOG_WARNING("Refreeing of stack");
        break;
    default:
        LOG_ERROR("Unknown error");
    }
}

//----------------------------------------------------------------------------------------------------------------------

STACK_ERROR stack_check(Stack *stack){
    stack_checkNULL(stack);

    if(!stack_is_init(stack)){
        stack_raise(STACK_UNINITIALIZED);
        return STACK_UNINITIALIZED;
    }

    if(stack_info_hash(stack) != stack->infoHash){
        stack_raise(STACK_INFO_CORRUPTED);
        return STACK_INFO_CORRUPTED;
    }

    stack_check_canary(stack);

    if(stack->data != stack->raw_data + STACK_CANARY_SZ / 2)
        stack_raise(STACK_INFO_CORRUPTED);

    if(stack_data_hash(stack) != stack->dataHash)
        stack_raise(STACK_DATA_CORRUPTED);

    if(stack->size > stack->capacity || stack->capacity == 0)
        stack_raise(STACK_SIZE_CORRUPTED);
    return STACK_ERRNO;
}

//----------------------------------------------------------------------------------------------------------------------

int stack_is_init(const Stack *stack){
    stack_checkNULL(stack);
    return stack->data != NULL && stack->capacity != 0;
}

//----------------------------------------------------------------------------------------------------------------------

hash_t hashROT13(const unsigned char *array, const size_t size){
    LOG_ASSERT(array != NULL);

    hash_t hash = 0;
    for(size_t i = 0; i < size; ++i){
        hash += array[i];
        hash -= (hash << 13) | (hash >> 19);
    }
    return hash;
}

//----------------------------------------------------------------------------------------------------------------------

hash_t stack_data_hash(const Stack *stack){
    LOG_ASSERT(stack != NULL);
    LOG_ASSERT(stack->data != NULL);

    size_t dataBytes = stack->capacity * sizeof(stack_element_t) / sizeof (unsigned char);
    return hashROT13((unsigned char *)stack->data, dataBytes);
}

//----------------------------------------------------------------------------------------------------------------------

hash_t stack_info_hash(const Stack *stack){
    unsigned long long pre_hash = 0;
    pre_hash ^= (unsigned long long) stack->size;
    pre_hash ^= (unsigned long long) stack->capacity;
    pre_hash ^= (unsigned long long) stack->raw_data;
    return (hash_t) (pre_hash ^ (pre_hash >> sizeof (hash_t)));
}

//----------------------------------------------------------------------------------------------------------------------

void stack_reHash(Stack *stack){
    stack_checkNULL(stack);
    stack_check_init(stack);
    stack->infoHash = stack_info_hash(stack);
    stack->dataHash = stack_data_hash(stack);
}

//----------------------------------------------------------------------------------------------------------------------

void stack_check_init(Stack *stack){
    if(!stack_is_init(stack)){
        stack_raise(STACK_UNINITIALIZED);
    }
}

//----------------------------------------------------------------------------------------------------------------------

void stack_check_canary(Stack *stack){
    stack_checkNULL(stack);
    stack_check_init(stack);

    if( stack->raw_data[0] != 0                     || stack->raw_data[stack->capacity + STACK_CANARY_SZ - 1] != 0 ||
        stack->raw_data[1] != STACK_CANARY_VALUE    || stack->raw_data[stack->capacity + STACK_CANARY_SZ - 2] != STACK_CANARY_VALUE){
        stack_raise(STACK_CANARY_DEATH);
    }
}

//----------------------------------------------------------------------------------------------------------------------

void stack_dump_data(const Stack *stack){
    LOG_ASSERT(stack->data != NULL);
    if(stack->infoHash != stack_info_hash(stack)){
        LOG_DEBUG("Inner stack information is corrupted. Unable to dump data\n");
        return;
    }
#ifdef STACK_USE_INT
    const char* const format = "%i";
#elifdef STACK_USE_DOUBLE
    const char* const format = "%f";
#elifdef STACK_USE_PTR
    const char* const format = "%p";
#else
    const char* const format = "%i";
#endif

    LOG_DEBUG("\nDumping data:\n");
    for(size_t i = 0; i < stack->capacity; ++i){
        LOG_DEBUG_F("Ceil №%zu\tat address: %p\thas value:", i, stack->data + i);
        LOG_DEBUG_F2(format, stack->data[i]);
        LOG_DEBUG_F2("\n");
    }
    LOG_DEBUG("Dumping data end\n\n");
}

//----------------------------------------------------------------------------------------------------------------------

void stack_place_canary(Stack *stack){
    stack_checkNULL(stack);
    stack_check_init(stack);

    stack->raw_data[0] = 0;
    stack->raw_data[1] = STACK_CANARY_VALUE;
    stack->raw_data[stack->capacity + STACK_CANARY_SZ - 2] = STACK_CANARY_VALUE;
    stack->raw_data[stack->capacity + STACK_CANARY_SZ - 1] = 0;
}
