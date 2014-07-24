# README of NOS component makefile
# Component developer only need to set 
# COMP_NAME, COMP_SRCS and DEFINES 

COMP_NAME = isis
COMP_VER = 0.0.1
COMP_SRCS = dict.c isisInit.c isisIpc.c isis_adjacency.c isis_circuit.c isis_dlpi.c isis_dynhn.c isis_flags.c isis_pdu.c isis_route.c isis_spf.c isis_zebra.c iso_checksum.c \
	isisInterface.c isisUtil.c isis_bpf.c isis_csm.c isis_dr.c isis_events.c isis_lsp.c isis_misc.c isis_pfpacket.c isis_routemap.c isis_tlv.c isisd.c isisRibmgr.c \
	isisDebug.c isisCmd.c isisCmd_GEN.c


DEFINES   = -DHAVE_NETLINK -D__USE_MISC -DIS_SHARED_LIBRARY -DHAVE_BSD_STRUCT_IP_MREQ_HACK
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
QUAGGAPATH = $(NOSPATH)/src/3rd/libquagga2/lib

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

INCLUDES = -I. -I$(QUAGGAPATH) -I$(NOSINCPATH) -I$(NOSINCPATH)/include -I$(NOSCMDPATH) -I$(NOSEXTPATH) -I$(NOSINCPATH)/riblib -I$(NOSINCPATH)/lcslib 
LDPATH   = -L$(NOSLIBPATH) -L/usr/local/lib 

CFLAGS   = -g -Wall -Wno-unused-function -Wpointer-arith -Wshadow -Winline $(DEFINES) $(INCLUDES)
LDFLAGS  = -shared
LIBS     = -lpthread -lnoslib -ldl -lrt
NOSDATA  = -lnosdata -lnoslcs -lnosrib -lcmdlib -lzebra  

COMP_SRCS += $(NOSINCPATH)/nosLib.c
COMP_OBJS = $(COMP_SRCS:.c=.o)
TASK_SRCS = $(NOSINCPATH)/dlu.c ./isisDluMain.c $(NOSCMDPATH)/compStatic.c
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
 
clean:
	$(RM) $(TARGET_LIB) $(COMP_OBJS) $(SRC:.c=.d)
	$(RM) $(TARGET) $(TASK_OBJS)
	$(RM) $(NOSBINPATH)/$(TARGET)
	$(RM) $(NOSBINPATH)/$(TARGET_LIB)
