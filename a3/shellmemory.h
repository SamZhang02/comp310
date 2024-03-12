#ifndef SHELLMEMORY_H
#define SHELLMEMORY_H

void mem_init();
char *mem_get_value(char *var, char caller);
void mem_set_value(char *var, char *value);
void mem_set_value_at(int pos, const char *pid, const char *value_in,
                      int *valid_bit);
int frame_alloc(const char *pid, int *index, int *valid_bit);
char *mem_get_value_at(int pos);
void mem_free_at(int pos);
void reset_var_zone();
void clear_frame(int start_pos);
void printShellMemory();

#endif