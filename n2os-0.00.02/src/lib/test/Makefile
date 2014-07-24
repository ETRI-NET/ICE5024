CC	:= gcc
CFLAGS	:= -O2 -g -Wall
TARGET1	:= list_test
TARGET2 := avl_test
TARGET3 := hash_test
TARGET4 := cqueue_test
TARGET5 := memMgr_test

INCLUDES:= -L../../../lib
LIBS	:= -l n2oslib

all : $(TARGET1) $(TARGET2) $(TARGET3) $(TARGET4) $(TARGET5)

$(TARGET1) : list_test.o test.o
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^ $(LIBS);

$(TARGET2) : avl_test.o test.o
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^ $(LIBS);

$(TARGET3) : hash_test.o test.o
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^ $(LIBS);

$(TARGET4) : cqueue_test.o test.o
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^ $(LIBS);

$(TARGET5) : memMgr_test.o test.o
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^ $(LIBS);

list_test.o : list_test.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $^

avl_test.o : avl_test.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $^

hash_test.o : hash_test.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $^

cqueue_test.o : cqueue_test.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $^

memMgr_test.o : memMgr_test.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $^

test.o : test.c
	$(CC) -c $^

clean:
	rm -rf *.o $(TARGET1) $(TARGET2) $(TARGET3) $(TARGET4) $(TARGET5)
