#include <shivers.h>

static void free_element(jsonelement_t *element) {
  ASSERT(element, !=, NULL);

  if (element->key) {
    free(element->key);
  }

  if (element->value) {
    free(element->value);
  }

  free(element);
}

static void free_elements(jsonelement_t *parent) {
  ASSERT(parent, !=, NULL);

  uint32_t capacity = 128;
  jsonelement_t **stack = allocate(NULL, 0, capacity, sizeof(jsonelement_t *));
  if (stack == NULL)
    return;

  uint32_t top = 0;
  stack[top++] = parent;

  while (top > 0) {
    jsonelement_t *current = stack[--top];

    if (current->type == JSON_ARRAY || current->type == JSON_OBJECT) {
      jsonelement_t **elements = (jsonelement_t **) current->value;

      for (uint32_t i = 0; i < current->size; ++i) {
        jsonelement_t *element = elements[i];
        if (top >= capacity) {
          stack = allocate(stack, capacity, capacity * 2, sizeof(jsonelement_t *));
          capacity *= 2;

          if (stack == NULL)
            return;
        }

        stack[top++] = element;
      }
    }

    free_element(current);
  }

  free(stack);
}

void json_free(jsonelement_t *element, const bool all) {
  jsonelement_t *top = element;
  ASSERT(element, !=, NULL);

  if (all) {
    while (top->parent) {
      top = top->parent;
    }
  }

  free_elements(top);
}
