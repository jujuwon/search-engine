CC=gcc
OBJS=main.o
TARGET=search-engine

all : $(TARGET)

clean : 
	rm -f $(OBJS)
	rm -f $(TARGET)

$(TARGET) : $(OBJS)
	$(CC) -o $@ $(OBJS) -lm

main.o : main.c