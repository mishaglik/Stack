#include "stdio.h"
#include "Loglib/Logger.h"
#include "Stack.h"

const char* LOG_FILENAME = "stack.log";

int main(){
//    logger_set_file(LOG_FILENAME);
//    logger_set_log_level(INFO);
//    logger_set_abort_level(WARNING);
//    logger_clear_log_file();

    Stack stack = {};
    stack_init(&stack);
    stack_reserve(&stack, 20);

    for(int i = 0; i < 100; ++i)
        stack_push(&stack, i);
    for(int i = 0; i < 88; ++i)
        stack_pop(&stack);

    stack_element_t top;
    stack_get(&stack, &top);
    printf("%i\n", top);

    STACK_DUMP(&stack);

    stack_free(&stack);
    logger_finish();
    return 0;
}
