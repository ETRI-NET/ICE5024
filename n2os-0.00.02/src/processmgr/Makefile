# README of NOS component makefile
# Component developer only need to set 
# COMP_NAME, COMP_SRCS and DEFINES 

COMP_NAME = procmgr
COMP_VER = 0.0.1
COMP_SRCS = processmgrMain.c processmgrTask.c processmgrIpc.c processmgrCmIpc.c processmgrCmd.c processmgrCmd_GEN.c

DEFINES   = -DIS_PROCMGR
NOSPATH   = ../..

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


INCLUDES = -I. -I$(NOSINCPATH) -I$(NOSINCPATH)/include -I$(NOSCMDPATH) -I$(NOSEXTPATH) -I$(NOSINCPATH)/lcslib
LDPATH   = -L$(NOSLIBPATH) -L/usr/local/lib

CFLAGS   = -g -Wall -Wno-unused-function -Wpointer-arith -Wshadow -Winline $(DEFINES) $(INCLUDES)
LDFLAGS  = -shared
LIBS     = -lpthread -lnoslib -lcmdlib -ldl -lrt
NOSDATA  = -lnosdata -lnoslcs -lcmdlib

COMP_SRCS += $(NOSINCPATH)/nosLib.c 
COMP_OBJS = $(COMP_SRCS:.c=.o)
TASK_SRCS = $(NOSINCPATH)/dlu.c ./compMain.c ./processmgrUtility.c
TASK_OBJS = $(TASK_SRCS:.c=.o)

TARGET     = $(COMP_NAME)d
TARGET_LIB = lib$(COMP_NAME)_$(COMP_VER).so
EXTHDR     = exthdr

CONF_FILE  = procmgr.conf
SCRIPT_FILE = n2os

.PHONY: all clean
all:$(EXTHDR) $(TARGET_LIB) $(TARGET)

$(EXTHDR): $(SHARED_HEADER)
	$(NOSEXTPATH)/exthdr.pl $(SHARED_HEADER)
	$(RM) $(NOSINCPATH)/nosLib.o

$(TARGET_LIB): $(COMP_OBJS)
	$(CC) ${LDFLAGS} -o $@ $^ -Wl,-whole-archive $(NOSDATA) -Wl,-no-whole-archive $(LDPATH)
	$(CP) $(TARGET_LIB) $(NOSPATH)/bin

$(COMP_OBJS):%.o:%.c
	$(CC) -c -fPIC $(CFLAGS) $< -o $@
	
$(TARGET): $(TASK_OBJS)
	$(CC) -o $@ $(TASK_OBJS) $(LIBS) $(LDPATH) 
	$(CP) $(TARGET) $(NOSPATH)/bin
	$(CP) $(CONF_FILE) $(NOSPATH)/bin
	$(CP) $(SCRIPT_FILE) $(NOSPATH)/bin

$(TASK_OBJS):%.o:%.c
	$(CC) -c $(CFLAGS) $< -o $@

install:
	$(CP) $(TARGET) $(NOSBINPATH)
	$(CP) $(TARGET_LIB) $(NOSBINPATH)/.libs
	
clean:
	$(RM) $(TARGET_LIB) $(COMP_OBJS) $(SRC:.c=.d)
	$(RM) $(TARGET) $(TASK_OBJS)
#	$(RM) $(NOSBINPATH)/$(TARGET)
#	$(RM) $(NOSLIBPATH)/$(TARGET_LIB)
