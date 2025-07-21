#include "context.h"
#include "parser.h"
#include "renderer/to_html.h"
#include "token.h"
#include "utils/debug.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
  if (argc < 2 || argc > 4) {
    fprintf(stderr, "Invalid argument count: provided %d, expected 1 to 3\n",
            argc - 1);
    return EXIT_FAILURE;
  }

  char *path = argv[1];
  FILE *file = fopen(path, "r");

  if (file == NULL) {
    fprintf(stderr, "Failed to open file: %s\n", path);

    return EXIT_FAILURE;
  }

  ParserContext ctx;
  if (init_parser_context(&ctx) < 0) {
    fclose(file);
    return EXIT_FAILURE;
  }

  if (parse_file(file, &ctx) < 0) {
    goto fail;
  }

  HtmlParserUserOptions user_opts = {
      .pretty = 0,
      .omit_body = 0,
  };

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--pretty") == 0) {
      user_opts.pretty = 1;
    } else if (strcmp(argv[i], "--omit-body") == 0) {
      user_opts.omit_body = 1;
    }

    if (strcmp(argv[i], "--pretty") == 0) {
      user_opts.pretty = 1;
    }
  }

  render_as_html(ctx.ast, user_opts);

  free_parser_context(&ctx);
  fclose(file);
  return EXIT_SUCCESS;

fail:
  free_parser_context(&ctx);
  fclose(file);
  return EXIT_FAILURE;
}
