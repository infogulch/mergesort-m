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

void mergesort_i(int *arr, size_t count)
{
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

void mergesort_f(int *arr, size_t count)
{
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

    for (int t = 0; t < threads; t++)
        pthread_cancel(thread[t]);
}

int get_section(mergeinfo_t *info, int **lp, int **rp, int **ep, int c)
{
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
{
    mergeinfo_t *info = vinfo;
    int *l, *r, *e = NULL, *buff = NULL;
    size_t buff_sz = 0;
    while (get_section(info, &l, &r, &e, e != NULL))
    {
        if (e-l <= 8)
            insertionsort(l, e-l);
        else
        {
            /*
            if ((size_t)(e-l) > buff_sz)
            {
                buff_sz = e-l;
                if (buff)
                    free(buff);
                buff = malloc(buff_sz*sizeof(int));
                printf("Allocated buffer %p\n", buff);
            }
            merge_b(l, r, e, buff);
            */
            merge_i(l,r,e);
        }
    }
    free(buff);
    return NULL; // don't use pthread_exit(), this can be called from main thread
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
{   // Merge a section of the array with a buffer.
    int *b = buff, *ls = l, *rs = r, *f;
    while (l < rs && r < e)
        *b++ = *r > *l ? *l++ : *r++;
    size_t x;
    if (l != rs) // copy the remaining elements into buff
        f = l, x = rs-l;
    else
        f = r, x = e-r;
    // printf("Moving %d items from %p to %p\n", x, f, b);
    memmove(b, f, x*sizeof(int));
    // printf("Moving %d items from %p to %p\n", e-ls, buff, ls);
    memmove(ls, buff, (e-ls)*sizeof(int)); 
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

