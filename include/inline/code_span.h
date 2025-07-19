#ifndef CODE_SPAN_H
#define CODE_SPAN_H

#include "inline/parser.h"

int handle_code_span(InlineParserContext *ctx);
int push_code_span_into_stack(InlineParserContext *ctx, char *content);

#endif
