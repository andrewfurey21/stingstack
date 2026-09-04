SRCS := $(wildcard *.c)
OBJS := $(SRCS:.c=.o)
FLAGS:= -g

acc: $(OBJS)
	$(CC) $(OBJS) $(FLAGS) -o $@

%.o: %.c
	$(CC) -c $< -o $@

clean:
	rm acc *.o -f
