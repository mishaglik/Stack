#include "Stack_Private.h"
#include "Stack.h"
#include "string.h"

extern const canary_t STACK_CANARY_VALUE;

//----------------------------------------------------------------------------------------------------------------------
#define caseErr(error, msg) case error: LOG_MESSAGE(errorLevel, #error ": " msg); break
STACK_ERROR stack_log_error(const STACK_ERROR error){
    ErrorLevel errorLevel = stack_get_ErrorLevel(error);
    switch(error){
    case STACK_ERRNO:
        break;
    //###################### Fatals ####################################################
    caseErr(STACK_ANY_FATAL, "Something fatal occurred :(");
    caseErr(STACK_NULL, "Trying to access with NULL");
    caseErr(STACK_UNINITIALIZED, "Trying to use without init");
    caseErr(STACK_SIZE_CORRUPTED, "Stack size got inappropriate value");
    caseErr(STACK_INFO_CORRUPTED, "Internal stack information is corrupted");

    //###################### Errors #######################################################
    caseErr(STACK_ANY_ERROR, "Some error found during work");
    caseErr(STACK_CANARY_DEATH, "Stackoverflow - data goes over allowed");
    caseErr(STACK_DATA_CORRUPTED, "Found memory leak. Data probably corrupted");
    caseErr(STACK_BAD_ALLOC, "Initial memory allocation is unsuccessful");
    caseErr(STACK_EMPTY_GET, "Getting element from empty stack");

    //###################### Warnings ############################################################
    caseErr(STACK_ANY_WARNING, "Unknown warning so be warned");
    caseErr(STACK_BAD_REALLOC, "Reallocation is unsuccessful");
    caseErr(STACK_WRONG_REALLOC, "Inappropriate use of realloc. Size is more than new capacity");
    caseErr(STACK_VALID_FAIL, "stack_check() failed. See log before.");
    caseErr(STACK_REINIT, "Reinitializing of stack");
    caseErr(STACK_EMPTY_POP, "Called pop to empty stack");
    caseErr(STACK_REFREE, "Refreeing of stack");
    default:
        LOG_MESSAGE(errorLevel, "Unknown error");
    }
#ifndef STACK_NO_FAIL
    LOG_RAISE(errorLevel);
#endif
    return error;
}
#undef caseErr
//----------------------------------------------------------------------------------------------------------------------

STACK_ERROR stack_check(Stack *stack){
    if(stack == NULL){
        return stack_log_error(STACK_NULL);
    }

#if (STACK_PROTECTION_LEVEL) & STACK_VALID_CHECK
    if(!stack_is_init(stack)){
        return stack_log_error(STACK_UNINITIALIZED);
    }

    if((void*)stack->data != (void*)((canary_t*)stack->raw_data + STACK_CANARY_SZ / 2)){        //Check data and raw_data points to one memory
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
//Warning: Stack->infoHash must be ignored. So we have to drop const qualifier.
//In caused not to allow old stack hash to be part of new hash
hash_t stack_info_hash(const Stack *stack){
    LOG_ASSERT(stack != NULL);

    Stack* tmp_stack = (Stack*)stack;       //Dropping const qualifier
    hash_t exHash = tmp_stack->infoHash;
    tmp_stack->infoHash = 0;                //Clearing hash
    hash_t hash = hashROT13((const unsigned char *)tmp_stack, sizeof(stack));
    tmp_stack->infoHash = exHash;           //Return to beginning;

    return hash;
}
#endif
//----------------------------------------------------------------------------------------------------------------------
#if (STACK_PROTECTION_LEVEL) & STACK_HASH_CHECK
void stack_reHash(Stack *stack){
    LOG_ASSERT(stack != NULL);
    LOG_ASSERT(stack_is_init(stack));

    stack->dataHash = stack_data_hash(stack);
    stack->infoHash = stack_info_hash(stack);
}
#else
void stack_reHash(Stack *stack){}
#endif

//----------------------------------------------------------------------------------------------------------------------

#if (STACK_PROTECTION_LEVEL) & STACK_CANARY_CHECK
int stack_check_canary(Stack *stack){
    LOG_ASSERT(stack != NULL);
    LOG_ASSERT(stack_is_init(stack));

    canary_t local_canary_value = STACK_CANARY_VALUE ^ (canary_t)stack;
    size_t delta = STACK_CANARY_SZ / 2 * sizeof(canary_t) + stack->capacity * sizeof(stack_element_t);

    return (*(canary_t*) stack->raw_data            == local_canary_value &&
            *(canary_t*)(stack->raw_data + delta)   == local_canary_value &&
            stack->canary_beg                       == local_canary_value &&
            stack->canary_end                       == local_canary_value);
}

//----------------------------------------------------------------------------------------------------------------------

void stack_place_canary(Stack *stack){
    LOG_ASSERT(stack != NULL);
    LOG_ASSERT(stack_is_init(stack));

    canary_t local_canary_value = STACK_CANARY_VALUE ^ (canary_t)stack;
    size_t delta = STACK_CANARY_SZ / 2 * sizeof(canary_t) + stack->capacity * sizeof(stack_element_t);
    *(canary_t*)stack->raw_data             = local_canary_value;
    *(canary_t*)(stack->raw_data + delta)   = local_canary_value;
    stack->canary_beg                       = local_canary_value;
    stack->canary_end                       = local_canary_value;
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

    void* newData = realloc(stack->raw_data, (new_capacity) * sizeof(stack_element_t) + STACK_CANARY_SZ *  sizeof(canary_t));
    if(newData == NULL){
        return stack_log_error(STACK_BAD_REALLOC);
    }
    stack->raw_data = newData;
    stack->data = (stack_element_t*) (stack->raw_data + STACK_CANARY_SZ / 2 * sizeof(canary_t));

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
