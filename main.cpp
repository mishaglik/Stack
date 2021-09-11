#include "stdio.h"
#include "Logger/Logger.h"
#include "Stack.h"

const char* LOG_FILENAME = "stack.log";

int main(){
    logger_set_file(LOG_FILENAME);

    Stack stack = {};
    stack_init(&stack);

    for(int i = 0; i < 100; ++i)
        stack_push(&stack, i);
    for(int i = 0; i < 88; ++i)
        stack_pop(&stack);

    stack_element_t top;
    top = stack_top(&stack);
    printf("%i\n", top);

    stack_free(&stack);
    logger_finish();
}
