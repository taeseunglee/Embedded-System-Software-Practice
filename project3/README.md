# Project3
## Description
Implement a stopwatch program using module programming and device driver and
interrupt implementation.
1. Implement a stopwatch module(module/stopwatch.c) including a device driver
   (gpio_fnd), timer module and interrupt.  
2. Implement an application(app/app.c) that uses the implemented module.  

## How to Set
0-1) Modify gpio buttons
In archroimx_kernel/arch/arm/mach-mx6/board-achroimx.c, modify codoes like below
c code
``` c
static struct gpio_keys_button ard_buttons[] = {
//	GPIO_BUTTON(SABREAUTO_ANDROID_HOME,    KEY_HOME,       1, "home",        0),
//	GPIO_BUTTON(SABREAUTO_ANDROID_BACK,    KEY_BACK,       1, "back",        0),
//	GPIO_BUTTON(SABREAUTO_ANDROID_VOLUP,   KEY_VOLUMEUP,   1, "volume-up",   0),
//	GPIO_BUTTON(SABREAUTO_ANDROID_VOLDOWN, KEY_VOLUMEDOWN, 1, "volume-down", 0),
//	GPIO_BUTTON(SABREAUTO_ANDROID_POWER,   KEY_POWER,      1, "power-key",   1),
};
```

0-2) Compile kernel and build bootimage
``` shell
$ make clean
$ make achroimx_defconfig
$ make -j4
$ ./make_bootimage.sh # in android/ directory
```

0-3) Change Kernel
``` shell
# set the target board(device) into u-boot mode
$ fastboot # device
$ fastboot erase boot # Host
$ fastboot flash boot boot.img # Host
$ fastboot reboot # Host
```

1) Insert the stopwatch module
``` shell
$ make -C ./module/ # Host
$ adb push kernel_timer.ko /sdcard/ # Host
$ insmod stopwatch.ko # Target Board. But it need to copy from /sdcard/ to
.(working directory)
$ mknod /dev/stopwatch c 245 0 # Target Board.
```

2) Make an execution file of app.c and Send app to the target board.
``` shell
$ make -C ./app/ # Host
$ adb push app /sdcard/ # Host
$ ./app # Target Board. This is an example, but it need to copy from
/sdcard/ to .
```

3) Remove the kernel timer module
```
$ rmmod stopwatch.ko
```

## Usage
- Home: start main timer
- Back: pause main timer
- Vol+: reset main timer
- Vol-: Start end timer. If press vol- 3 seconds continuously, exit the executing application 
