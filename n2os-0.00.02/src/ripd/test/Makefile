CC 	:= gcc 
CFLAGS 	:=
TARGET 	:= riptester

DEFS 	:= 
INCLUDES:= -I. -I../../lib -I../../lib/include -I../ -L../../../lib 
LIBS	:= -lpthread -ln2oslib -lrt

all : $(TARGET)

#$(TARGET) : ripTester.o
#	$(CC) $(CFLAGS) $(DEFS) $(INCLUDES) -o $@ $^ $(LIBS);
#	cp $(TARGET) ../../bin

riptester : ripTester.o 
	$(CC) $(CFLAGS) $(DEFS) $(INCLUDES) -o $@ $^ $(LIBS);

ripTester.o : ripTester.c
	$(CC) $(CFLAGS) $(DEFS) $(INCLUDES) -c -o $@ $^
clean :
	rm -rf *.o riptester
