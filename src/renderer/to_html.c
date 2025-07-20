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

void to_html(Token *root, Token *prev) {
  if (!root) {
    return;
  }

  Attribute attributes[20];
  int num_attributes = get_attributes(root, attributes);
  char *tag = get_tag_from_type(root->type, root->meta);

  if (tag) {
    if (root->type == CODE_BLOCK) {
      printf("<pre>");
    }

    printf("<%s", tag);
    for (int i = 0; i < num_attributes; i++) {
      printf(" %s=\"%s\"", attributes[i].key, attributes[i].value);
    }
    if (is_self_closing(root->type)) {
      printf("/>");
      free_attributes(attributes, num_attributes);
      return;
    } else {
      printf(">");
    }
  }

  if (root->type == LINE && prev && prev->type == LINE) {
    printf(" ");
  }

  if (root->content) {
    if (is_code(root)) {
      escape_html_entities(stdout, root);
    } else {
      printf("%s", root->content);
    }
  }

  for (size_t i = 0; i < root->child_count; i++) {
    to_html(root->children[i], i > 0 ? root->children[i - 1] : NULL);
  }

  if (tag) {
    printf("</%s>", tag);

    if (root->type == CODE_BLOCK) {
      printf("</pre>");
    }
  }

  free_attributes(attributes, num_attributes);
}
