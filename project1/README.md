# Project1
## Description
Implement Clock, Counter, Text Editor, and Draw Board Mode  
using device control and IPC.  

## Usage
```
$ make
$ ./main
```

## Implementation
This program three processes: Main process, Input Process and Output Process,  
and this uses "message queue" to communicate between processes as 
IPC(Inter Process Communication).  
![Process image](/images/process.png)

### Role of processes
* Input Process: This manages input devices, event(key) device and push-switch device.  
* Main Process: This gets messages from input process and handles & computes the data
of the message. And then, this sends results of a computation to Output process.   
* Output Process: This gets data from Main process and outputs to a device using data.  

### Device Control
This program uses device drivers.

### Modes
* Clock: Clock mode outputs the time when the program begins and can change the time on the FND.  
* Counter: Counter mode outputs the counted number.  
* Text Editor: Text Editor mode ouputs the text obtained from switchs at a LCD device.  
* Draw Board: Draw Board mode outputs the board at a dot device.  
* Setting(Additional implementation): Setting mode manages the additional
    funtions of program modes.  
    1. If LED(1) sets, the time of Clock mode goes
    2. If LED(2) sets, 4th digit of base 10 is printed out on FND in Counter mode.
