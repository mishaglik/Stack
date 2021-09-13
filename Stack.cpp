#include "Stack.h"
#include "stdlib.h"
#include "Protector.h"

const size_t MIN_STACK_SZ = 8;
extern const size_t STACK_CANARY_SZ;

#undef STACK_CHECK
#define STACK_CHECK(stack) if(stack_check(stack) != STACK_ERRNO) return STACK_VALID_FAIL

STACK_ERROR stack_init(Stack *stack){
    stack_checkNULL(stack);

    if(stack_is_init(stack)){
        STACK_RAISE(STACK_REINIT);
    }

    stack->raw_data = (stack_element_t*)calloc(MIN_STACK_SZ + STACK_CANARY_SZ, sizeof(stack_element_t));
    if(stack->raw_data == NULL) {
        STACK_RAISE(STACK_BAD_ALLOC);
    }

    stack->capacity = MIN_STACK_SZ;
    stack->size = 0;
    stack->data = stack->raw_data + STACK_CANARY_SZ / 2;

    stack_place_canary(stack);
    stack_reHash(stack);
    return STACK_ERRNO;
}

//----------------------------------------------------------------------------------------------------------------------

void stack_free(Stack *stack){
    stack_checkNULL(stack);
    if(stack->raw_data != NULL){
        free(stack->raw_data);
        stack->raw_data = NULL;
        stack->data = NULL;
        stack->size = 0;
        stack->capacity = 0;
#if STACK_PROTECTION_LEVEL & STACK_HASH_CHECK
        stack->infoHash = 0;
        stack->dataHash = 0;
#endif
    }
    else{
        STACK_WARN(STACK_REFREE);
    }
    return;
}

//----------------------------------------------------------------------------------------------------------------------

STACK_ERROR stack_expand(Stack *stack){
    STACK_CHECK(stack);

    if(2 * stack->size < stack->capacity){
        STACK_WARN(STACK_WRONG_EXPAND);
    }

    return stack_realloc(stack, stack->capacity * 2);
}

//----------------------------------------------------------------------------------------------------------------------

STACK_ERROR stack_shrink(Stack *stack){
    STACK_CHECK(stack);

    if(4 * stack->size > stack->capacity || stack->capacity < 4 * MIN_STACK_SZ){
        STACK_RAISE(STACK_WRONG_SHRINK);
    }
    return stack_realloc(stack, stack->capacity/2);
}

//----------------------------------------------------------------------------------------------------------------------

STACK_ERROR stack_get(Stack *stack, stack_element_t *value){
    STACK_CHECK(stack);
    LOG_ASSERT(value != NULL);

    if(stack->size == 0){
        STACK_RAISE(STACK_EMPTY_GET);
    }

    *value = stack->data[stack->size - 1];
    return STACK_ERRNO;
}

//----------------------------------------------------------------------------------------------------------------------

STACK_ERROR stack_pop(Stack *stack){
    if(stack_check(stack) != STACK_ERRNO)
        return STACK_VALID_FAIL;

    if(stack->size == 0){
        STACK_RAISE(STACK_EMPTY_POP);
    }

    stack->data[--stack->size] = 0;     //Clears value and moves size to previous position. Prefix decrement is important.
    stack_reHash(stack);

    if(4 * stack->size < stack->capacity && stack->capacity > 4 * MIN_STACK_SZ)
        return stack_shrink(stack);
    return STACK_ERRNO;
}

//----------------------------------------------------------------------------------------------------------------------

STACK_ERROR stack_push(Stack *stack, stack_element_t val){
    STACK_CHECK(stack);

    if(stack->size == stack->capacity - 1){             //Expanding stack
        if(stack_expand(stack) != STACK_ERRNO)
            return STACK_BAD_REALLOC;
    }

    stack->data[stack->size++] = val;

    stack_reHash(stack);
    return STACK_ERRNO;
}

//----------------------------------------------------------------------------------------------------------------------

STACK_ERROR stack_realloc(Stack *stack, size_t new_capacity){
    STACK_CHECK(stack);

    if(stack->size > new_capacity){
        STACK_RAISE(STACK_WRONG_REALLOC);
    }

    stack_element_t* newData = (stack_element_t*) realloc(stack->raw_data, (new_capacity + STACK_CANARY_SZ) * sizeof(stack_element_t));
    if(newData == NULL){
        STACK_RAISE(STACK_BAD_REALLOC);
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
void stack_dump(const Stack *stack){
    LOG_DEBUG("Dumping stack information:\n");
    LOG_DEBUG_F("Address: %p\n", stack);
    if(stack == NULL)
        return;
    LOG_DEBUG_F("Capacity: %zu\n", stack->capacity);
    LOG_DEBUG_F("Number of elements: %zu\n", stack->size);
    LOG_DEBUG_F("Data address: %p\n", stack->data);
    if(stack->data == NULL){
        LOG_DEBUG("Stack data is null. No more dumping");
        return;
    }
    stack_dump_data(stack);
}