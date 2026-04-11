CC = gcc
CFLAGS = -Wall -Wextra -I/usr/include/postgresql

TARGET = main

# all c files in current dir + your other library files
SRCS = main.c ../reading.c ../db.c ../quit.c reading_list.c read.c

# convert source files to object files
OBJS = $(SRCS:.c=.o)

# Link WiringPi and math library
LIBS = -lwiringPi -lm -lpq

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
