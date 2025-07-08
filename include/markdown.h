#ifndef MARKDOWN_H
#define MARKDOWN_H

#include "stack.h"
#include "token.h"
#include <stddef.h>
#include <stdio.h>

int process_file(FILE *file, Stack *block_stack, Stack *inline_stack,
                 Token *ast);
int is_front_matter(char *line, int *in_front_matter);
int parse_heading(char *line, Token **out);
int handle_heading(char *line, Token *ast);
int handle_paragraph(char *line, Stack *stack);
int handle_text(char *line, Stack *inline_stack);
int handle_blank_line(Stack *block_stack, Stack *inline_stack, Token *ast);
void print_ast(Token *root, int level);
void free_ast(Token *root);
char *get_token_type_str(TokenType token_type);

#endif
