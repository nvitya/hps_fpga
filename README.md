# hps_fpga

I Just want to write down (and share) my experiences with the DE0-Nano-SoC (or Atlas-SoC) board.

I wanted to understand how it works, and I wanted to have a useable development environment with tolerable compile/synthesis times.j

## The Board
The actual board I experienced with is a DE0-Nano-SoC but also called as Atlas-SoC. This has 
  - 2x ARM Cortex A9 Cores at 925 MHz, also referenced as HPS (Hard Processor System)
  - A Cyclone V FPGA with 40k LUT

## Pure FPGA Development
The board can be used also for pure FPGA development, just like anoter board without integrated Hard Processors. For the Altera/Intel development you need to download the Intel Quartus Lite with the Cyclone V package. The board has an integrated USB Blaster, so the FPGA development is easy.

## The Linux...
The Terasic provides a Linux image with and Angström (?) Linux on it. The board has an intergrated FTDI USB-UART where you see the Linux console messages and you can log-in, check the board actual IP address, to log in via SSH for smoother control.
### Not bad, but I prefer Debian.
I work with mc (Midnight Commander). If you learn it you can be much faster than using just the command line.
And for other tools I like better to work on Debian/Ubuntu whatever... so easily extensible Linux without having enourmous compile times

Luckily Mr. Ichiro Kawazome (https://github.com/ikwzm) published many of his works for FPGA-ARM SoC boards (including Zynq from Xilinx). And he made a howto for installing a Debian Linux with kernel version 5.4: https://github.com/ikwzm/FPGA-SoC-Linux

It is not the easiest thing to make, but I could successfully install Debian onto the SDCARD.

With the debian you can install the compilers you want, the tools you like. The armhf architecture is very well supported.

So I could develop Linux application for the ARM Linux. That's fine, but I have much better and cheaper boards for that, like the Raspberry PI 4.

# Developing FPGA + ARM Applications

This is much-much more challenging than I thought...
And unfortunately I began with not the right examples and tutorials.

So you want to communicate with your own FPGA code using your own ARM code on the HPS.
There should be some address range reserved in the 4G space where you can map your own FPGA registers. Yes this is how it works.

## ALTERA SOC Memory Map
This can be found at the Chapter 2-18 of Cyclone V HPS Reference Manual:
```
0000_0000: BOOT / SDRAM (3 GB)
C000_0000: FPGA Slaves (960 MB)
FC00_0000: Peripherals (64 MB)
  FF20_0000: LW Slaves (2M)
  FF40_0000: LW HPS->FPGA Bridge Registers
  FF50_0000: HPS->FPGA Bridge Registers
  FF60_0000: FPGA->HPS Bridge Registers
  FF70_0000: HPS Integrated peripherals
    FF70_0000: Ethernet MAC0 registers
    ...
``` 
So you have three bridges and two memory mapped areas to communicate with your own FPGA.
The FPGA->HPS bridge is rather for implementing a DMA in the FPGA.
There is a light-weight HPS->FPGA bridge and a "full/heavy" one.
The LW has only 2MByte memory range at 0xFF200000. The other one 960 MByte at 0xC0000000.
I've played only with the LW so far.

## Activating the LW Bridge
You don't need to bother this if you are using the original Linux image from terasic.
Unfortunately the LW bridge was deactivated in the Linux from Mr. Kawazome (https://github.com/ikwzm/FPGA-SoC-Linux).
It took a while until I figured it out. And then additionally a litte to solve it.
The compiled device tree must be placed to the boot partition at /mnt/boot fortunately there are the source files too (devicetree-5.4.105-socfpga.dts).
Just search for "fpga_bridge@ff400000" and set the bridge-enable to 0x01.
```
    fpga_bridge@ff400000 {
            compatible = "altr,socfpga-lwhps2fpga-bridge";
            reg = <0xff400000 0x100000>;
            resets = <0x06 0x61>;
            clocks = <0x05>;
            bridge-enable = <0x01>;
            phandle = <0x25>;
    };
```                
Compile with
```
# dtc -O dtb -o devicetree-5.4.105-socfpga.dtb devicetree-5.4.105-socfpga.dts
```
And reboot and LW bridge is active.
You can check with "dmesg".

## AXI Bus
