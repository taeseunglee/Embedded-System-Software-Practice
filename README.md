# Embedded-System-Software-Practice
## Description
This repository for the Emebedded System Software Course(CSE4116) in Sogang Univ.

## Environment
Cross-Platfrom Development  
Host PC: Linux, Ubuntu 14.04 on x64 architecture  
Target Board: Embedded Linux System on ARM architecture  


## Project1
### Description
Implement Clock, Counter, Text Editor, and Draw Board Mode 
using device control and IPC.

## Project2
### Description
Implement programs using system call programming, module programming and device
driver implementation.
1. Implement a module(module/kernel_timer.c) including a device driver(fpga_fnd, fpga_led,
   fpga_dot, fpga_text_lcd) and timer module.
2. Implement a system call(syscall/pack_timer.c) that takes parameters and returns them as a single
   variable.
3. Implement an application(app/app.c) that performs simple output using the implemented
   device driver and system call.

## Project3
### Description
Implement a stopwatch program using module programming and device driver and
interrupt implementation.
1. Implement a stopwatch module(module/stopwatch.c) including a device driver
   (gpio_fnd), timer module and interrupt.  
2. Implement an application(app/app.c) that uses the implemented module.  

### Usage
- Home: start main timer
- Back: pause main timer
- Vol+: reset main timer
- Vol-: Start end timer. If press vol- 3 seconds continuously, exit the executing application 

## Author
 * Taeseung Lee

## Final Project for Embeded system software with Taeguk Kwon

### Description
Modify android native framework for project.
Modified Android Framework : Surface Flinger, Graphics part of HAL
Android Version : Kitkat (API 19)

### Author
* Taeseung Lee (me)
* Taeguk Kwon (https://www.github.com/taeguk)
