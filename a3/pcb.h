#ifndef PCB_H
#define PCB_H

typedef struct PCB PCB;
typedef struct PAGE PAGE;

struct PCB {
  int pid;
  int size;
  int *program_counter;
  PAGE **page_table;
  int page_table_size;
  FILE *source_file;
};

struct PAGE {
  int index[3];
  int valid_bit[3];
  int page_index;
  int page_pid;
};

PCB *PCB_init(int PID, const char *script);
void free_PCB(PCB *PCB_data);
int PCB_done(PCB *PCB);
int load_PAGE(PCB *PCB, int PAGE_index);
void PAGE_evict(PCB *PCB, int PAGE_index);
int PCB_compare(const void *p1, const void *p2);
int PCB_equal(const void *p1, const void *p2);
int PAGE_equal(const void *p1, const void *p2);
char *PAGE_string(const void *p1);

#endif