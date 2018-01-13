obj-m := 05_input_event_keyboard.o
KDIR  := /lib/modules/$(shell uname -r)/build

all:
	make -C $(KDIR) M=$$PWD
	
clean:
	rm *.mod.c *.o modules.order *.mod.o Module.symvers *.ko
