#ifndef MERGE_H
#define MERGE_H

#include <stdlib.h>
#include <assert.h>

void mergesort_ii(int *, size_t);
void mergesort_ib(int *, size_t);
void mergesort_r(int *, size_t);
void mergesort_s(int *, size_t);
void mergesort_m(int *, size_t);
void mergesort_rm(int *, size_t);

void merge_i(int *, int *, int *, int *);
void merge_b(int *, int *, int *, int *);

void insertionsort(int *, size_t);

#ifndef MAIN
extern
#endif
void *boundstart, *boundend;

#define BOUNDS_SET(s, e) boundstart = s, boundend = e
#define BOUNDS_CHECK(ptr) assert(boundstart <= (void*)(ptr) && (void*)(ptr) < boundend)

#endif // MERGE_H
