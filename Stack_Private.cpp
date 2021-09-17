#include "Stack_Private.h"
#include "Stack.h"
#include "string.h"

const stack_element_t STACK_CANARY_VALUE = 0x12345; //TODO: More poisons.

//----------------------------------------------------------------------------------------------------------------------

STACK_ERROR stack_log_error(const STACK_ERROR error){
    ErrorLevel errorLevel = stack_get_ErrorLevel(error);
    switch(error){
    case STACK_ERRNO:
        break;
    case STACK_NULL:
        LOG_MESSAGE(errorLevel, "Trying to access with NULL");
        break;
    case STACK_UNINITIALIZED:
        LOG_MESSAGE(errorLevel, "Trying to use without init");
        break;
    case STACK_SIZE_CORRUPTED:
        LOG_MESSAGE(errorLevel, "Stack size got inappropriate value");
        break;
    case STACK_INFO_CORRUPTED:
        LOG_MESSAGE(errorLevel, "Internal stack information is corrupted");
        break;
    case STACK_ANY_FATAL:
        LOG_MESSAGE(errorLevel, "Something fatal occurred :(");


    case STACK_CANARY_DEATH:
        LOG_MESSAGE(errorLevel, "Stackoverflow - data goes over allowed");
        break;
    case STACK_DATA_CORRUPTED:
        LOG_MESSAGE(errorLevel, "Found memory leak. Data probably corrupted");
        break;
    case STACK_BAD_ALLOC:
        LOG_MESSAGE(errorLevel, "Initial memory allocation is unsuccessful");
        break;
    case STACK_EMPTY_GET:
        LOG_MESSAGE(errorLevel, "Getting element from empty stack");
        break;
    case STACK_ANY_ERROR:
        LOG_MESSAGE(errorLevel, "Some error found during work");


    case STACK_BAD_REALLOC:
        LOG_MESSAGE(errorLevel, "Reallocation is unsuccessful");
        break;
    case STACK_WRONG_REALLOC:
        LOG_MESSAGE(errorLevel, "Inappropriate use of realloc. Size is more than new capacity");
        break;
    case STACK_VALID_FAIL:
        LOG_MESSAGE(errorLevel, "stack_check() failed. See log before.");
        break;
    case STACK_REINIT:
        LOG_MESSAGE(errorLevel, "Reinitializing of stack");
        break;
    case STACK_EMPTY_POP:
        LOG_MESSAGE(errorLevel, "Called pop to empty stack");
        break;
    case STACK_REFREE:
        LOG_MESSAGE(errorLevel, "Refreeing of stack");
        break;
    case STACK_ANY_WARNING:
        LOG_MESSAGE(errorLevel, "Unknown warning so be warned");
    default:
        LOG_MESSAGE(errorLevel, "Unknown error");
    }
#ifndef STACK_NO_FAIL
    LOG_RAISE(errorLevel);
#endif
    return error;
}

//----------------------------------------------------------------------------------------------------------------------

STACK_ERROR stack_check(Stack *stack){
    if(stack == NULL){
        return stack_log_error(STACK_NULL);
    }

#if (STACK_PROTECTION_LEVEL) & STACK_VALID_CHECK
    if(!stack_is_init(stack)){
        return stack_log_error(STACK_UNINITIALIZED);
    }

    if(stack->data != stack->raw_data + STACK_CANARY_SZ / 2){
        return stack_log_error(STACK_VALID_FAIL);
    }
#endif

#if (STACK_PROTECTION_LEVEL) & STACK_HASH_CHECK
    if(stack_info_hash(stack) != stack->infoHash){
        return stack_log_error(STACK_INFO_CORRUPTED);
    }

    if(stack_data_hash(stack) != stack->dataHash){
        return stack_log_error(STACK_DATA_CORRUPTED);
    }
#endif

#if (STACK_PROTECTION_LEVEL) & STACK_CANARY_CHECK

    if(!stack_check_canary(stack)){
        return stack_log_error(STACK_CANARY_DEATH);
    }
#endif

#if (STACK_PROTECTION_LEVEL) & STACK_VALID_CHECK
    if(stack->size > stack->capacity || stack->capacity == 0){
        return stack_log_error(STACK_SIZE_CORRUPTED);
    }
#endif
    return STACK_ERRNO;
}

//----------------------------------------------------------------------------------------------------------------------

int stack_is_init(const Stack *stack){
    return stack != NULL && stack->data != NULL && stack->capacity != 0;
}

