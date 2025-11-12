CC ?= gcc
CFLAGS ?= -std=c11 -O2 -Wall -Wextra
INCLUDES := -Iinclude

TARGET := scheduling
SRCS := main.c job.c queue.c utils.c scheduler.c scheduler_rr.c scheduler_sjf.c scheduler_mlfq.c
OBJS := $(SRCS:.c=.o)

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

run: $(TARGET)
	@ if [ -z "$(INPUT)" ]; then echo "Usage: make run INPUT=path/to/input_file.txt"; exit 1; fi
	./$(TARGET) < "$(INPUT)"

clean:
	$(RM) $(OBJS) $(TARGET)

