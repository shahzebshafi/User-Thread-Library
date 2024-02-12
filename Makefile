CC = gcc
CFLAGS = -Wall -Wextra -Werror
LIB = libuthread.a
OBJS = queue.o context.o uthread.o preempt.o

all: $(LIB)

$(LIB): $(OBJS)
	ar rcs $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(LIB)


