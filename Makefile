obj-m += pamir-ai-soundcard.o pamir-ai-i2c-sound.o pamir-ai-rpi-soundcard.o

pamir-ai-soundcard-objs := pamir-ai-soundcard-main.o
pamir-ai-i2c-sound-objs := pamir-ai-i2c-sound-main.o
pamir-ai-rpi-soundcard-objs := pamir-ai-rpi-soundcard-main.o

KERNEL_DIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) clean

install:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) modules_install

.PHONY: all clean install