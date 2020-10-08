#if !defined(IO_H)
#define IO_H
#include "types.h"

void print(const char *);
void putSizeT(size_t);
void putSizeTHex(size_t);
void putPtr(void *);

#define DUMP_PTR(ptr) \
    putPtr(ptr);      \
    print("\t");
#define ENDL print("\n");

#endif // IO_H
