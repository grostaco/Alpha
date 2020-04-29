#ifndef __PARSER_H
#define __PARSER_H

#include "ds/queue.h"
#include "ds/stack.h"
#include "basic_hash.h"

/* Exclusive to struct operator's flags field, BINARY_OPERATOR and FUNCTION being an exception */
#define OPEN_PAREN      1
#define CLOSED_PAREN    0
#define BINARY_OPERATOR 2
#define FUNCTION        3

#define OPERATOR_IS(op,id)          (((op)->flags&3)==id) 
#define OPERATOR_TYPE(op,id)        ((op)->flags&3)
#define OPERATOR_FUNCTION_INDEX(op) ((op)->flags >> 2)

/* Exclusive to struct parser_output's flags field */
#define IMM             0

/* IMM's type */
#define NONE            0
#define INT             1
#define FLOAT           2

#define OUTPUT_IS(f,id)        (((f)&3)==id)
#define OUTPUT_TYPE_IS(f,type) ((((f)>>2)&3)==type) 

typedef uint16_t binary_operator;

#define ASSOC_LEFT_TO_RIGHT 0
#define ASSOC_RIGHT_TO_LEFT 1

#define CREATE_OPERATOR(operator, pred, assoc) ((uint8_t)operator | (pred << 8) | (assoc << 12))
#define OPERATOR_OF(operator)                  ((operator & 0xff))
#define PRECEDENCE(operator)                   (operator & (0xf << 8))
#define ASSOCIATIVITY(operator)                (operator & (0x1 << 12))

struct operator {
    uint32_t flags;
    binary_operator binary_operator; /* Unused if function flag is set */
};

struct parser_output {
    uint64_t flags;
    union {
        int i;
        float f;
        char c;
        const char* str;
    }payload;
};

queue_t parse(char*);

#endif