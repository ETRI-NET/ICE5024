.SUFFIXES = .c .o 

INCLUDES:= -I./ -I../ -I../include -I../cmdlib
CFLAGS   = -O2 -g -Wall -fno-strict-aliasing -Wno-unused-function -fPIC $(INCLUDES)
LDFLAGS  = -shared
DEFS    := -DHAVE_INET_NTOP

NOS_RIB_OBJS = nnVector.o nnTable.o nnIf.o nnRoutemap.o nnKeychain.o nnFilter.o nnPlist.o nnSocketOption.o nnSocketUnion.o nnIfRouteMap.o nnDistribute.o hash.o
NOS_RIB_SRCS = $(NOS_RIB_OBJS:.o=.c)
NOS_RIB_TARGET = libnosrib.a 

all : $(NOS_RIB_TARGET)

$(NOS_RIB_TARGET) : $(NOS_RIB_OBJS)
	$(AR) rsv $(NOS_RIB_TARGET) $(NOS_RIB_OBJS)
	cp $(NOS_RIB_TARGET) ../../../lib


dep :
	gccmakedep $(NOS_RIB_SRCS) 


clean:
	rm -f *.o $(NOS_RIB_TARGET)
	rm -f ../../../lib/$(NOS_RIB_TARGET)
