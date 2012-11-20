#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sort.h>

typedef struct {
    int *arr;
    size_t count;
    size_t layer;
    size_t section;
    size_t completed;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} mergeinfo_t;

#define MERGEINFO_INITIALIZER(arr, count) \
    ((mergeinfo_t){arr, count, 3, 0, 0, PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER})

void *mergesort_t(void *);
int arr_issorted(int *, size_t);
void arr_print(int *, size_t);

void mergesort_ii(int *arr, size_t count)
{   // Iterative and In-place merge sort
    size_t len = 8; // length of each merge
    int *arrend = &arr[count]; // exclusive
    do {
        int *e = arr;
        while (e < arrend)
        {
            int *l = e;
            int *r = e+len;
            e += len << 1;
            if (e > arrend)
                e = arrend;
            if (len <= 8)
                insertionsort(l, e-l);
            else
                merge_i(l, r, e);
        }
        len <<= 1;
    } while (len < count);
}

void mergesort_ib(int *arr, size_t count)
{   // Iterative and In-place merge sort
    size_t len = 8; // length of each merge
    int *arrend = &arr[count]; // exclusive
    int *b = malloc(count*sizeof(int));
    do {
        int *e = arr;
        while (e < arrend)
        {
            int *l = e;
            int *r = e+len;
            e += len << 1;
            if (e > arrend)
                e = arrend;
            if (len <= 8)
                insertionsort(l, e-l);
            else
                merge_b(l, r, e, b);
        }
        len <<= 1;
    } while (len < count);
    free(b);
}

void mergesort_rr(int *arr, size_t count, int *buff)
{
    if (count <= 8)
    {
        insertionsort(arr, count);
        return;
    }
    // sort the subarrays
    int *l = arr, *r = &arr[count/2]; // left, right
    int *le = r, *re = &arr[count]; // left end, right end
    mergesort_rr(l, le-l, buff);
    mergesort_rr(r, re-r, buff+count/2);
    // merge them
    merge_b(l, r, re, buff);
}

void mergesort_r(int *arr, size_t count)
{
    int *buff = malloc(count*sizeof(int));
    mergesort_rr(arr, count, buff);
    free(buff);
}

void mergesort_f(int *arr, size_t count)
{   // single-threaded merge sort using the thread function
    mergeinfo_t info = MERGEINFO_INITIALIZER(arr, count);
    mergesort_t(&info);
}

void mergesort_m(int *arr, size_t count)
{
    mergeinfo_t info = MERGEINFO_INITIALIZER(arr, count);
    int threads = 3; // turn into parameter + sanity check
    pthread_t thread[threads];
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    for (int t = 0; t < threads; t++)
        pthread_create(&thread[t], &attr, mergesort_t, &info);

    pthread_attr_destroy(&attr);

    for (int t = 0; t < threads; t++)
        pthread_join(thread[t], NULL);

    pthread_mutex_destroy(&info.mutex);
    pthread_cond_destroy(&info.cond);
}

int get_section(mergeinfo_t *info, int **lp, int **rp, int **ep, int c)
{   // allocates one section of the array for use by a thread
    size_t layer, length, sections, section, completed, count = info->count;
    int *arr = info->arr, *end = &arr[info->count];

    pthread_mutex_lock(&info->mutex);
    info->completed += !!c;

    for (;;)
    {
        layer = info->layer;
        section = info->section;
        length = 1 << layer;
        sections = count / length + !!(count % length); // divide, rounded up
        completed = info->completed;
        if (section == sections)
        {
            if (length > count)
            {
                pthread_mutex_unlock(&info->mutex);
                return 0;
            }
            if (completed == sections)
            {   // THIS merge just completed a layer
                info->layer++;
                info->section = 0;
                info->completed = 0;
                // let the other threads continue on the new layer
                pthread_cond_broadcast(&info->cond);
            }
            else // wait for the layer to finish
                pthread_cond_wait(&info->cond, &info->mutex);
            continue; // change to goto?
        }
        info->section++;
        break;
    }

    pthread_mutex_unlock(&info->mutex);

    *lp = &arr[section*length];
    *rp = &arr[section*length + length/2];
    *ep = &arr[(section+1)*length];
    if (*ep > end)
        *ep = end;

#ifdef DEBUG
    printf("    %d:%d:%d (%d,%d,%d)\n", *lp-arr, *rp-arr, *ep-arr, length, section, sections);
#endif

    return 1;
}

void *mergesort_t(void *vinfo)
{   // merge sort for one thread
    mergeinfo_t *info = vinfo;
    int *l, *r, *e = NULL, *buff = NULL;
    size_t buff_sz = 0;
    while (get_section(info, &l, &r, &e, e != NULL))
    {
        if (e-l <= 8)
            insertionsort(l, e-l);
        else
        {
            if ((size_t)(e-l) > buff_sz)
            {
                buff_sz = e-l;
                if (buff)
                    free(buff);
                buff = malloc(buff_sz*sizeof(int));
            }
            merge_b(l, r, e, buff);
        }
    }
    if (buff)
        free(buff);
    return NULL; // don't use pthread_exit(), this might be called from main thread
}

void merge_i(int *l, int *r, int *e) // left, right, end
{   // Merge of one section of the array in place
    while (l < r && r < e)
    {
        if (*r < *l)
        {
            int t = *r;
            memmove(l+1, l, (r-l)*sizeof(int));
            *l = t;
            r++;
        }
        l++;
    }
}

void merge_b(int *l, int *r, int *e, int *buff)
{   // Merge a section of the array using a buffer as temp storage
    int *b = buff, *arr = l, *le = r, *re = e;
    // l: left, r: right, le: left end, re: right end
    while (l < le && r < re)
        *b++ = *r < *l ? *r++ : *l++;
    // copy the remaining elements
    while (l < le)
        *b++ = *l++;
    while (r < re)
        *b++ = *r++;
    // copy the result back to the array
    memcpy(arr, buff, (e-arr)*sizeof(int));
}

void insertionsort(int *arr, size_t count)
{
    for (size_t j = 0, i = 1; i < count; j = i++)
    {
        int v = arr[i];
        while (arr[j] > v)
        {
            arr[j+1] = arr[j];
            if (!j--)
                break;
        }
        arr[j+1] = v;
    }
}

