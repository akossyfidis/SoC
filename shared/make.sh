make clean
make CROSS_COMPILE=/usr/bin/arm-linux-gnueabihf- ARCH=arm
arm-linux-gnueabihf-gcc led_control.c -o led
