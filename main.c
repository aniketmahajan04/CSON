#include <ctype.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
  JSON_NULL,
  JSON_BOOL,
  JSON_NUMBER,
  JSON_STRING,
  JSON_ARRAY,
  JSON_OBJECT
} JsonType;

typedef struct JsonPair {
  char *key;
  struct JsonValue *value;
} JsonPair;

typedef struct JsonValue {
  JsonType type;
  union {
    int boolean;
    long number;
    char *string;
    struct {
      struct JsonValue **items;
      int count;
    } array;
    struct {
      struct JsonPair *pairs;
      int count;
    } object;
  } data;
} JsonValue;

long parse_integer(const char **ptr) {
  long result = 0;
  while (isdigit(**ptr)) {
    result = result * 10 + (**ptr - '0');
    (*ptr)++;
  }
  return result;
}

char *parse_string(const char **ptr) {

  while (**ptr != '\"' && **ptr != '\0')
    (*ptr)++;

  (*ptr)++;

  const char *start = *ptr;

  while (**ptr != '\"' && **ptr != '\0')
    (*ptr)++;

  int length = *ptr - start;

  char *my_name = malloc(length + 1);

  for (int i = 0; i < length; i++) {
    my_name[i] = start[i];
  }

  my_name[length] = '\0';

  (*ptr)++;

  return my_name;
}

int is_space(char c) {
  if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
    return 1;
  }

  return 0;
}

void skip_whitespaces(const char **ptr) {
  // while (**ptr != '\0' && isspace(**ptr)) { // <- this isspace is comming
  // from ctype.h
  //   (*ptr)++;
  // }
  while (**ptr != '\0' && is_space(**ptr)) {
    (*ptr)++;
  }
}

JsonValue *create_json_value(JsonType type) {
  JsonValue *v = malloc(sizeof(JsonValue));
  v->type = type;

  memset(&v->data, 0, sizeof(v->data));
  return v;
}

JsonValue *parse_value(const char **ptr) {
  skip_whitespaces(ptr);

  /*
  if (**ptr == '\"') {
    char *s = parse_string(ptr);
    printf("String: %s\n", s);
    free(s);
  } else if (isdigit(**ptr)) {
    long n = parse_integer(ptr);
    printf("Number: %ld\n", n);
  } else if (strncmp(*ptr, "true", 4) == 0) {
    printf("Boolean: true\n");
    *ptr += 4;
  } else if (strncmp(*ptr, "false", 5) == 0) {
    printf("Boolean: false\n");
    *ptr += 5;
  } else if (strncmp(*ptr, "null", 4) == 0) {
    printf("Literal: null\n");
    *ptr += 4;
  } else if (**ptr == '[') {
    (*ptr)++;

    while (**ptr != ']' && **ptr != '\0') {
      skip_whitespaces(ptr);

      if (**ptr == ',') {
        (*ptr)++;
        skip_whitespaces(ptr);
      }

      if (**ptr == ']')
        break;

      parse_value(ptr);
      skip_whitespaces(ptr);
    }
    if (**ptr == ']')
      (*ptr)++;
  } else if (**ptr == '{') {
    (*ptr)++;

    while (**ptr != '}' && **ptr != '\0') {
      skip_whitespaces(ptr);

      if (**ptr == ',' || **ptr == ':') {
        (*ptr)++;
        skip_whitespaces(ptr);
      }

      if (**ptr == '}')
        break;

      parse_value(ptr);
    }

    if (**ptr == '}')
      (*ptr)++;
  }
*/

  JsonValue *v = NULL;

  if (**ptr == '\"') {

    v = create_json_value(JSON_STRING);
    v->data.string = parse_string(ptr);

  } else if (isdigit(**ptr)) {

    v = create_json_value(JSON_NUMBER);
    v->data.number = parse_integer(ptr);

  } else if (strncmp(*ptr, "true", 4) == 0) {

    v = create_json_value(JSON_BOOL);
    v->data.boolean = 1;
    *ptr += 4;

  } else if (strncmp(*ptr, "false", 5) == 0) {

    v = create_json_value(JSON_BOOL);
    v->data.boolean = 0;
    *ptr += 5;

  } else if (strncmp(*ptr, "null", 4) == 0) {

    v = create_json_value(JSON_NULL);
    // if the value is null dont set any data
    *ptr += 4;

  } else if (**ptr == '[') {

    v = create_json_value(JSON_ARRAY);
    (*ptr)++;

    while (**ptr != ']' && **ptr != '\0') {
      skip_whitespaces(ptr);

      if (**ptr == ',') {
        (*ptr)++;
        skip_whitespaces(ptr);
      }

      if (**ptr == ']') {
        break;
      }

      JsonValue *child = parse_value(ptr);

      v->data.array.items = realloc(
          v->data.array.items, sizeof(JsonValue *) * (v->data.array.count + 1));

      v->data.array.items[v->data.array.count] = child;
      v->data.array.count++;
    }
    if (**ptr == ']')
      (*ptr)++;

  } else if (**ptr == '{') {
    v = create_json_value(JSON_OBJECT);

    (*ptr)++;

    while (**ptr != '}' && **ptr != '\0') {
      skip_whitespaces(ptr);

      if (**ptr == ',') {
        (*ptr)++;
        skip_whitespaces(ptr);
      }

      if (**ptr == '}')
        break;

      char *key = parse_string(ptr);

      skip_whitespaces(ptr);
      if (**ptr == ':')
        (*ptr)++;

      JsonValue *val = parse_value(ptr);

      v->data.object.pairs = realloc(
          v->data.object.pairs, sizeof(JsonPair) * (v->data.object.count + 1));

      v->data.object.pairs[v->data.object.count].key = key;
      v->data.object.pairs[v->data.object.count].value = val;
      v->data.object.count++;

      skip_whitespaces(ptr);
    }

    if (**ptr == '}')
      (*ptr)++;
  }

  return v;
}

