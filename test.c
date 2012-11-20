#define MAIN

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include <sort.h>

#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_RESET     "\x1b[0m"

int arr_len;
int *arr_sorted;
int *arr_unsorted;

void (*fn_sort)(int *, size_t) = NULL;
void (*fn_merge)(int *, int *, int *, int *) = NULL;

void arr_shuffle(int *arr, size_t len)
{
    for (size_t i = 0; i < len; i++)
    {
        size_t r = rand() % len;
        int tmp = arr[i];
        arr[i] = arr[r];
        arr[r] = tmp;
    }
}

void arr_print(int *arr, size_t count)
{
    printf("{");
    for (size_t i = 0; i < count; i++)
        printf("%d%s", arr[i], i == count-1 ? "" : ", ");
    printf("}\n");
}

int arr_issorted(int *arr, size_t count)
{
    for (size_t i = 1; i < count; i++)
        if (arr[i-1] > arr[i])
            return 0;
    return 1;
}

int test_arr_shuffle()
{
    int arr1[] = {1,2,3,4,5};
    int arr2[] = {2,5,4,3,1};
    srand(31337);
    arr_shuffle(arr1, 5);
    return !memcmp(arr1, arr2, 5*sizeof(int));
}

int test_none()
{
    int *arr = malloc(arr_len*sizeof(int));
    memcpy(arr, arr_unsorted, arr_len*sizeof(int));
    fn_sort(arr+50, 0);
    int result = !memcmp(arr, arr_unsorted, arr_len*sizeof(int));
    free(arr);
    return result;
}

int test_one()
{
    int *arr = malloc(arr_len*sizeof(int));
    memcpy(arr, arr_unsorted, arr_len*sizeof(int));
    fn_sort(arr+50, 1);
    int result = !memcmp(arr, arr_unsorted, arr_len*sizeof(int));
    free(arr);
    return result;
}

int test_two_sorted()
{
    int arr1[] = {3, 7};
    int arr2[] = {3, 7};
    fn_sort(arr1, 2);
    return !memcmp(arr1, arr2, 2*sizeof(int));
}

int test_many_sorted()
{
    int *arr = malloc(arr_len*sizeof(int));
    memcpy(arr, arr_sorted, arr_len*sizeof(int));
    fn_sort(arr, arr_len);
    int result = !memcmp(arr, arr_sorted, arr_len*sizeof(int));
    free(arr);
    return result;
}

int test_two_unsorted()
{
    int arr1[] = {7, 3};
    int arr2[] = {3, 7};
    fn_sort(arr1, 2);
    int result = !memcmp(arr1, arr2, 2*sizeof(int));
    return result;
}

int test_many_unsorted()
{
    int *arr = malloc(arr_len*sizeof(int));
    memcpy(arr, arr_unsorted, arr_len*sizeof(int));
#ifdef DEBUG
    printf("    Before: "); arr_print(arr, arr_len);
#endif
    fn_sort(arr, arr_len);
#ifdef DEBUG
    printf("    After: "); arr_print(arr, arr_len);
#endif
    int result = !memcmp(arr, arr_sorted, arr_len*sizeof(int));
    free(arr);
    return result;
}

int test_merge()
{
    int section = 1, length = 8;
    int arr1[] = {0,9,0,9,0,9,0,9, 1,3,5,7,2,4,6,8, 0,9,0,9,0,9,0,9};
    int arr2[] = {0,9,0,9,0,9,0,9, 1,2,3,4,5,6,7,8, 0,9,0,9,0,9,0,9};
    int *arr3 = malloc(length*sizeof(int));
#ifdef DEBUG
    printf("\n    Before: "); arr_print(arr1, 24);
#endif
    int *l = &arr1[section*length];
    int *r = &arr1[section*length + length/2];
    int *e = &arr1[(section+1)*length];
    fn_merge(l, r, e, arr3);
#ifdef DEBUG
    printf("\n    After: "); arr_print(arr1, 24);
#endif
    free(arr3);
    return !memcmp(arr1, arr2, 14*sizeof(int));
}

typedef struct {
    const char *name;
    int (*func)();
} test_t;

#define TEST(x) { #x, x }
test_t arraytests[] = {
    TEST(test_arr_shuffle)
};

test_t sorttests[] = {
    TEST(test_none),
    TEST(test_one),
    TEST(test_two_sorted),
    TEST(test_many_sorted),
    TEST(test_two_unsorted),
    TEST(test_many_unsorted),
};

test_t mergetests[] = {
    TEST(test_merge),
};

void run_tests(test_t tests[], size_t count)
{
    for (size_t i = 0; i < count; i++)
    {
        printf("  %-20s", tests[i].name);
        clock_t start = clock();
        const char *result = tests[i].func() ? "pass" : ANSI_COLOR_RED "FAIL" ANSI_RESET;
        printf(":: %s (%.5fs)\n", result, (double)(clock()-start)/CLOCKS_PER_SEC);
    }
}

int main(int argc, char *argv[])
{
    printf("Testing array functions\n");
    run_tests(arraytests, sizeof(arraytests)/sizeof(test_t));

    if (argc < 2 || !sscanf(argv[1], "%u", &arr_len) || arr_len == 0)
        arr_len = 30; // default

    arr_sorted = malloc(arr_len*sizeof(int));
    arr_unsorted = malloc(arr_len*sizeof(int));

    printf("Generating sorted and shuffled array for testing...\n");
    for (int i = 0; i < arr_len; i++)
        arr_sorted[i] = i;
    memcpy(arr_unsorted, arr_sorted, arr_len*sizeof(int));
    srand(0);
    arr_shuffle(arr_unsorted, arr_len);

    // Omit this test, it's unlikely to change and it takes too long.
    // fn_sort = insertionsort;
    // printf("Test sorting with insertionsort\n");
    // run_tests(sorttests, sizeof(sorttests)/sizeof(test_t));

    fn_merge = merge_i;
    printf("Test in-place merging\n");
    run_tests(mergetests, sizeof(mergetests)/sizeof(test_t));

    fn_merge = merge_b;
    printf("Test buffered merging\n");
    run_tests(mergetests, sizeof(mergetests)/sizeof(test_t));

    // fn_sort = mergesort_ii;
    // printf("Test sorting with iterative in-place mergesort\n");
    // run_tests(sorttests, sizeof(sorttests)/sizeof(test_t));

    fn_sort = mergesort_ib;
    printf("Test sorting with iterative buffered mergesort\n");
    run_tests(sorttests, sizeof(sorttests)/sizeof(test_t));

    fn_sort = mergesort_r;
    printf("Test sorting with recursive mergesort\n");
    run_tests(sorttests, sizeof(sorttests)/sizeof(test_t));

    fn_sort = mergesort_rm;
    printf("Test sorting with multithreaded recursive mergesort\n");
    run_tests(sorttests, sizeof(sorttests)/sizeof(test_t));

    // fn_sort = mergesort_s;
    // printf("Test sorting with single-threaded iterative mergesort\n");
    // run_tests(sorttests, sizeof(sorttests)/sizeof(test_t));

    // fn_sort = mergesort_m;
    // printf("Test sorting with multithreaded iterative mergesort\n");
    // run_tests(sorttests, sizeof(sorttests)/sizeof(test_t));

    printf("Done!\n");

    free(arr_sorted);
    free(arr_unsorted);
}

