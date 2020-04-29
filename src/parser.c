#include "parser.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef DEBUG
#include <stdio.h>
#define PARSER_LOG(out,fmt,...) fprintf(out, "%s:%d "fmt, __FILE__, __LINE__, __VA_ARGS__)
#else
#define PARSER_LOG(...)
#endif

typedef union {
    int i; 
    float f; 
    char* c; 
    struct parser_output parser_output; 
    struct operator* operator;
}wildcard;

#define ARRAY_SIZE(x)       (sizeof(x) / sizeof(*x))
#define consume_forall(x,c) while(*x==c)++x;

const char* functions[] = {"sin", "cos", "tan"}; // dummy trig function
size_t      functions_hash[ARRAY_SIZE(functions)]; 

/* If OPERATOR_OF(operator) yield 0, it's invalid */
binary_operator operators[] = {CREATE_OPERATOR('/', 4, ASSOC_LEFT_TO_RIGHT), CREATE_OPERATOR('*', 4, ASSOC_LEFT_TO_RIGHT),\
                               CREATE_OPERATOR('+', 3, ASSOC_LEFT_TO_RIGHT), CREATE_OPERATOR('-', 3, ASSOC_LEFT_TO_RIGHT),\
                               0};

/*
    
*/
static inline int to_function(const char* nptr, char** endptr) {
    static int hashed = 0;
    size_t     resulted_hash;
    char*      loc;
    uint64_t   i;
    if (!hashed && (hashed=1)) 
        for (uint64_t i_ = 0 ; i_ < ARRAY_SIZE(functions) ; ++i_) functions_hash[i_] = hash(functions[i_]);
    if ((loc=index(nptr, '('))) { // There might be a function 
        resulted_hash = hash_n(nptr,(int)((char*)loc-(char*)nptr));
        for (i = 0 ; i < ARRAY_SIZE(functions) ; ++i) {
            if (resulted_hash == functions_hash[i])
                break;
        }
        if (i < ARRAY_SIZE(functions)) { 
            *endptr = loc;
            return i;
        }
    }
    *endptr = (char*)nptr;
    return -1;
}

static inline binary_operator apropos_operator(char operator) {
    binary_operator* init = operators;
    while (OPERATOR_OF(*init)!=operator && *init)++init;
    return *init;
}

