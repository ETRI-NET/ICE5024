# README of NOS component makefile
# Component developer only need to set 
# COMP_NAME, COMP_SRCS and DEFINES 

COMP_NAME = lacp
COMP_VER = 0.0.1
COMP_SRCS = lacpLog.c lacpProc.c lacpEnv.c lacpRun.c lacpIpc.c lacpCmd.c lacpCmd_GEN.c

DEFINES   = -D__USE_MISC -DIS_SHARED_LIBRARY -DN2OS -DN2OS_KERNEL_2_6_3x -DN2OS_IFLA_ADDRESS

MODULE_PATH = ./module-team
SHARED_JANSSON_LIBS = -ljansson
SHARED_NL_LIBS = -lnl-cli-3 -lnl-genl-3 -lnl-nf-3 -lnl-route-3 -lnl-3
TEAM_SHARED_LIBS = -lm $(SHARED_JANSSON_LIBS) $(SHARED_NL_LIBS)
TEAM_PATH = ./libteam-1.9
TEAM_MAKE = $(TEAM_PATH)/Makefile
TEAM_APP_PATH = $(TEAM_PATH)/teamd
TEAM_COM_PATH = $(TEAM_PATH)/libteam/.libs
TEAM_OBJS = \
			$(TEAM_COM_PATH)/libteam_la-ifinfo.o \
			$(TEAM_COM_PATH)/libteam_la-libteam.o \
			$(TEAM_COM_PATH)/libteam_la-options.o \
			$(TEAM_COM_PATH)/libteam_la-ports.o \
			$(TEAM_COM_PATH)/libteam_la-stringify.o \
			$(TEAM_APP_PATH)/teamd-teamd_balancer.o \
			$(TEAM_APP_PATH)/teamd-teamd_bpf_chef.o \
			$(TEAM_APP_PATH)/teamd-teamd_common.o \
			$(TEAM_APP_PATH)/teamd-teamd_config.o \
			$(TEAM_APP_PATH)/teamd-teamd_events.o \
			$(TEAM_APP_PATH)/teamd-teamd_hash_func.o \
			$(TEAM_APP_PATH)/teamd-teamd_ifinfo_watch.o \
			$(TEAM_APP_PATH)/teamd-teamd_json.o \
			$(TEAM_APP_PATH)/teamd-teamd_link_watch.o \
			$(TEAM_APP_PATH)/teamd-teamd_option_watch.o \
			$(TEAM_APP_PATH)/teamd-teamd_per_port.o \
			$(TEAM_APP_PATH)/teamd-teamd_phys_port_check.o \
			$(TEAM_APP_PATH)/teamd-teamd_runner_basic_ones.o \
			$(TEAM_APP_PATH)/teamd-teamd_runner_lacp.o \
			$(TEAM_APP_PATH)/teamd-teamd_state.o 
		#	$(TEAM_APP_PATH)/teamd-teamd_runner_loadbalance.o \
		#	$(TEAM_APP_PATH)/teamd-teamd_runner_activebackup.o \
		#	$(TEAM_APP_PATH)/teamd-teamd_ctl.o \
		#	$(TEAM_APP_PATH)/teamd-teamd_workq.o \
		#	$(TEAM_APP_PATH)/teamd-teamd_usock.o \
		#	$(TEAM_APP_PATH)/teamd-teamd_dbus.o \
		#	$(TEAM_APP_PATH)/teamd-teamd_zmq.o \

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

INCLUDES = -I. -I$(NOSINCPATH) -I$(NOSINCPATH)/include -I$(NOSCMDPATH) -I$(NOSEXTPATH) -I$(NOSINCPATH)/riblib -I$(NOSINCPATH)/lcslib
LDPATH   = -L$(NOSLIBPATH) -L/usr/local/lib

CFLAGS   = -g -O2 -Wall -Wno-unused-function -Wpointer-arith -Wshadow -Winline $(DEFINES) $(INCLUDES)
LDFLAGS  = -shared
LIBS     = -lpthread -lnoslib -ldl -lrt
NOSDATA  = -lnosdata -lnoslcs -lnosrib -lcmdlib

