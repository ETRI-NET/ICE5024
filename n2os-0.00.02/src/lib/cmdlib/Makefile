# README of NOS component makefile
# Component developer only need to set 
# COMP_NAME, COMP_SRCS and DEFINES 

COMP_NAME = cmdlib

COMP_SRCS = nnCmdMd5.c nnCmdMsg.c nnCmdSha1.c nnCmdSha2.c \
		nnCmdSha4.c nnCmdLink.c nnCmdCommon.c nnCmdInstall.c nnCmdCli.c nnCmdNode.c nnCmdCmsh.c nnCompProcess.c

GLOBAL_CMD_SRC = nnGlobalCmd.c

GLOBAL_CMD_SRCS = ../../cmd/cmsh/cmshMgrCmd.c ../../cmd/cm/cmMgrCmd.c \
../../polmgr/polMgrCmd.c ../../ribmgr/ribmgrCmd.c ../../ripd/ripCmd.c \
../../dluTester/dluTesterCmd.c ../../processmgr/processmgrCmd.c \
../../ipcmgr/ipcmgrCmd.c \
../../isisd/isisCmd.c ../../ospfd/ospfCmd.c\
../../pifmgr/pifCmd.c \
../../lacpd/lacpCmd.c

COMP_SRCS += $(GLOBAL_CMD_SRC)

DEFINES   = -Wpointer-arith -Wshadow -Winline 
NOSPATH   = ../../..

# Macro belows should not be modified 
CC  = gcc
MV  = mv
CP  = cp
RM  = rm -f

NOSINCPATH = $(NOSPATH)/src/lib
NOSLIBPATH = $(NOSPATH)/lib
NOSCMDPATH = $(NOSPATH)/src/lib/cmdlib

INCLUDES = -I. -I./nos -I$(NOSINCPATH) -I$(NOSINCPATH)/include -I$(NOSCMDPATH) 

CFLAGS   = -g -fPIC -Wall $(DEFINES) $(INCLUDES)

COMP_OBJS = $(COMP_SRCS:.c=.o)

TARGET_LIB = lib$(COMP_NAME).a

.PHONY: all clean
all: clear $(GLOBAL_CMD_SRC) $(TARGET_LIB)

$(TARGET_LIB): $(COMP_OBJS)
	$(AR) rsv $@ $^
	$(CP) $(TARGET_LIB) $(NOSPATH)/lib/

$(COMP_OBJS):%.o:%.c
	$(CC) -c $(CFLAGS) $< -o $@

$(GLOBAL_CMD_SRC): $(GLOBAL_CMD_SRCS)
	./cmdMiner.pl $(GLOBAL_CMD_SRCS) || exit;

install:
	$(CP) $(TARGET_LIB) $(NOSPATH)/lib/

clear:
	$(RM) $(GLOBAL_CMD_SRC)
	
clean:
	$(RM) $(TARGET_LIB) $(COMP_OBJS) $(SRC:.c=.d)
	$(RM) $(NOSPATH)/lib/$(TARGET_LIB)
