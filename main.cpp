#include "stdio.h"
#include "Stack.h"
int main(){
    Stack stack = {};
    stack_init(&stack);
    stack_push(&stack, 1);
    stack_push(&stack, 2);
    stack_push(&stack, 3);
    stack_pop(&stack);
    stack_pop(&stack);
//    stack_pop(&stack);
//    stack_pop(&stack);
    stack_element_t top;
    top = stack_top(&stack);
    printf("%i\n", top);
    stack_free(&stack);
}
