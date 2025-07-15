#ifndef BLOCK_STACK_H
#define BLOCK_STACK_H

#include "context.h"
#include "token.h"

typedef int (*FlushPredicate)(Token *token, void *userdata);
typedef int (*FlushCallback)(ParserContext *ctx, Token *token, void *userdata);

int flush_stack(ParserContext *ctx, FlushPredicate stop_when,
                FlushCallback on_flush, void *userdata);
int stop_at_non_paragraph(Token *token, void *_);
int stop_at_non_list(Token *token, void *_);
int flush_paragraph(ParserContext *ctx);
int flush_list(ParserContext *ctx);
int flush_remaining_blocks(ParserContext *ctx);

#endif
