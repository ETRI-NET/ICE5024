# Component developer only need to set 
# COMP_NAME, COMP_SRCS and DEFINES 

COMP_NAME = pif_pal
COMP_SRCS = \
			interface.c \
			connected.c \
			palKernelIfTypes.c \
			palKernelProc.c \
			palKernelMii.c \
			palKernelNetlink.c \
			palKernelIoctl.c \
			palKernel.c
DEFINES   = 
NOSPATH   = /root/work-NOS/n2os-0.00.01/

# macro belows should be not modified 
CC  = gcc
MV  = mv
CP  = cp
RM  = rm -f
AR = ar
ARFLAGS = -rv

NOSINCPATH = $(NOSPATH)/src/lib
NOSLIBPATH = $(NOSPATH)/lib
NOSCMDPATH = $(NOSPATH)/src/cmd/common

INCLUDES = -I../ -I. -I./nos -I$(NOSINCPATH) -I$(NOSINCPATH)/include -I$(NOSCMDPATH) 
LDPATH   = -L../../lib -L/usr/local/lib
CFLAGS   = -g -Wall $(DEFINES) $(INCLUDES)
LDFLAGS  = 
LIBS     = -lrt -lpthread -ln2oslib 

COMP_OBJS = $(COMP_SRCS:.c=.o)
TARGET = libpal.a

.PHONY: all clean
all: $(TARGET) 

$(TARGET): $(COMP_OBJS)
	$(AR) $(ARFLAGS) $(TARGET) $(COMP_OBJS) 

$(COMP_OBJS):%.o:%.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	$(RM) $(TARGET) $(COMP_OBJS) $(SRC:.c=.d)

