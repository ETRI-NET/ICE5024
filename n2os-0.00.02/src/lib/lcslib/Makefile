.SUFFIXES = .c .o 

INCLUDES:= -I./ -I../ -I../include -I../cmdlib
CFLAGS   = -O2 -g -Wall -fno-strict-aliasing -Wno-unused-function -fPIC $(INCLUDES)
LDFLAGS  = -shared
DEFS    := -DHAVE_INET_NTOP

NOS_LCS_OBJS = lcsService.o lcsUtility.o

NOS_LCS_SRCS = $(NOS_LCS_OBJS:.o=.c)
NOS_LCS_TARGET = libnoslcs.a 

all : $(NOS_LCS_TARGET)

$(NOS_LCS_TARGET) : $(NOS_LCS_OBJS)
	$(AR) rsv $(NOS_LCS_TARGET) $(NOS_LCS_OBJS)
	cp $(NOS_LCS_TARGET) ../../../lib


dep :
	gccmakedep $(NOS_LCS_SRCS) 


clean:
	rm -f *.o $(NOS_LCS_TARGET)
	rm -f ../../../lib/$(NOS_LCS_TARGET)
