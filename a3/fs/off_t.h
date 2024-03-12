#ifndef FILESYS_OFF_T_H
#define FILESYS_OFF_T_H

#include <stdint.h>

/* An offset within a file.
   This is a separate header because multiple headers want this
   definition but not any others. */
typedef int32_t offset_t;

/* Format specifier for printf(), e.g.:
   printf ("offset=%"PROTd"\n", offset); */
#define PROTd PRId32

#endif /* fs/off_t.h */
