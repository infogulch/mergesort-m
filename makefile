CC=gcc
CFLAGS=-I. -std=c99 -Wall -Wextra -ggdb -O2 -fms-extensions -pthread
LFLAGS=-lm
Q=@

# Dependencies:
DEPS = sort.h

# Object Files:
OBJ = sort.o

%.o: %.c $(DEPS)
	@echo "compile $@"
	$(Q)$(CC) -c -o $@ $< $(CFLAGS)

test: test.o $(OBJ)
	@echo "link $@"
	$(Q)$(CC) -o $@ $^ $(CFLAGS) $(LFLAGS)

run: test
	$(Q)./test ${ARGS}

clean:
	$(Q)rm -f *.o test

grind: test
	$(Q)valgrind --leak-check=full --track-origins=yes ./test ${ARGS}

gdb: test
	$(Q)gdb --args ./test ${ARGS}