void print_json(JsonValue *v, int indent) {

  if (!v)
    return;

  for (int i = 0; i < indent; i++)
    printf(" ");

  switch (v->type) {
  case JSON_STRING:
    printf("STRING: %s\n", v->data.string);
    break;

  case JSON_NUMBER:
    printf("NUMBER: %ld", v->data.number);
    break;

  case JSON_BOOL:
    printf("BOOL: %s\n", v->data.boolean ? "true" : "false");
    break;

  case JSON_NULL:
    printf("NULL\n");
    break;

  case JSON_ARRAY:
    printf("ARRAY [\n");

    for (int i = 0; i < v->data.array.count; i++) {
      print_json(v->data.array.items[i], indent + 1);
    }
    for (int i = 0; i < indent; i++)
      printf(" ");

    printf("]\n");
    break;

  case JSON_OBJECT:
    printf("OBJECT {\n");

    for (int i = 0; i < v->data.object.count; i++) {
      for (int j = 0; j < indent + 1; j++)
        printf(" ");

      printf("KEY: \"%s\" -> ", v->data.object.pairs[i].key);

      print_json(v->data.object.pairs[i].value, 0);
    }
    for (int i = 0; i < indent; i++)
      printf("}\n");
    break;
  }
}

int main() {

  const char *json = "{ \"user\" : { \"id\" : 101, \"tags\" : [\"admin\", "
                     "\"dev\"] }, \"married\": false, \"data\" : null }";
  const char *cursor = json;

  skip_whitespaces(&cursor);

  // while (*cursor != '\0' && *cursor != '}') {
  //   skip_whitespaces(&cursor);
  //
  //   if (*cursor == '{' || *cursor == ':' || *cursor == ',') {
  //     cursor++;
  //     skip_whitespaces(&cursor);
  //   }
  //
  //   if (*cursor == '}' || *cursor == '\0')
  //     break;
  //
  //   parse_value(&cursor);
  // }

  JsonValue *root = parse_value(&cursor);

  if (root) {
    printf("---Parsed JSON Tree---");
    print_json(root, 0);
  }

  return 0;
}
