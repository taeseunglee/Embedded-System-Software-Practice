# Project2
## Description
Implement programs using system call programming, module programming and device
driver implementation.
1. Implement a module(module/kernel_timer.c) including a device driver(fpga_fnd, fpga_led,
   fpga_dot, fpga_text_lcd) and timer module.  
2. Implement a system call(syscall/pack_timer.c) that takes parameters and returns them as a single
   variable.  
3. Implement an application(app/app.c) that performs simple output using the implemented
   device driver and system call.  

## Usage
1) Insert a kernel timer module
```
$ ./make -C ./module/ # Host
$ adb push kernel_timer.ko /sdcard/ # Host
$ insmod kernel_timer.ko # Target Board. But it need to copy from /sdcard/ to .
$ mknod /dev/dev_driver c 242 0 # Target Board.
```

2) Add System call like kernel/ directory structure
3) Make an execution file of app.c and Send app to a target board.
```
$ make -C ./app/ # Host
$ adb push app /sdcard/ # Host
$ ./app 10 10 1000 # Example. Target Board. But it need to copy from /sdcard/ to .
```

4) Remove the kernel timer module
```
$ rmmod kernel_timer.ko
```
