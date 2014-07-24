# README of NOS component makefile
# Component developer only need to set 
# COMP_NAME, COMP_SRCS and DEFINES 

COMP_NAME = pif
COMP_VER = 0.0.1
PIF_SRCS = pifMain.c \
			pifDataTypes.c \
			pif.c \
			pifTimer.c \
			pifMsgIpc.c \
			pifMsgEvent.c \
			pifPortMgr.c \
			pifAggMgr.c \
			pifInterfaceMgr.c \
			pifVlanMgr.c \
			pifCmdApi.c \
			pifFeaApi.c \
			pifCmd_GEN.c \
			pifCmd.c 
FEA_SRCS = \
			./fea/pifFeaInterface.c \
			./fea/pifFeaBridge.c \
			./fea/pifFeaVlan.c \
			./fea/pifFeaLacp.c \
			./fea/pifFea.c 
HAL_SRCS = \
			./hal/l2swfwdr/hal.c \
			./hal/l2swfwdr/hal_bridge.c \
			./hal/l2swfwdr/hal_garp.c \
			./hal/l2swfwdr/hal_if_generic.c \
			./hal/l2swfwdr/hal_l2_fdb.c \
			./hal/l2swfwdr/hal_lacp.c 
PAL_SRCS = \
			./pal/interface.c \
			./pal/connected.c \
			./pal/palKernelIfTypes.c \
			./pal/palKernelProc.c \
			./pal/palKernelMii.c \
			./pal/palKernelNetlink.c \
			./pal/palKernelIoctl.c \
			./pal/palKernel.c 
COMP_SRCS = $(HAL_SRCS) $(PAL_SRCS) $(FEA_SRCS) $(PIF_SRCS)

DEFINES   = -DHAVE_NETLINK -DENABLE_PAL_PATH -DENABLE_HAL_PATH -DHAVE_L3
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

INCLUDES = -I. -I./fea -I./pal -I./hal -I./hal/L2 -I./hal/L3 -I./hal/l2swfwdr 
INCLUDES += -I$(NOSINCPATH) -I$(NOSINCPATH)/include -I$(NOSCMDPATH) -I$(NOSEXTPATH) -I$(NOSINCPATH)/riblib
LDPATH   = -L$(NOSLIBPATH) -L/usr/local/lib

CFLAGS   = -g -Wall -Wno-unused-function -Wpointer-arith -Wshadow -Winline $(DEFINES) $(INCLUDES)
LDFLAGS  = -shared
LIBS     = -lpthread -lnoslib -ldl -lrt
NOSDATA  = -lnosdata -lcmdlib

COMP_SRCS += $(NOSINCPATH)/nosLib.c
COMP_OBJS = $(COMP_SRCS:.c=.o)
TASK_SRCS = $(NOSINCPATH)/dlu.c ./compMain.c 
TASK_OBJS = $(TASK_SRCS:.c=.o)

TARGET     = $(COMP_NAME)d
TARGET_LIB = lib$(COMP_NAME)_$(COMP_VER).so

.PHONY: all clean
all: $(EXTHDR) $(TARGET_LIB) $(TARGET)
	cp $(TARGET) $(TARGET_LIB) $(NOSBINPATH)

$(EXTHDR): $(SHARED_HEADER)
	$(NOSEXTPATH)/exthdr.pl $(SHARED_HEADER)
	$(RM) $(NOSINCPATH)/nosLib.o

$(TARGET_LIB): $(COMP_OBJS)
	$(CC) ${LDFLAGS} -o $@ $^ -Wl,-whole-archive $(NOSDATA) -Wl,-no-whole-archive $(LDPATH)

$(COMP_OBJS):%.o:%.c
	$(CC) -c -fPIC $(CFLAGS) $< -o $@

$(TARGET): $(TASK_OBJS)
	$(CC) -o $@ $(TASK_OBJS) $(LIBS) $(LDPATH) 

$(TASK_OBJS):%.o:%.c
	$(CC) -c $(CFLAGS) $< -o $@

install:
	$(CP) $(TARGET) $(NOSBINPATH)
	$(CP) $(TARGET_LIB) $(NOSBINPATH)
	$(CP) pifMsgEventData.h $(NOSEXTPATH)
	$(CP) pifMsgIpc.h $(NOSEXTPATH)

clean:
	$(RM) $(TARGET_LIB) $(COMP_OBJS) $(SRC:.c=.d)
	$(RM) $(TARGET) $(TASK_OBJS)
	$(RM) $(NOSBINPATH)/$(TARGET)
	$(RM) $(NOSBINPATH)/$(TARGET_LIB)
	$(RM) $(NOSEXTPATH)/pifMsgEventData.h $(NOSEXTPATH)/pifMsgIpc.h
