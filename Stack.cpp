#include "Stack.h"
#include "stdlib.h"
#include "Protector.h"

const size_t MIN_STACK_SZ = 8;
const size_t STACK_EXTRA_SZ = 2;    //First and last value

void stack_init(Stack *stack){
    stack_checkNULL(stack);
    if(stack->error != STACK_UNINITIALIZED){
        stack_raise(stack, STACK_REINIT);
        return;
    }

    stack->raw_data = (stack_element_t*)calloc(MIN_STACK_SZ + STACK_EXTRA_SZ, sizeof(stack_element_t));
    if(stack->raw_data == NULL) {
        stack_raise(stack, STACK_BAD_ALLOC);
        return;
    }

    stack->capacity = MIN_STACK_SZ;
    stack->size = 0;
    stack->data = stack->raw_data + 1;
    stack->error = STACK_ERRNO;

    stack_reHash(stack);
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
        stack->error = STACK_UNINITIALIZED;
        stack->infoHash = 0;
        stack->dataHash = 0;
    }
    else{
        stack_raise(stack, STACK_REFREE);
    }
}

//----------------------------------------------------------------------------------------------------------------------

void stack_expand(Stack *stack){
    stack_check(stack);

    if(2 * stack->size < stack->capacity){
        stack_raise(stack, STACK_WRONG_EXPAND);
        return;
    }

    stack_realloc(stack, stack->capacity * 2);
}

//----------------------------------------------------------------------------------------------------------------------

void stack_shrink(Stack *stack){
    stack_check(stack);

    if(4 * stack->size > stack->capacity || stack->capacity < 4 * MIN_STACK_SZ){
        stack_raise(stack, STACK_WRONG_SHRINK);
        return;
    }
    stack_realloc(stack, stack->capacity/2);
}

//----------------------------------------------------------------------------------------------------------------------

stack_element_t stack_top(Stack *stack){
    stack_check(stack);

    if(stack->size == 0){
        stack_raise(stack, STACK_EMPTY_GET);
        return 0;
    }

    return stack->data[stack->size - 1];
}

//----------------------------------------------------------------------------------------------------------------------

void stack_pop(Stack *stack){
    stack_check(stack);

    if(stack->size == 0){
        stack_raise(stack, STACK_EMPTY_POP);
        return;
    }

    stack->data[--stack->size] = 0;     //Clears value and moves size to previous position. Prefix decrement is important.
    stack_reHash(stack);

    if(4 * stack->size < stack->capacity && stack->capacity > 4 * MIN_STACK_SZ)
        stack_shrink(stack);
}

//----------------------------------------------------------------------------------------------------------------------

void stack_push(Stack *stack, stack_element_t val){
    stack_check(stack);

    if(stack->size == stack->capacity - 1){             //Expanding stack
        stack_expand(stack);
        if(stack->error == STACK_BAD_REALLOC){
            stack_raise(stack, STACK_OVERFLOW);
            return;
        }
    }

    stack->data[stack->size++] = val;

    stack_reHash(stack);
}

//----------------------------------------------------------------------------------------------------------------------

void stack_realloc(Stack *stack, size_t new_capacity){
    stack_check(stack);

    if(stack->size > new_capacity){
        stack_raise(stack, STACK_WRONG_REALLOC);
        return;
    }

    stack_element_t* newData = (stack_element_t*) realloc(stack->raw_data, (new_capacity + STACK_EXTRA_SZ) * sizeof(stack_element_t));
    if(newData == NULL){
        stack_raise(stack, STACK_BAD_REALLOC);
        return;
    }
    stack->raw_data = newData;
    stack->data = stack->raw_data + 1;

    if(new_capacity > stack->capacity){
        stack_element_t* ptrBegin = stack->data + stack->capacity;
        stack_element_t* ptrEnd   = stack->data + new_capacity;
        while(ptrBegin < ptrEnd){
            *(ptrBegin++) = 0;
        }
    }
    stack->raw_data[new_capacity + STACK_EXTRA_SZ - 1] = 0;
    stack->capacity = new_capacity;

    stack_reHash(stack);
}
