BASE_PATH   := /lib/modules/$(shell uname -r)/build

NOSPATH   = ../../..
NOSBINPATH = $(NOSPATH)/bin

#base: 1.3.4
obj-m += nteam.o
#base: 1.3.5
obj-m += nteam_mode_loadbalance.o

nteam-objs += team.o
nteam_mode_loadbalance-objs += team_mode_loadbalance.o

FLAGS=-DN2OS #-I./include

all:
	echo $(BASE_PATH)
	make -C $(BASE_PATH) EXTRA_CFLAGS=$(FLAGS) M=$(PWD) modules

clean:
	make -C $(BASE_PATH) M=$(PWD) clean

install: all
	cp -f nteam.ko $(NOSBINPATH)
	cp -f nteam_mode_loadbalance.ko $(NOSBINPATH)
