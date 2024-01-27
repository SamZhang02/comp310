#include <string.h>

int isVariable(char *str);

void slice(const char *str, char *result, size_t start, size_t end);

char *parseNull(char *str);

char *parseSetInput(char *args[], int argsize);
