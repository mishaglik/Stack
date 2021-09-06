#include "string.h"
#include "time.h"
#include "assert.h"

#include "Stack.h"
#include "Logger.h"
#include "Protector.h"

FILE* LOG_FILE = NULL;
const char* DEFAULT_STACK_LOG = "stack.log";

void stack_logger_init(){
    if(LOG_FILE == NULL){
        LOG_FILE = fopen(DEFAULT_STACK_LOG, "a");
    }
    assert(LOG_FILE != NULL);
}

const char* getLevelMsg(ERROR_LEVEL errorLevel){
    switch(errorLevel){
    case INFO:
        return "INFO";
    case WARN:
        return "WARNING";
    case ERROR:
        return "ERROR";
    case FATAL:
        return "FATAL";
    default:
        return "OTHER";
    }
}

void stack_logger_finish(){
    if(LOG_FILE != NULL){
        fclose(LOG_FILE);
        LOG_FILE = NULL;
    }
}

void stack_dump(const Stack *stack){
    assert(LOG_FILE != NULL);
    fprintf(LOG_FILE, "Dumping stack information:\n");
    fprintf(LOG_FILE, "Address: %p\n", stack);
    fprintf(LOG_FILE, "Capacity: %zu\n", stack->capacity);
    fprintf(LOG_FILE, "Number of elements: %zu\n", stack->size);
    fprintf(LOG_FILE, "Current error code: %i\n", stack->error);
    fprintf(LOG_FILE, "Data address: %p\n", stack->data);
    stack_dump_data(stack);
}

void getErrorInfo(STACK_ERROR error, ERROR_LEVEL *errorLevel, const char **errorMsg){
    switch(error){
    case STACK_ERRNO:
        *errorLevel = INFO;
        *errorMsg   = "Debug";
        break;
    case STACK_NULL:
        *errorLevel = FATAL;
        *errorMsg   = "Trying to access with NULL";
        break;
    case STACK_UNINITIALIZED:
        *errorLevel = FATAL;
        *errorMsg   = "Trying to use without init";
        break;
    case STACK_OVERFLOW:
        *errorLevel = ERROR;
        *errorMsg   = "Stackoverflow - data goes over allowed";
        break;
    case STACK_DATA_CORRUPTED:
        *errorLevel = ERROR;
        *errorMsg   = "Found memory leak. Data probably corrupted";
        break;
    case STACK_SIZE_CORRUPTED:
        *errorLevel = FATAL;
        *errorMsg   = "Stack size got inappropriate value";
        break;
    case STACK_BAD_ALLOC:
        *errorLevel = ERROR;
        *errorMsg   = "Initial memory allocation is unsuccessful";
        break;
    case STACK_BAD_REALLOC:
        *errorLevel = WARN;
        *errorMsg   = "Reallocation is unsuccessful";
        break;
    case STACK_WRONG_REALLOC:
        *errorLevel = WARN;
        *errorMsg   = "Inappropriate use of realloc. Size is more than new capacity";
        break;
    case STACK_BAD_STATUS:
        *errorLevel = WARN;
        *errorMsg   = "Operating with stack with bad status";
        break;
    case STACK_REINIT:
        *errorLevel = WARN;
        *errorMsg   = "Reinitializing of stack";
        break;
    case STACK_WRONG_SHRINK:
        *errorLevel = WARN;
        *errorMsg   = "Wrong use of stack_shrink";
        break;
    case STACK_WRONG_EXPAND:
        *errorLevel = WARN;
        *errorMsg   = "Wrong use of stack_expand";
        break;
    case STACK_EMPTY_POP:
        *errorLevel = WARN;
        *errorMsg   = "Called pop to empty stack";
        break;
    case STACK_EMPTY_GET:
        *errorLevel = ERROR;
        *errorMsg   = "Getting element from empty stack";
        break;
    case STACK_INFO_CORRUPTED:
        *errorLevel = FATAL;
        *errorMsg   = "Internal stack information is corrupted";
        break;
    case STACK_REFREE:
        *errorLevel = WARN;
        *errorMsg   = "Refreeing of stack";
        break;
    default:
        *errorLevel = ERROR;
        *errorMsg   = "Unknown error";
    }
}

void stack_log(const ERROR_LEVEL level, const char *msg){
    if(level >= LOG_LEVEL){
        assert(LOG_FILE != NULL);

        time_t curTime = 0;
        curTime = time(NULL);
        char timeStamp[10] = "";
        strftime(timeStamp, 10, "%H:%M:%S", localtime(&curTime));

        fprintf(LOG_FILE, "%s StackLib [%s]:\t%s\n",timeStamp, getLevelMsg(level), msg);
    }
}

void stack_dump_data(const Stack *stack){
    assert(stack->data != NULL);
    assert(LOG_FILE != NULL);
    if(stack->infoHash != stack_info_hash(stack)){
        fprintf(LOG_FILE, "Inner stack information is corrupted. Unable to dump data\n");
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

    fprintf(LOG_FILE, "\nDumping data:\n");
    for(size_t i = 0; i < stack->capacity; ++i){
        fprintf(LOG_FILE, "Ceil №%zu\tat address: %p\thas value:", i, stack->data + i);
        fprintf(LOG_FILE, format, stack->data[i]);
        fprintf(LOG_FILE, "\n");
    }
    fprintf(LOG_FILE, "Dumping data end\n\n");
}