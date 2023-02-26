# stm32f103-libopencm3-eclipse
Resurecting and old project that was not working. Projects target bluepill boards. Updated and tested with Eclipse 2022-12.

A bare bones libopencm3 eclipse projects for the bluepill stm32f103c8 from maple mini sources. Should work for any stm32f103 with some modification.

Install libopencm3 in /opt:

  $ git clone https://github.com/libopencm3/libopencm3.git /opt/libopencm3
  
  $ cd /opt/libopencm3
  
  $ make

## Projects
* miniblink - blink an LED using a delat loop.
* systick - Using the systick timer blink an LED 1Hz
* usb-cdc - Instansiate a USB CDC device

---
## Setup Debugging with Blackmagic Probe
I have a couple of Black Pill boards flashed with the Black Magic Probe software that I use for debugging.
  
Build project (so the executable will be found)  
Run -> Debug Configurations -> GDB Hardware Debugging (double click) -> Debugger (arm-none-eabi-gdb)  (uncheck _Use remote target_) -> Startup -> After Halt checkbox

```
set mem inaccessible-by-default off
target extended-remote /dev/ttyBmpGdb
monitor swdp_scan
attach 1
```

After Set breakpoint at

```
   start
```

Apply

### Command line flash with BMP
```
arm-none-eabi-gdb <file>.elf
target extended_remote /dev/ttyACM0
monitor swdp_scan
attach 1
load
```
