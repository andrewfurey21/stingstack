
SRCS := $(wildcard *.c)
OBJS := $(SRCS:%.c=build/%.o)
DEPS := $(OBJS:.o=.d)
CC   := gcc

FLAGS := -g -MMD -MP

acc: $(OBJS)
	$(CC) $(OBJS) -o $@

build:
	mkdir -p build

build/%.o: %.c | build
	$(CC) $(FLAGS) -c $< -o $@

-include $(DEPS)

clean:
	rm -f acc
	rm -rf build

