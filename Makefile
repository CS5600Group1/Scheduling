CC ?= gcc
CFLAGS ?= -std=c11 -O2 -Wall -Wextra
INCLUDES := -Iinclude

TARGET := scheduling
SRCS := main.c job.c queue.c utils.c clock.c scheduler.c scheduler_rr.c scheduler_sjf.c scheduler_mlfq.c
OBJS := $(SRCS:.c=.o)

.PHONY: all build-sjf build-rr build-mlfq clean run run-sjf run-rr run-mlfq

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

run: $(TARGET)
	@ if [ -z "$(INPUT)" ]; then echo "Usage: make run INPUT=path/to/input_file.txt"; exit 1; fi
	./$(TARGET) $(ARGS) < "$(INPUT)"

build-sjf:
	$(CC) $(CFLAGS) $(INCLUDES) -DBUILD_SJF_ONLY -o $(TARGET)_sjf main.c job.c queue.c utils.c clock.c scheduler.c scheduler_sjf.c

build-rr:
	$(CC) $(CFLAGS) $(INCLUDES) -DBUILD_RR_ONLY -o $(TARGET)_rr main.c job.c queue.c utils.c clock.c scheduler.c scheduler_rr.c

build-mlfq:
	$(CC) $(CFLAGS) $(INCLUDES) -DBUILD_MLFQ_ONLY -o $(TARGET)_mlfq main.c job.c queue.c utils.c clock.c scheduler.c scheduler_mlfq.c

run-sjf: $(TARGET)
	@ if [ -z "$(INPUT)" ]; then echo "Usage: make run-sjf INPUT=path/to/input_file.txt"; exit 1; fi
	./$(TARGET) --policy sjf $(ARGS) < "$(INPUT)"

run-rr: $(TARGET)
	@ if [ -z "$(INPUT)" ]; then echo "Usage: make run-rr INPUT=path/to/input_file.txt QUANTUM=N"; exit 1; fi
	@ if [ -z "$(QUANTUM)" ]; then echo "Please provide QUANTUM for round-robin: make run-rr INPUT=... QUANTUM=4"; exit 1; fi
	./$(TARGET) --policy rr --quantum $(QUANTUM) $(ARGS) < "$(INPUT)"

run-mlfq: $(TARGET)
	@ if [ -z "$(INPUT)" ]; then echo "Usage: make run-mlfq INPUT=path/to/input_file.txt"; exit 1; fi
	./$(TARGET) --policy mlfq $(ARGS) < "$(INPUT)"

clean:
	$(RM) $(OBJS) $(TARGET)

