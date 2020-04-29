#include "parser.h"
#include <stdio.h>

int main () {
    queue_t               queue;
    struct parser_output* parser_output;

    queue = parse("2 / (3 + 4) * 64 + 5 + 2 / 321 + 9 * 2");
    while ((parser_output = queue_pop_front(&queue, struct parser_output))) {
        if (OUTPUT_IS(parser_output->flags, IMM)) {
            if (OUTPUT_TYPE_IS(parser_output->flags, INT))
                printf("%d ", parser_output->payload.i);
        }else if (OUTPUT_IS(parser_output->flags, BINARY_OPERATOR)) {
            printf("%c ", parser_output->payload.c); 
        }
    }
    putc('\n', stdout);
}