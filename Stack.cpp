#include "Stack.h"
#include "Stack_Private.h"

const size_t MIN_STACK_SZ = 8;
extern const size_t STACK_CANARY_SZ;

#ifdef STACK_META_INFORMATION
STACK_ERROR stack_init_meta(Stack* stack, Location location){
#else
STACK_ERROR stack_init(Stack *stack){
#endif
    STACK_CHECK_NULL(stack);

    if(stack_is_init(stack)){
        return stack_log_error(STACK_REINIT, stack);
    }
    #ifdef STACK_META_INFORMATION
        stack->location = location;
    #endif
    /**
     * @brief Using malloc because of untrivial expression.
     * @warning Do not forget sizeof(). It is malloc. 
     * On bug case come here.
     */
    stack->raw_data = malloc(MIN_STACK_SZ * sizeof(stack_element_t) + STACK_CANARY_SZ * sizeof(canary_t));
    if(stack->raw_data == NULL) {
        return stack_log_error(STACK_BAD_ALLOC, stack);
    }

    stack->capacity = MIN_STACK_SZ;
    stack->size = 0;
    stack->data = (stack_element_t*)((char*)stack->raw_data + STACK_CANARY_SZ / 2 * sizeof(canary_t));

    stack_place_canary(stack);
    stack_reHash(stack);

    STACK_CHECK(stack)
    return STACK_ERRNO;
}

//----------------------------------------------------------------------------------------------------------------------

void stack_free(Stack *stack){
    if(stack == NULL) return;
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
        stack_log_error(STACK_REFREE, stack);
    }
    return;
}

//----------------------------------------------------------------------------------------------------------------------

STACK_ERROR stack_get(Stack *stack, stack_element_t *value){
    STACK_CHECK(stack)
    LOG_ASSERT(value != NULL);

    if(stack->size == 0){
        return stack_log_error(STACK_EMPTY_GET, stack);
    }

    *value = stack->data[stack->size - 1];
    return STACK_ERRNO;
}

//----------------------------------------------------------------------------------------------------------------------

STACK_ERROR stack_remove(Stack *stack){
    STACK_CHECK(stack)

    if(stack->size == 0){
        return stack_log_error(STACK_EMPTY_POP, stack);
    }

    stack->data[--stack->size] = 0;     //Clears value and moves size to previous position. Prefix decrement is important.
    stack_reHash(stack);

    if(4 * stack->size < stack->capacity && stack->capacity > 4 * MIN_STACK_SZ && stack->reserved <= stack->capacity / 2)
        return stack_realloc(stack, stack->capacity / 2);

    STACK_CHECK(stack)
    return STACK_ERRNO;
}

//----------------------------------------------------------------------------------------------------------------------

STACK_ERROR stack_push(Stack *stack, stack_element_t val){
    STACK_CHECK(stack)
    STACK_ERROR error = STACK_ERRNO;
    if(stack->size == stack->capacity - 1){             //Expanding stack
        error =  stack_realloc(stack, stack->capacity * 2);
        if(error != STACK_ERRNO){
            return error;
        }
    }

    stack->data[stack->size++] = val;

    stack_reHash(stack);
    STACK_CHECK(stack)
    return error;
}

//----------------------------------------------------------------------------------------------------------------------