TEAMMAIN_SRCS = lacpTeam.c
TEAMMAIN_OBJS = $(TEAMMAIN_SRCS:.c=.o)
COMP_SRCS += $(NOSINCPATH)/nosLib.c
COMP_OBJS = $(COMP_SRCS:.c=.o)
TASK_SRCS = $(NOSINCPATH)/dlu.c ./lacpMain.c $(NOSCMDPATH)/compStatic.c
TASK_OBJS = $(TASK_SRCS:.c=.o)

TARGET     = $(COMP_NAME)d
TARGET_LIB = lib$(COMP_NAME)_$(COMP_VER).so

RESULT := $(shell rm -f ./version; \
	uname -r > ./version; \
	grep "2.6.3" ./version ; \
	if [ -n "$?" ]; then rm -f ./version; fi; \
	exit $?)

.PHONY: all bin-all clean team install
all:
	@if test ! -f ./version; then \
		echo ""; \
		echo "*************************************************************"; \
		echo "*************************************************************"; \
		echo "* LACP must run on N2OS target envirionments (linux-2.6.32) *"; \
		echo "*************************************************************"; \
		echo "*************************************************************"; \
		echo ""; \
	else \
		make bin-all; \
	fi

bin-all: $(EXTHDR) team $(TARGET_LIB) $(TARGET)
	cp $(TARGET) $(TARGET_LIB) $(NOSBINPATH)

$(EXTHDR): $(SHARED_HEADER)
	$(NOSEXTPATH)/exthdr.pl $(SHARED_HEADER)
	$(RM) $(NOSINCPATH)/nosLib.o

$(TARGET_LIB): $(COMP_OBJS) $(TEAMMAIN_OBJS)
	$(CC) ${LDFLAGS} -g -O2 -Wall -o $@ $^ $(TEAM_OBJS) -Wl,-whole-archive $(NOSDATA) -Wl,-no-whole-archive $(LDPATH)

$(COMP_OBJS):%.o:%.c Makefile *.h
	$(CC) -c -fPIC $(CFLAGS) $< -o $@

$(TARGET): $(TASK_OBJS)
	$(CC) -D_REENTRANT -D_GNU_SOURCE -g -O2 -Wall -o $@ $(TASK_OBJS) $(LIBS) $(TEAM_SHARED_LIBS) $(LDPATH)

$(TEAMMAIN_OBJS):%.o:%.c Makefile *.h
	$(CC) -DHAVE_CONFIG_H -I$(TEAM_PATH) -I$(TEAM_PATH)/teamd -I$(TEAM_PATH)/include -I/usr/local/include -D_REENTRANT -D_GNU_SOURCE -g -O2 -Wall -Wno-unused-function -Wpointer-arith -fPIC $(DEFINES) $(INCLUDES) -c $< -o $@

$(TASK_OBJS):%.o:%.c Makefile *.h
	$(CC) -c $(CFLAGS) $< -o $@

install:
	$(CP) $(TARGET) $(NOSBINPATH)
	$(CP) $(TARGET_LIB) $(NOSBINPATH)

clean:
	$(RM) $(TARGET_LIB) $(COMP_OBJS) $(SRC:.c=.d) $(TEAMMAIN_OBJS)
	$(RM) $(TARGET) $(TASK_OBJS)
	$(RM) $(NOSBINPATH)/$(TARGET)
	$(RM) $(NOSBINPATH)/$(TARGET_LIB)
	@if test -f $(TEAM_MAKE); then \
		make -C $(TEAM_PATH) clean; \
	fi
	@cd $(MODULE_PATH); make clean; cd ..

distclean: clean
	@if test -f $(TEAM_MAKE); then \
		make -C $(TEAM_PATH) distclean; \
	fi

team:
	@cd $(MODULE_PATH); make install; cd ..
	@if test ! -f $(TEAM_MAKE) ; then \
		cd $(TEAM_PATH); ./configure --disable-dbus CFLAGS='-DN2OS -DN2OS_KERNEL_2_6_3x -DN2OS_IFLA_ADDRESS -fPIC' PKG_CONFIG_PATH=/usr/local/lib/pkgconfig/; cd ..; \
	fi
	@make -C $(TEAM_PATH)
	@touch lacpTeam.c

lib: lib-all

lib-all:
	make -f Makefile.lib all

lib-install:
	make -f Makefile.lib install

lib-clean:
	make -f Makefile.lib clean

lib-distclean: lib-clean
	make -f Makefile.lib distclean
