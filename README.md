# Safe stack
Common library realising safe stack.
Dependent on errLib
##Function list

stack_init(Stack* stack): inits stack.

stack_free(Stack* stack): frees stack.

stack_push(Stack* stack, stack_element_t val):  adds element to stack

stack_get(Stack* stack): returns element of stack

stack_pop(Stack* stack): pops element from stack

All of these functions returns STACK_ERROR - error occurred during operating. If no errors return STACK_ERRNO

Warning!
Do not make const Stack. This is useless and will not work
