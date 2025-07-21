#include "renderer/to_html.h"
#include "token.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *get_tag_from_type(TokenType type, void *meta) {
  switch (type) {
  case DOCUMENT:
    return "body";
  case PARAGRAPH:
    return "p";
  case BOLD:
    return "strong";
  case ITALIC:
    return "em";
  case LINK:
    return "a";
  case LIST:
    return "ul";
  case ORDERED_LIST:
    return "ol";
  case LIST_ITEM:
    return "li";
  case IMAGE:
    return "img";
  case BLOCK_QUOTE:
    return "blockquote";
  case THEMATIC_BREAK:
    return "hr";
  case CODE_BLOCK:
  case CODE_SPAN:
    return "code";
  case HEADING: {
    HeadingData heading_meta = *(HeadingData *)meta;
    switch (heading_meta.level) {
    case 1:
      return "h1";
    case 2:
      return "h2";
    case 3:
      return "h3";
    case 4:
      return "h4";
    case 5:
      return "h5";
    case 6:
    default:
      return "h6";
    }
  }
  default:
    return NULL;
  }

  return NULL;
}

int is_self_closing(TokenType type) {
  return type == IMAGE || type == THEMATIC_BREAK;
}

int is_code(Token *token) {
  return token->type == CODE_BLOCK || token->type == CODE_SPAN;
}

void free_attributes(Attribute *attributes, size_t num_attributes) {
  for (int i = 0; i < num_attributes; i++) {
    free(attributes[i].value);
  }
}

int get_attributes(Token *token, Attribute *attributes) {
  size_t count = 0;

  switch (token->type) {
  case LINK:
    strcpy(attributes[count].key, "href");
    attributes[count].value = strdup(token->meta->link.href);
    if (!attributes[count].value) {
      return -1;
    }
    count++;
    break;
  case IMAGE:
    strcpy(attributes[count].key, "src");
    attributes[count].value = strdup(token->meta->image.src);
    if (!attributes[count].value) {
      return -1;
    }
    count++;
    strcpy(attributes[count].key, "alt");
    attributes[count].value = strdup(token->meta->image.alt);
    if (!attributes[count].value) {
      free(attributes[count - 1].value);
      return -1;
    }
    count++;
    break;
  default:
    break;
  }

  return count;
}

void escape_html_entities(FILE *out, Token *token) {
  char *p = token->content;
  while (*p) {
    switch (*p) {
    case '<':
      fputs("&lt;", out);
      break;
    case '>':
      fputs("&gt;", out);
      break;
    case '"':
      fputs("&quot;", out);
      break;
    case '\'':
      fputs("&#39;", out);
      break;
    case '&':
      fputs("&amp;", out);
      break;
    default:
      fputc(*p, out);
      break;
    }
    p++;
  }
}

void print_indent(int level) {
  for (size_t i = 0; i < level; i++) {
    printf("  ");
  }
}

int is_block_element(TokenType type) {
  return type == PARAGRAPH || type == ORDERED_LIST || type == LIST ||
         type == BLOCK_QUOTE || type == HEADING || type == THEMATIC_BREAK ||
         type == DOCUMENT;
}

void render_as_html(Token *root, HtmlParserUserOptions user_opts) {
  HtmlParserOptions opts = {
      .user = user_opts,
      .is_last = 0,
      .is_first = 1,
      .level = 0,
      .has_nested_list = 0,
  };
  return to_html(root, &opts);
}

void render_children(Token *root, HtmlParserOptions *opts) {
  for (size_t i = 0; i < root->child_count; i++) {
    Token *child = root->children[i];
    HtmlParserOptions child_opts = {
        .user = opts->user,
        .level = opts->level +
                 (opts->user.omit_body && root->type == DOCUMENT ? 0 : 1),
        .is_last = i == root->child_count - 1,
        .is_first = i == 0,
        .has_nested_list = 0,
    };
    if (opts->user.pretty && !child_opts.is_first &&
        (child->type == LIST || child->type == ORDERED_LIST) &&
        root->type == LIST_ITEM) {
      opts->has_nested_list = 1;
      printf("\n");
    }
    to_html(root->children[i], &child_opts);
  }
}

void to_html(Token *root, HtmlParserOptions *opts) {
  if (!root) {
    return;
  }

  if (opts->user.omit_body && root->type == DOCUMENT) {
    render_children(root, opts);
    return;
  }

  Attribute attributes[20];
  int num_attributes = get_attributes(root, attributes);
  char *tag = get_tag_from_type(root->type, root->meta);
  int is_block = is_block_element(root->type);

  if (tag) {

    if (root->type == CODE_BLOCK) {
      printf("<pre>");
    }

    if (opts->user.pretty && (is_block || root->type == LIST_ITEM)) {
      print_indent(opts->level);
    }
    printf("<%s", tag);
    for (int i = 0; i < num_attributes; i++) {
      printf(" %s=\"%s\"", attributes[i].key, attributes[i].value);
    }
    if (is_self_closing(root->type)) {
      printf("/>");
      if (opts->user.pretty && is_block) {
        printf("\n");
      }
      free_attributes(attributes, num_attributes);
      return;
    } else {
      printf(">");
      if (opts->user.pretty && is_block) {
        printf("\n");
      }
    }
  }

  if (root->type == LINE) {
    if (opts->user.pretty && opts->is_first) {
      print_indent(opts->level);
    } else if (opts->is_last) {
      printf(" ");
    }
  }

  if (root->content) {
    if (opts->user.pretty && is_block) {
      print_indent(opts->level + 1);
    }

    if (is_code(root)) {
      escape_html_entities(stdout, root);
    } else {
      printf("%s", root->content);
    }

    if (opts->user.pretty && is_block) {
      printf("\n");
    }
  }

  render_children(root, opts);

  if (root->type == LINE && opts->user.pretty && opts->is_last) {
    printf("\n");
  }

  if (tag) {
    if (opts->user.pretty && (is_block || opts->has_nested_list)) {
      print_indent(opts->level);
    }

    printf("</%s>", tag);

    if (opts->user.pretty && (is_block || root->type == LIST_ITEM)) {
      printf("\n");
    }

    if (root->type == CODE_BLOCK) {
      printf("</pre>");

      if (opts->user.pretty) {
        printf("\n");
      }
    }
  }

  free_attributes(attributes, num_attributes);
}