void stack_dump(const Stack *stack, Location location){
    LOG_MESSAGE_F(DEBUG, "Dumping variable \"%s\" in \"%s(%i)\" in file \"%s\":\n", location.var_name, location.func, location.line, location.filename);
    LOG_MESSAGE_F(DEBUG, "\n");
    if (stack == NULL){
        LOG_MESSAGE_F(DEBUG,"Stack [%p];", stack);
        return;
    }
#ifdef STACK_META_INFORMATION
    LOG_MESSAGE_F(DEBUG,"Stack \"%s\" born in \"%s(%i)\" in file: \"%s\" [%p]\n", stack->location.var_name, stack->location.func, stack->location.line , stack->location.filename, stack);
#else
    LOG_DEBUG_F("Stack [%p]{\n", stack);
#endif
    #if (STACK_PROTECTION_LEVEL) & STACK_CANARY_CHECK
        extern const canary_t STACK_CANARY_VALUE;
        canary_t local_canary_value = STACK_CANARY_VALUE ^ (canary_t)stack;
        LOG_MESSAGE_F(DEBUG, "\t.canary_beg = ");
        LOG_MESSAGE_F(NO_CAP, "0x%0llx", stack->canary_beg);
        LOG_MESSAGE_F(NO_CAP, "\t\t(%s),\n", (stack->canary_beg == local_canary_value ? "ok" : "ERROR"));
    #endif

    LOG_MESSAGE_F(DEBUG, "\t.size = %zu,\n", stack->size);
    LOG_MESSAGE_F(DEBUG, "\t.capacity = %zu,\n", stack->capacity);

    #if (STACK_PROTECTION_LEVEL) & STACK_HASH_CHECK
        LOG_MESSAGE_F(DEBUG, "\t.infoHash = 0x%0x\t\t\t\t(%s)\n", stack->infoHash,
                      (stack->infoHash == stack_info_hash(stack) ? "ok" : "ERROR"));
        LOG_MESSAGE_F(DEBUG, "\t.dataHash = 0x%0x\t\t\t\t(%s)\n", stack->dataHash,
                      (stack->dataHash == stack_data_hash(stack) ? "ok" : "ERROR"));
    #endif

    LOG_MESSAGE_F(DEBUG, "\t.raw_data = %p,\n", stack->raw_data);
    LOG_MESSAGE_F(DEBUG, "\t.data[%p] = {\n", stack->data);

    if( stack->data != NULL && stack->raw_data != NULL //Check if stack->data correct
        #if (STACK_PROTECTION_LEVEL) & STACK_HASH_CHECK
        && stack->infoHash == stack_info_hash(stack)
        #endif
        )
    {
//###################################### Data dumping begin ############################################################
        #if (STACK_PROTECTION_LEVEL) & STACK_CANARY_CHECK
            LOG_MESSAGE_F(DEBUG, "\t\t.canary_beg= ");
            LOG_MESSAGE_F(NO_CAP, "0x%0llx", *(canary_t*)stack->raw_data);
            LOG_MESSAGE_F(NO_CAP, "\t(%s),\n", (stack->canary_end == local_canary_value ? "ok" : "ERROR"));
        #endif

        for (size_t i = 0; i < stack->capacity; ++i){
            LOG_MESSAGE_F(DEBUG, "\t\t[%03zu] = ", i);
            LOG_MESSAGE_F(NO_CAP, stack_element_format, stack->data[i]);
            if(i == stack->size - 1)
                LOG_MESSAGE_F(NO_CAP, " (<--LAST)");
            LOG_MESSAGE_F(NO_CAP, "\n");
        }

        #if (STACK_PROTECTION_LEVEL) & STACK_CANARY_CHECK
            LOG_MESSAGE_F(DEBUG, "\t\t.canary_end = ");
            LOG_MESSAGE_F(NO_CAP, "%0x%0llx", *(canary_t*)((char*) stack->raw_data + stack->capacity * sizeof(stack_element_t) + sizeof(canary_t)));
            LOG_MESSAGE_F(NO_CAP, "\t(%s),\n", (stack->canary_end == local_canary_value ? "ok" : "ERROR"));
        #endif
//###################################### Data dumping end ##############################################################
    }
    else{
        LOG_MESSAGE_F(DEBUG, "\tUnable to dump stack. Info is corrupted\n}\n");
    }
    LOG_MESSAGE_F(DEBUG, "\t}\n");

#if (STACK_PROTECTION_LEVEL) & STACK_CANARY_CHECK
    LOG_MESSAGE_F(DEBUG, "\t.canary_end = ");
    LOG_MESSAGE_F(NO_CAP, "%0x%0llx", stack->canary_end);
    LOG_MESSAGE_F(NO_CAP, "\t\t(%s),\n", (stack->canary_end == local_canary_value ? "ok" : "ERROR"));
#endif

    LOG_MESSAGE_F(DEBUG, "}\n");

}

//----------------------------------------------------------------------------------------------------------------------

STACK_ERROR stack_reserve(Stack *stack, size_t to_reserve){
    STACK_CHECK(stack)
    if(stack->reserved > to_reserve){
        stack->reserved = to_reserve;
        stack_reHash(stack);
    }
    if(stack->reserved > stack->capacity){
        return stack_realloc(stack, stack->reserved);
    }
    STACK_CHECK(stack);
    return STACK_ERRNO;
}

//------------------------------------------------------------------------------------------------------------------------

STACK_ERROR stack_pop(Stack* stack, stack_element_t* value){
    STACK_CHECK(stack);
    if(value != NULL){
        STACK_ERROR error = stack_get(stack, value);
        if(error)
            return error;
    }

    return stack_remove(stack);
}