#include "debug.h"
#include "round.h"
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

/* Dumps the SIZE bytes in BUF to the console as hex bytes
   arranged 16 per line.  Numeric offsets are also included,
   starting at OFS for the first byte in BUF.  If ASCII is true
   then the corresponding ASCII characters are also rendered
   alongside. */
void hex_dump(uintptr_t ofs, const void *buf_, size_t size, bool ascii) {
  const uint8_t *buf = buf_;
  const size_t per_line = 16; /* Maximum bytes per line. */

  while (size > 0) {
    size_t start, end, n;
    size_t i;

    /* Number of bytes on this line. */
    start = ofs % per_line;
    end = per_line;
    if (end - start > size)
      end = start + size;
    n = end - start;

    /* Print line. */
    printf("%08jx  ", (uintmax_t)ROUND_DOWN(ofs, per_line));
    for (i = 0; i < start; i++)
      printf("   ");
    for (; i < end; i++)
      printf("%02hhx%c", buf[i - start], i == per_line / 2 - 1 ? '-' : ' ');
    if (ascii) {
      for (; i < per_line; i++)
        printf("   ");
      printf("|");
      for (i = 0; i < start; i++)
        printf(" ");
      for (; i < end; i++)
        printf("%c", isprint(buf[i - start]) ? buf[i - start] : '.');
      for (; i < per_line; i++)
        printf(" ");
      printf("|");
    }
    printf("\n");

    ofs += n;
    buf += n;
    size -= n;
  }
}

/* Halts the OS, printing the source file name, line number, and
   function name, plus a user-specific message. */
void debug_panic(const char *file, int line, const char *function,
                 const char *message, ...) {
  printf("Kernel PANIC at %s:%d in %s(): %s ", file, line, function, message);
}

/* Prints SIZE, which represents a number of bytes, in a
   human-readable format, e.g. "256 kB". */
void print_human_readable_size(uint64_t size) {
  if (size == 1)
    printf("1 byte");
  else {
    static const char *factors[] = {"bytes", "kB", "MB", "GB", "TB", NULL};
    const char **fp;

    for (fp = factors; size >= 1024 && fp[1] != NULL; fp++)
      size /= 1024;
    printf("%lu %s", size, *fp); //%llu
  }
}
