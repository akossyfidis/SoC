obj-m:=hello.o
#KERNEL_SOURCE=/lib/modules/$(shell uname -r)/build
KERNEL_SOURCE=/home/ensea/linux-socfpga
#CROSS_COMPILE=/usr/bin/arm-linux-gnueabihf-
#ARCH=arm
CFLAGS_MODULE=-fno-pic

all :
	make -C $(KERNEL_SOURCE) M=$(PWD) modules
clean :
	make -C $(KERNEL_SOURCE) M=$(PWD) clean
install :
	make -C $(KERNEL_SOURCE) M=$(PWD) modules install
