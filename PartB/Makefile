CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Iinclude
LDLIBS = -lrt

SRCS = src/main.c src/config.c src/shared_mem.c src/files.c src/process.c src/grader.c src/util.c src/semaphores.c
OBJS = $(SRCS:.c=.o)
TARGET = grading_sim

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) $(LDLIBS)

clean:
	rm -f $(TARGET) *.o
