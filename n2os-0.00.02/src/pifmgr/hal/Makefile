# Component developer only need to set 
# COMP_NAME, COMP_SRCS and DEFINES 

COMP_NAME = pif_hal
COMP_SRCS = \
			./l2swfwdr/hal.c \
			./l2swfwdr/hal_bridge.c \
			./l2swfwdr/hal_garp.c \
			./l2swfwdr/hal_if_generic.c \
			./l2swfwdr/hal_l2_fdb.c \
			./l2swfwdr/hal_lacp.c 

DEFINES   = 
NOSPATH   = /root/work-NOS/n2os-0.00.01

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

INCLUDES = -I. -I./l2swfwdr -I./L2 -I./L3 -I./nos -I$(NOSINCPATH) -I$(NOSINCPATH)/include -I$(NOSCMDPATH) 
LDPATH   = -L../../lib -L/usr/local/lib
CFLAGS   = -g -Wall $(DEFINES) $(INCLUDES)
LDFLAGS  = 
LIBS     = -lrt -lpthread -ln2oslib 

COMP_OBJS = $(COMP_SRCS:.c=.o)
TARGET = libhal.a

.PHONY: all clean
all: $(TARGET) 

$(TARGET): $(COMP_OBJS)
	$(AR) $(ARFLAGS) $(TARGET) $(COMP_OBJS) 

$(COMP_OBJS):%.o:%.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	$(RM) $(TARGET) $(COMP_OBJS) $(SRC:.c=.d)

