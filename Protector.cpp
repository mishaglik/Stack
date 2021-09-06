#include "Protector.h"
#include "Stack.h"
#include "Logger.h"

#include "stdlib.h"
#include "assert.h"
#include "string.h"

void stack_checkNULL(const Stack *stack){
    if(stack == NULL){
        stack_raise(NULL, STACK_NULL);
    }
}

//----------------------------------------------------------------------------------------------------------------------

void stack_raise(Stack* stack, const STACK_ERROR error){
    stack_logger_init();

    ERROR_LEVEL errLevel = INFO;
    const char* errMsg = NULL;
    getErrorInfo(error, &errLevel, &errMsg);

    stack_log(errLevel, errMsg);

    if(stack != NULL){
        stack->error = error;
        stack_dump(stack);
    }
    if(errLevel >= ABORT_LEVEL){
        stack_logger_finish();
        abort();
    }
}

//----------------------------------------------------------------------------------------------------------------------

void stack_check(Stack *stack){
    stack_checkNULL(stack);

    if(!stack_is_init(stack))
        stack_raise(stack, STACK_UNINITIALIZED);

    if(stack->error != STACK_ERRNO)
        stack_raise(stack, STACK_BAD_STATUS);

    hash_t h = stack_info_hash(stack);
    if(stack_info_hash(stack) != stack->infoHash)
        stack_raise(stack, STACK_INFO_CORRUPTED);

    stack_check_overflow(stack);

    if(stack->data != stack->raw_data + 1)
        stack_raise(stack, STACK_INFO_CORRUPTED);

    if(stack_data_hash(stack) != stack->dataHash)
        stack_raise(stack, STACK_DATA_CORRUPTED);

    if(stack->size > stack->capacity || stack->capacity == 0)
        stack_raise(stack, STACK_SIZE_CORRUPTED);
}

//----------------------------------------------------------------------------------------------------------------------

int stack_is_init(const Stack *stack){
    stack_checkNULL(stack);
    return stack->error != STACK_UNINITIALIZED && stack->data != NULL;
}

//----------------------------------------------------------------------------------------------------------------------

hash_t hashROT13(const unsigned char *array, const size_t size){
    assert(array != NULL);

    hash_t hash = 0;
    for(size_t i = 0; i < size; ++i){
        hash += array[i];
        hash -= (hash << 13) | (hash >> 19);
    }
    return hash;
}

//----------------------------------------------------------------------------------------------------------------------

hash_t stack_data_hash(const Stack *stack){
    assert(stack != NULL);
    assert(stack->data != NULL);

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
    if(!stack_is_init(stack))
        stack_raise(stack, STACK_UNINITIALIZED);
}

//----------------------------------------------------------------------------------------------------------------------

void stack_check_overflow(Stack *stack){
    stack_checkNULL(stack);
    stack_check_init(stack);

    if(stack->raw_data[0] != 0 ||  stack->raw_data[stack->capacity + 1] != 0){
        stack_raise(stack, STACK_OVERFLOW);
    }
}


