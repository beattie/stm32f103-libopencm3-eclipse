# stm32f103-libopencm3-eclipse
Resurecting and old project that was not working, updated and tested with Eclipse 2022-12.

A bare bones libopencm3 eclipse project for the bluepill stm32f103c8 from maple mini source. Should work for any stm32f103

Install libopencm3 in /opt:

  $ git clone https://github.com/libopencm3/libopencm3.git /opt/libopencm3
  
  $ cd /opt/libopencm3
  
  $ make

---
## Setup Debugging with Blackmagic Probe
I have a couple of Black Pill boards flashed with the Black Magic Probe software that I use for debugging. The following steps should not be needed.
  
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