queue_t parse(char* expr) {
    char*    endptr = expr;
    queue_t  output_queue;
    stack_t  operator_stack;
    wildcard auxiliary, _auxiliary;
    
    stack_create(&operator_stack, malloc(1024), 1024);
    queue_create(&output_queue,   malloc(1024), 1024);

    while (*(expr = endptr)) {
        if (*expr == ' ') {
            consume_forall(expr, ' ');
            endptr = expr;
        }
        auxiliary.i = strtol(expr, &endptr, 10);
        if (expr != endptr) {
            PARSER_LOG(stderr, "Pushing %d onto output queue\n", auxiliary.i);
            queue_push_back(&output_queue, ((struct parser_output){.flags=IMM|(INT << 2), .payload.i=auxiliary.i}), struct parser_output);
            continue;
        }
        auxiliary.f = strtof(expr, &endptr);
        if (expr != endptr) {
            PARSER_LOG(stderr, "Pushing %f onto output queue\n", auxiliary.f);
            queue_push_back(&output_queue, ((struct parser_output){.flags=IMM|(FLOAT << 2), .payload.f=auxiliary.f}), struct parser_output);
            continue;
        }
        auxiliary.i = to_function(expr, &endptr);
        if (expr != endptr) {
            PARSER_LOG(stderr, "Pushing %s onto output queue\n", functions[auxiliary.i]);
            stack_push(&operator_stack, ((struct operator){.flags=FUNCTION | (auxiliary.i << 2)}), struct operator);
            continue;
        }
        auxiliary.i = apropos_operator(*expr);
        if (OPERATOR_OF(auxiliary.i)) {
            PARSER_LOG(stderr, "Processing operator %c\n", OPERATOR_OF(auxiliary.i));
            auxiliary.operator=&(struct operator){.flags=BINARY_OPERATOR,.binary_operator=OPERATOR_OF(auxiliary.i)};
            _auxiliary.operator = stack_back(&operator_stack, struct operator);
            
            while ((_auxiliary.operator && !OPERATOR_IS(_auxiliary.operator, OPEN_PAREN))                           &&    ( \
                   OPERATOR_IS(_auxiliary.operator, FUNCTION)                                                            || \
                   ((OPERATOR_IS(_auxiliary.operator,BINARY_OPERATOR)                                                      && \
                    ((PRECEDENCE(auxiliary.operator->binary_operator) < PRECEDENCE(_auxiliary.operator->binary_operator)) ||
                    ((PRECEDENCE(auxiliary.operator->binary_operator) == PRECEDENCE(_auxiliary.operator->binary_operator)) && 
                     ASSOCIATIVITY(auxiliary.operator->flags) == ASSOC_LEFT_TO_RIGHT)))
                   )
                  )) {
                        struct operator* operator = stack_pop(&operator_stack, struct operator);
                        if (!operator)break;
                        if (OPERATOR_IS(operator, BINARY_OPERATOR)) {
                            PARSER_LOG(stderr, "Pushing %c onto output queue\n", OPERATOR_OF(operator->binary_operator));
                            queue_push_back(&output_queue, ((struct parser_output){.flags=BINARY_OPERATOR, .payload.c=OPERATOR_OF(operator->binary_operator)}), struct parser_output);
                            continue;   
                        }
                    PARSER_LOG(stderr, "Very unexpected fallthrough%c", 10);
                   }
            PARSER_LOG(stderr, "Pushing %c onto operator stack\n", OPERATOR_OF(auxiliary.operator->binary_operator));
            stack_push(&operator_stack, *auxiliary.operator, struct operator);
            ++endptr;
        }else if (*expr == '(') {
            PARSER_LOG(stderr, "Pushing %c onto operator stack\n", '(');
            stack_push(&operator_stack, (struct operator){.flags=OPEN_PAREN}, struct operator);
            consume_forall(endptr, ' ');
            ++endptr;
        }else if (*expr == ')') {
            
            while (!OPERATOR_IS(stack_back(&operator_stack, struct operator), OPEN_PAREN)) {
                _auxiliary.operator = stack_pop(&operator_stack, struct operator);
                if (!_auxiliary.operator) {
                    PARSER_LOG(stderr, "Fatal: parentheses mismatch%c", 10);
                }
                if (OPERATOR_IS(_auxiliary.operator, FUNCTION)){
                    PARSER_LOG(stderr, "Pushing %s onto output queue\n", functions[auxiliary.i]);
                    queue_push_back(&output_queue,                                                                                   \
                    ((struct parser_output){.flags=FUNCTION, .payload.str=functions[OPERATOR_FUNCTION_INDEX(_auxiliary.operator)]}), \
                    struct parser_output);
                }else if (OPERATOR_IS(_auxiliary.operator, BINARY_OPERATOR)) {
                    PARSER_LOG(stderr, "Pushing %c onto output queue\n", OPERATOR_OF(_auxiliary.operator->binary_operator));
                    queue_push_back(&output_queue,                                                                                  \
                    ((struct parser_output){.flags=BINARY_OPERATOR, .payload.c=OPERATOR_OF(_auxiliary.operator->binary_operator)}), \
                    struct parser_output);
                }
            }
            _auxiliary.operator = stack_back(&operator_stack, struct operator);
            if (OPERATOR_IS(_auxiliary.operator, OPEN_PAREN)) {
                PARSER_LOG(stderr, "Popping %c from operator stack\n", '(');
                (void)stack_pop(&operator_stack, struct operator);
            }
            ++endptr;
        }
    }
    while ((auxiliary.operator = stack_pop(&operator_stack, struct operator))) {
        if (OPERATOR_IS(auxiliary.operator, FUNCTION)){
            
            queue_push_back(&output_queue,                                                                                  \
                            ((struct parser_output){.flags=FUNCTION, .payload.str=functions[OPERATOR_FUNCTION_INDEX(auxiliary.operator)]}), \
                            struct parser_output);
        }else if (OPERATOR_IS(auxiliary.operator, BINARY_OPERATOR)) {
            PARSER_LOG(stderr, "Popped %c from operator stack\n", OPERATOR_OF(auxiliary.operator->binary_operator));
            PARSER_LOG(stderr, "Pushing %c onto output queue\n", OPERATOR_OF(auxiliary.operator->binary_operator));
            queue_push_back(&output_queue,                                                                                 \
                            ((struct parser_output){.flags=BINARY_OPERATOR, .payload.c=OPERATOR_OF(auxiliary.operator->binary_operator)}), \
                            struct parser_output);
        }
    }
    return output_queue;
}