//----------------------------------------------------------------------------------------------------------------------

#if (STACK_PROTECTION_LEVEL) & STACK_HASH_CHECK

hash_t hashROT13(const unsigned char *array, const size_t size){
    LOG_ASSERT(array != NULL);

    hash_t hash = 0;
    for(size_t i = 0; i < size; ++i){
        hash += array[i];
        hash -= (hash << 13) | (hash >> 19);        //Magic hash numbers.
    }
    return hash;
}

//----------------------------------------------------------------------------------------------------------------------

hash_t stack_data_hash(const Stack *stack){
    LOG_ASSERT(stack != NULL);
    LOG_ASSERT(stack->data != NULL);

    size_t dataBytes = stack->capacity * sizeof(stack_element_t);
    return hashROT13((unsigned char *)stack->data, dataBytes);
}

//----------------------------------------------------------------------------------------------------------------------
//Warning: Stack->infoHash must be ignored.
//In caused not to allow old stack hash to be part of new hash
//TODO: FIX -^
hash_t stack_info_hash(const Stack *stack){
    LOG_ASSERT(stack != NULL);
    return 0;
//    return hashROT13((const unsigned char*)stack, sizeof(stack));
}
#endif
//----------------------------------------------------------------------------------------------------------------------
#if (STACK_PROTECTION_LEVEL) & STACK_HASH_CHECK
void stack_reHash(Stack *stack){

    LOG_ASSERT(stack != NULL);
    LOG_ASSERT(stack_is_init(stack));

    stack->infoHash = stack_info_hash(stack);
    stack->dataHash = stack_data_hash(stack);

}
#else
void stack_reHash(Stack *stack){}
#endif

//----------------------------------------------------------------------------------------------------------------------

#if (STACK_PROTECTION_LEVEL) & STACK_CANARY_CHECK
//TODO: Remake canary system
int stack_check_canary(Stack *stack){
    LOG_ASSERT(stack != NULL);
    LOG_ASSERT(stack_is_init(stack));

    return (stack->raw_data[0] == 0                     && stack->raw_data[stack->capacity + STACK_CANARY_SZ - 1] == 0 &&
            stack->raw_data[1] == STACK_CANARY_VALUE    && stack->raw_data[stack->capacity + STACK_CANARY_SZ - 2] == STACK_CANARY_VALUE);
}

//----------------------------------------------------------------------------------------------------------------------

void stack_place_canary(Stack *stack){
    LOG_ASSERT(stack != NULL);
    LOG_ASSERT(stack_is_init(stack));

    stack->raw_data[0] = 0;
    stack->raw_data[1] = STACK_CANARY_VALUE;
    stack->raw_data[stack->capacity + STACK_CANARY_SZ - 2] = STACK_CANARY_VALUE;
    stack->raw_data[stack->capacity + STACK_CANARY_SZ - 1] = 0;
}
#else
int stack_check_canary(Stack *stack){return 1;}
void stack_place_canary(Stack *stack){}
#endif

//----------------------------------------------------------------------------------------------------------------------

STACK_ERROR stack_realloc(Stack *stack, size_t new_capacity){
    STACK_CHECK(stack)
    if(stack->size > new_capacity){
        return stack_log_error(STACK_WRONG_REALLOC);
    }

    stack_element_t* newData = (stack_element_t*) realloc(stack->raw_data, (new_capacity + STACK_CANARY_SZ) * sizeof(stack_element_t));
    if(newData == NULL){
        return stack_log_error(STACK_BAD_REALLOC);
    }
    stack->raw_data = newData;
    stack->data = stack->raw_data + STACK_CANARY_SZ / 2;

    if(new_capacity > stack->capacity){
        stack_element_t* ptrBegin = stack->data + stack->capacity;
        stack_element_t* ptrEnd   = stack->data + new_capacity;
        while(ptrBegin < ptrEnd){
            *(ptrBegin++) = 0;
        }
    }
    stack->capacity = new_capacity;
    stack_place_canary(stack);

    stack_reHash(stack);
    return STACK_ERRNO;
}

//----------------------------------------------------------------------------------------------------------------------

ErrorLevel stack_get_ErrorLevel(STACK_ERROR error){
    if(error >= STACK_ANY_FATAL)
        return FATAL;
    if(error >= STACK_ANY_ERROR)
        return ERROR;
    if(error >= STACK_ANY_WARNING)
        return WARNING;
    return INFO;
}
