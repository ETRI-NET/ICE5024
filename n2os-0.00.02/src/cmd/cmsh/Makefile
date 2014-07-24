# README of NOS component makefile
# Component developer only need to set 
# COMP_NAME, COMP_SRCS and DEFINES 

COMP_NAME = cmsh
COMP_VER = 0.0.1

DEFINES   = -D_GNU_SOURCE
NOSPATH   = ../../..

# Macro belows should not be modified 
CC  = gcc
MV  = mv
CP  = cp
RM  = rm -f

NOSINCPATH = $(NOSPATH)/src/lib
NOSLIBPATH = $(NOSPATH)/lib
NOSBINPATH = $(NOSPATH)/bin
NOSCMDPATH = $(NOSPATH)/src/lib/cmdlib
NOSEXTPATH = $(NOSPATH)/src/exthdr


NOSEDTPATH = $(NOSPATH)/src/lib/3rd/include
NOSEDTLIBS = $(NOSPATH)/src/lib/3rd/lib

SHARED_HEADER = $(NOSCMDPATH)/nnCmdCli.h \
                $(NOSCMDPATH)/nnCmdCmsh.h  \
                $(NOSCMDPATH)/nnCmdCommon.h  \
                $(NOSCMDPATH)/nnCmdCrypto.h  \
                $(NOSCMDPATH)/nnCmdDefines.h \
                $(NOSCMDPATH)/nnCmdInstall.h \
                $(NOSCMDPATH)/nnCmdLink.h  \
                $(NOSCMDPATH)/nnCmdMd5.h \
                $(NOSCMDPATH)/nnCmdMsg.h \
                $(NOSCMDPATH)/nnCmdNode.h  \
                $(NOSCMDPATH)/nnCmdSha1.h  \
                $(NOSCMDPATH)/nnCmdSha2.h  \
                $(NOSCMDPATH)/nnCmdSha4.h  \
                $(NOSCMDPATH)/nnGlobalCmd.h

INCLUDES = -I. -I$(NOSINCPATH) -I$(NOSINCPATH)/include -I$(NOSCMDPATH) -I$(NOSEDTPATH)
LDPATH   = -L$(NOSLIBPATH) -L$(NOSEDTLIBS) -L/usr/local/lib

CFLAGS   = -g -Wall -Wno-unused-function -Wpointer-arith -Wshadow -Winline $(DEFINES) $(INCLUDES)
LIBS     = -lpthread -lnoslib -lcmdlib -ldl -lrt -lcurses
NOSDATA  = -lnosdata -lcmdlib

TASK_SRCS = ./cmshMain.c ./cmshMgrCmIpc.c ./cmshMgrCmd.c ./cmshMgrCmd_GEN.c
TASK_SRCS += $(NOSCMDPATH)/compStatic.c
TASK_OBJS = $(TASK_SRCS:.c=.o)

TARGET     = $(COMP_NAME)
EXTHDR     = exthdr

.PHONY: all clean
all: $(EXTHDR) $(TARGET)

$(EXTHDR): $(SHARED_HEADER)
	$(NOSEXTPATH)/exthdr.pl $(SHARED_HEADER)

$(TARGET): $(TASK_OBJS)
	$(CC) -o $@ $(TASK_OBJS) $(NOSEDTLIBS)/libedit.a $(LIBS) $(LDPATH) 

$(TASK_OBJS):%.o:%.c
	$(CC) -c $(CFLAGS) $< -o $@

install:
	$(CP) $(TARGET) $(NOSBINPATH)
	
clean:
	$(RM) $(TARGET) $(TASK_OBJS)
	$(RM) $(NOSBINPATH)/$(TARGET)
