#include "Stack.h"
#include "stdlib.h"
#include "Stack_Private.h"

const size_t MIN_STACK_SZ = 8;
extern const size_t STACK_CANARY_SZ;

#undef STACK_CHECK
#define STACK_CHECK(stack) if(stack_check(stack) != STACK_ERRNO) return STACK_VALID_FAIL

STACK_ERROR stack_init(Stack *stack){
    STACK_CHECK_NULL(stack);

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

//-------------------------------------------------------------------------------------------------------------------

#ifdef STACK_EXTRA_INFO
STACK_ERROR stack_extra_init(Stack* stack, const char* init_var_name, int line, const char* file){
    stack_checkNULL(stack);
    stack->init_var_name = init_var_name;
    stack->init_line = line;
    stack->init_file = file;
    return stack_init(stack);
}
#endif
//----------------------------------------------------------------------------------------------------------------------

void stack_free(Stack *stack){
    stack_checkNULL(stack);
    if(stack->raw_data != NULL){
        free(stack->raw_data);
        stack->raw_data = NULL;
        stack->data = NULL;
        stack->size = 0;
        stack->capacity = 0;

#if (STACK_PROTECTION_LEVEL) & STACK_HASH_CHECK
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
    STACK_CHECK(stack);

    if(stack->size == 0){
        STACK_RAISE(STACK_EMPTY_POP);
    }

    stack->data[--stack->size] = 0;     //Clears value and moves size to previous position. Prefix decrement is important.
    stack_reHash(stack);

    if(4 * stack->size < stack->capacity && stack->capacity > 4 * MIN_STACK_SZ && stack->reserved <= stack->capacity / 2)
        return stack_realloc(stack, stack->capacity / 2);

    return STACK_ERRNO;
}

//----------------------------------------------------------------------------------------------------------------------

STACK_ERROR stack_push(Stack *stack, stack_element_t val){
    STACK_CHECK(stack);
    STACK_ERROR error = STACK_ERRNO;
    if(stack->size == stack->capacity - 1){             //Expanding stack
        error =  stack_realloc(stack, stack->capacity * 2);
        if(error == STACK_BAD_REALLOC){
            return STACK_BAD_REALLOC;
        }
    }

    stack->data[stack->size++] = val;

    stack_reHash(stack);
    return error;
}

//----------------------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------------------------

void stack_dump(const Stack *stack){
   /* LOG_DEBUG("Dumping stack information:\n");
    LOG_DEBUG_F("Address: %p\n", stack);
    if(stack == NULL) return;
#ifdef STACK_EXTRA_INFO
    LOG_DEBUG_F("Stack inited with name: %s\n", stack->init_var_name);
    LOG_DEBUG_F("Stack inited in file %s on line: %i\n", stack->init_file, stack->init_line);
#endif
    LOG_DEBUG_F("Capacity: %zu\n", stack->capacity);
    LOG_DEBUG_F("Number of elements: %zu\n", stack->size);
    LOG_DEBUG_F("Data address: %p\n", stack->data);

    if(stack->data == NULL){
        LOG_DEBUG("Stack data is null. No more dumping");
        return;
    }

    stack_dump_data(stack);*/

    if (stack == NULL){
        LOG_DEBUG_F("Stack [%p];", stack);
        return;
    }
    LOG_DEBUG_F("Stack \"%s\" at \"%s\" [%p]{\n", stack->init_var_name, stack->init_file, stack);
    LOG_DEBUG_F("\t.size = %zu\n", stack->size);
    LOG_DEBUG_F("\t.capacity = %zu\n", stack->capacity);

    LOG_DEBUG_F("\t.raw_data = %p\n", stack->raw_data);
    LOG_DEBUG_F("\t.data[%p] = {\n", stack->data);

    for(size_t i  = 0; i < stack->capacity; ++i){
        LOG_DEBUG_F("\t\t[%03zu] = ", i);
        LOG_DEBUG_F2(stack_element_format, stack->data[i]);
        LOG_DEBUG_F2("\n");
    }
    LOG_DEBUG_F("\t}\n");
    LOG_DEBUG_F("}\n");

}

STACK_ERROR stack_reserve(Stack *stack, size_t to_reserve){
    STACK_CHECK(stack);
    if(stack->reserved < to_reserve)
        stack->reserved = to_reserve;
    if(stack->reserved > stack->capacity){
        return stack_realloc(stack, stack->reserved);
    }
    return STACK_ERRNO;
}
