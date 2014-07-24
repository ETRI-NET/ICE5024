.SUFFIXES = .c .o 

INCLUDES:= -I./ -I./include -I../cmd/common -I./riblib
CFLAGS   = -O2 -g -Wall -fno-strict-aliasing -Wno-unused-function $(INCLUDES)\
		   -fPIC
LDFLAGS  = -shared
DEFS    := -DHAVE_INET_NTOP

NOS_CMDLIB_PATH = ./cmdlib

NOS_LIB_OBJS = ipcService.o eventService.o taskManager.o \
               nnMemmgr.o nnLog.o nnTime.o nnBuffer.o \
               $(NOS_CMDLIB_PATH)/compStatic.o \

NOS_DATA_OBJS = nnCqueue.o nnStr.o nnHash.o nnList.o \
                nnAvl.o nnAvlLong.o nnAvlStr.o nnBuffer.o \
                nnUtility.o nnPrefix.o nnTime.o

#NOS_LCS_OBJS = lcsService.o lcsUtility.o


NOS_LIB_SRCS = $(NOS_LIB_OBJS:.o=.c)
NOS_DATA_SRCS = $(NOS_DATA_OBJS:.o=.c)
#NOS_LCS_SRCS = $(NOS_LCS_OBJS:.o=.c)

LIBEVENT_HOME = ../3rd/libevent-2.1.3-alpha

LIBEVENT_TARGET = ../lib/3rd/lib/libevent.a 
NOS_LIB_TARGET = libnoslib.a
NOS_DATA_TARGET = libnosdata.a
#NOS_LCS_TARGET = libnoslcs.a


NOS_COMP_LIB_DIRS := riblib cmdlib lcslib


#all : $(NOS_LIB_TARGET) $(NOS_DATA_TARGET) $(NOS_LCS_TARGET)
all : $(NOS_LIB_TARGET) $(NOS_DATA_TARGET)
	@for dir in $(NOS_COMP_LIB_DIRS) ; do \
	make -C $$dir || exit $?;\
	done

$(NOS_LIB_TARGET) : $(NOS_LIB_OBJS)
	@for dir in $(LIBEVENT_HOME) ; do \
	make all install -C $$dir ; \
	done
	cp $(LIBEVENT_TARGET) $(NOS_LIB_TARGET)
	$(AR) rsv $(NOS_LIB_TARGET) $(NOS_LIB_OBJS)
	cp $(NOS_LIB_TARGET) ../../lib

$(NOS_DATA_TARGET) : $(NOS_DATA_OBJS)
	$(AR) rsv $(NOS_DATA_TARGET) $(NOS_DATA_OBJS)
	cp $(NOS_DATA_TARGET) ../../lib

#$(NOS_LCS_TARGET) : $(NOS_LCS_OBJS)
#	$(AR) rsv $(NOS_LCS_TARGET) $(NOS_LCS_OBJS)
#	cp $(NOS_LCS_TARGET) ../../lib

dep :
#	gccmakedep $(NOS_LIB_SRCS) $(NOS_DATA_SRCS) $(NOS_LCS_SRCS)
	gccmakedep $(NOS_LIB_SRCS) $(NOS_DATA_SRCS)


clean:
	rm -f *.o $(NOS_LIB_TARGET) $(NOS_DATA_TARGET)
	rm -f ../../lib/$(NOS_LIB_TARGET) \
		  ../../lib/$(NOS_DATA_TARGET)
#\		  ../../lib/$(NOS_LCS_TARGET)
	@for dir in $(NOS_COMP_LIB_DIRS) ; do \
	make -C $$dir clean;\
	done

