CC = gcc
CFLAGS = -Wall -Wextra

TARGET = main
SRCS = main.c ../quit.c lcd_meter.c ../db.c ../reading.c
OBJS = $(SRCS:.c=.o)

# Link I2C library
LIBS = -li2c -lpq

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
