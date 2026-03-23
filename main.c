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

  if (**ptr == '\"')
    (*ptr)++;

  const char *start = *ptr;

  while (**ptr != '\0') {

    if (**ptr == '\\' && *(*ptr + 1) != '\0') {
      *ptr += 2;
    } else if (**ptr == '\"') {

      break;
    } else {
      (*ptr)++;
    }
  }

  int length = *ptr - start;

  char *my_name = malloc(length + 1);

  int src = 0;
  int dst = 0;

  while (src < length) {
    if (start[src] == '\\') {
      src++;

      if (start[src] == 'n')
        my_name[dst++] = '\n';
      else if (start[src] == 't')
        my_name[dst++] = '\t';
      else if (start[src] == '\"')
        my_name[dst++] = '\"';
      else if (start[src] == '\\')
        my_name[dst++] = '\\';
      else
        my_name[dst++] = start[src];
    } else {
      my_name[dst++] = start[src];
    }
    src++;
  }

  my_name[dst] = '\0';

  if (**ptr == '\"')
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

  if (v == NULL && **ptr != '\0') {
    fprintf(stderr, "Unexpected character '%c' at: %.10s...\n", **ptr, *ptr);
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
    printf("NUMBER: %ld\n", v->data.number);
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

      print_json(v->data.object.pairs[i].value, indent + 1);
    }
    for (int i = 0; i < indent; i++)
      printf(" ");

    printf("}\n");
    break;
  }
}

void free_json(JsonValue *v) {
  if (!v)
    return;

  switch (v->type) {
  case JSON_STRING:
    free(v->data.string);
    break;

  case JSON_ARRAY:
    for (int i = 0; i < v->data.array.count; i++) {
      free_json(v->data.array.items[i]);
    }
    free(v->data.array.items);
    break;

  case JSON_OBJECT:
    for (int i = 0; i < v->data.object.count; i++) {
      free(v->data.object.pairs[i].key);
      free_json(v->data.object.pairs[i].value);
    }
    free(v->data.object.pairs);
    break;

  default:
    break;
  }
  free(v);
}

int main() {

  const char *json =
      "{"
      "  \"project\": \"C-SON Parser\","
      "  \"version\": 1,"
      "  \"active\": true,"
      "  \"metadata\": {"
      "    \"author\": \"Aniket\","
      "    \"tags\": [\"C\", \"JSON\", \"Parsing\"],"
      "    \"stats\": {"
      "      \"lines\": 200,"
      "      \"complexity\": \"medium\""
      "    }"
      "  },"
      "  \"data_points\": ["
      "    {\"id\": 1, \"value\": \"start\"},"
      "    {\"id\": 2, \"value\": \"middle\", \"extra\": [10, 20]},"
      "    {\"id\": 3, \"value\": \"end\", \"nested_null\": null}"
      "  ],"
      "  \"settings\": {"
      "    \"flags\": [true, false, true],"
      "    \"config\": {} "
      "  }"
      "}";
  const char *cursor = json;

  skip_whitespaces(&cursor);

  JsonValue *root = parse_value(&cursor);

  if (root) {
    printf("---Parsed JSON Tree---\n");
    print_json(root, 0);

    free_json(root);
    printf("\nMemory freed successfully!\n");
  }

  return 0;
}
