#include <string.h>
#include <unistd.h>

int isVariable(char *str) { return str[0] == '$' ? 1 : 0; }

void slice(const char *str, char *result, size_t start, size_t end) {
  strncpy(result, str + start, end - start);
}
