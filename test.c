#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <sort.h>

#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_RESET     "\x1b[0m"

int arr_len;
int *arr_sorted;
int *arr_unsorted;

void (*fn_sort)(int *, size_t) = NULL;

int arr_equals(int *arr1, int *arr2, size_t len)
{
    for (size_t i = 0; i < len; i++)
        if (arr1[i] != arr2[i])
            return 0;
    return 1;
}

void arr_copy(int *arr_to, int *arr_fm, size_t len)
{
    for (size_t i = 0; i < len; i++)
        arr_to[i] = arr_fm[i];
}

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

int test_arr_equals()
{
    int arr1[] = {5,7,9, 1};
    int arr2[] = {5,7,9,10};
    return arr_equals(arr1,arr2,3) && !arr_equals(arr1, arr2, 4);
}

int test_arr_copy()
{
    int arr1[] = {3,7,2,5};
    int arr2[4];
    arr_copy(arr2, arr1, 4);
    return arr_equals(arr1, arr2, 4);
}

int test_arr_shuffle()
{
    int arr1[] = {1,2,3,4,5};
    int arr2[] = {2,5,4,3,1};
    srand(31337);
    arr_shuffle(arr1, 5);
    return arr_equals(arr1, arr2, 5);
}

int test_none()
{
    int arr[arr_len];
    arr_copy(arr, arr_unsorted, arr_len);
    fn_sort(arr, 0);
    return arr_equals(arr, arr_unsorted, arr_len);
}

int test_one()
{
    int arr[arr_len];
    arr_copy(arr, arr_unsorted, arr_len);
    fn_sort(arr, 1);
    return arr_equals(arr, arr_unsorted, arr_len);
}

int test_two_sorted()
{
    int arr[] = {3, 7};
    fn_sort(arr, 2);
    return arr_equals(arr, arr, 2);
}

int test_many_sorted()
{
    int arr[arr_len];
    arr_copy(arr, arr_sorted, arr_len);
    fn_sort(arr, arr_len);
    return arr_equals(arr, arr_sorted, arr_len);
}

int test_two_unsorted()
{
    int arr1[] = {7, 3};
    int arr2[] = {3, 7};
    fn_sort(arr1, 2);
    int result = arr_equals(arr1, arr2, 2);
    /*
    if (!result)
    {
        printf("Got: ");
        arr_print(arr1, 2);
        printf("Should be: ");
        arr_print(arr2, 2);
    }
    */
    return result;
}

int test_many_unsorted()
{
    int arr[arr_len];
    arr_copy(arr, arr_unsorted, arr_len);
#ifdef DEBUG
    printf("    Before: "); arr_print(arr, arr_len);
#endif
    fn_sort(arr, arr_len);
#ifdef DEBUG
    printf("    After: "); arr_print(arr, arr_len);
#endif
    return arr_equals(arr, arr_sorted, arr_len);
}

int test_merge_i()
{
    int arr2[] = {9,9,9, 1,2,3,4,5,6,7,8,  0,0,0}
      , arr1[] = {9,9,9, 1,3,5,7, 2,4,6,8, 0,0,0};
#ifdef DEBUG
    printf("\n    Before: "); arr_print(arr1, 14);
#endif
    merge_i(&arr1[3], &arr1[7], &arr1[11]);
#ifdef DEBUG
    printf("\n    After: "); arr_print(arr1, 14);
#endif
    return arr_equals(arr1, arr2, 14);
}

int test_merge_b()
{
    int section = 1, length = 8;
    int arr1[] = {0,9,0,9,0,9,0,9, 1,3,5,7,2,4,6,8, 0,9,0,9,0,9,0,9};
    int arr2[] = {0,9,0,9,0,9,0,9, 1,2,3,4,5,6,7,8, 0,9,0,9,0,9,0,9};
    int *arr3 = malloc(8*sizeof(int));
#ifdef DEBUG
    printf("\n    Before: "); arr_print(arr1, 24);
#endif
    int *l = &arr1[section*length];
    int *r = &arr1[section*length + length/2];
    int *e = &arr1[(section+1)*length];
    merge_b(l, r, e, arr3);
#ifdef DEBUG
    printf("\n    After: "); arr_print(arr1, 24);
#endif
    free(arr3);
    return arr_equals(arr1, arr2, 14);
}

typedef struct {
    const char *name;
    int (*func)();
} test_t;

#define TEST(x) { #x, x }
test_t arraytests[] = {
      TEST(test_arr_equals)
    , TEST(test_arr_copy)
    , TEST(test_arr_shuffle)
};

test_t sorttests[] = {
      TEST(test_none)
    , TEST(test_one)
    , TEST(test_two_sorted)
    , TEST(test_many_sorted)
    , TEST(test_two_unsorted)
    , TEST(test_many_unsorted)
};

test_t mergetests[] = {
      TEST(test_merge_i)
    , TEST(test_merge_b)
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

    for (int i = 0; i < arr_len; i++)
        arr_sorted[i] = i;
    arr_copy(arr_unsorted, arr_sorted, arr_len);
    srand(0);
    arr_shuffle(arr_unsorted, arr_len);

    // Omit this test, it's unlikely to change and it takes too long.
    // fn_sort = insertionsort;
    // printf("Test sorting with insertionsort\n");
    // run_tests(sorttests, sizeof(sorttests)/sizeof(test_t));

    printf("Test merging\n");
    run_tests(mergetests, sizeof(mergetests)/sizeof(test_t));

    // fn_sort = mergesort_i;
    // printf("Test sorting with iterative mergesort\n");
    // run_tests(sorttests, sizeof(sorttests)/sizeof(test_t));

    fn_sort = mergesort_f;
    printf("Test sorting with flat mergesort\n");
    run_tests(sorttests, sizeof(sorttests)/sizeof(test_t));

    fn_sort = mergesort_m;
    printf("Test sorting with multithreaded mergesort\n");
    run_tests(sorttests, sizeof(sorttests)/sizeof(test_t));

    printf("Done!\n");

    free(arr_sorted);
    free(arr_unsorted);
}

