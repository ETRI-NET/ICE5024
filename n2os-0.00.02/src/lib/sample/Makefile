CC	:= gcc
CFLAGS	:=
TARGET1	:= cqueue_sample
TARGET2	:= list_sample
TARGET3	:= avl_sample
TARGET4	:= hash_sample
TARGET5	:= memmgr_sample
TARGET6	:= str_sample
TARGET7	:= log_sample
TARGET8	:= prefix_sample
TARGET9	:= avllong_sample
TARGET10:= avlstr_sample

INCLUDES:= -I../ -I../include -L../../../lib
LIBS	:= -l n2oslib -l rt

all : $(TARGET1) $(TARGET2) $(TARGET3) $(TARGET4) $(TARGET5) $(TARGET6) $(TARGET7) $(TARGET8) $(TARGET9) $(TARGET10)

$(TARGET1) : cqueue_sample.o
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^ $(LIBS);

$(TARGET2) : list_sample.o
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^ $(LIBS);

$(TARGET3) : avl_sample.o
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^ $(LIBS);

$(TARGET4) : hash_sample.o
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^ $(LIBS);

$(TARGET5) : memmgr_sample.o
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^ $(LIBS);

$(TARGET6) : str_sample.o
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^ $(LIBS);

$(TARGET7) : log_sample.o
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^ $(LIBS);

$(TARGET8) : prefix_sample.o
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^ $(LIBS);

$(TARGET9) : avllong_sample.o
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^ $(LIBS);

$(TARGET10) : avlstr_sample.o
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^ $(LIBS);

cqueue_sample.o : cqueue_sample.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $^

list_sample.o : list_sample.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $^

avl_sample.o : avl_sample.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $^

hash_sample.o : hash_sample.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $^

memmgr_sample.o : memmgr_sample.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $^

str_sample.o : str_sample.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $^

log_sample.o : log_sample.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $^

prefix_sample.o : prefix_sample.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $^

avllong_sample.o : avllong_sample.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $^

avlstr_sample.o : avlstr_sample.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $^

clean:
	rm -f *.o
	rm -f cqueue_sample list_sample avl_sample hash_sample memmgr_sample str_sample log_sample prefix_sample avllong_sample avlstr_sample
