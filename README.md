# Multithreaded Merge Sort in C
This is just a test case and only sorts integers atm. After it actually starts working I might follow the example of `qsort()` and change to use `void*`, `width`, and a caller-defined comparator function.

I attempted to do something resembling unit testing, and it halfway works, but it's not very clean and it certainly doesn't have enough tests.

All sorting tests are checked for correctness.

## Compiling
```
make
```

## Running tests
To run a test on 30 items (default):
```
./test
```

To test a larger number of items:
```
./test 10000
```

### Valgrind
To run the tests in valgrind:
```
make grind
```
    
To test a different number of items:
```
make grind ARGS=10000
```
