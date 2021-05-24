# hps_fpga

I just want to write down (and share) my experiences with the DE0-Nano-SoC (or Atlas-SoC) board.

I wanted to understand how it works, and I wanted to have a useable development environment with tolerable compile/synthesis times.

## The Board
The actual board I experiemented with is a DE0-Nano-SoC but also called as Atlas-SoC. This has 
  - 2x ARM Cortex A9 Cores at 925 MHz, also referenced as HPS (Hard Processor System)
  - A Cyclone V FPGA with 40k LUT

## Pure FPGA Development
The board can be used also for pure FPGA development, just like anoter board without integrated Hard Processors. For the Altera/Intel development you need to download the Intel Quartus Lite with the Cyclone V package. The board has an integrated USB Blaster, so you don't need any additional HW for the FPGA development.

### Design Template
There is a design template for the board here https://fpgacloud.intel.com/devstore/platform/17.0.0/Standard/atlas-soc-de0-nano-soc-baseline-pinout/
At least it defines the IO PIN connections, but **Unfortunately the clocks in the TOP template (de0_nano_soc_baseline.v) have wrong names so you have to correct them from FPGA_CLKx_5 to CLOCKx_50.**

## The Linux...
The Terasic provides a Linux image with and Angström (?) Linux on it. The board has an intergrated FTDI USB-UART where you see the Linux console messages and you can log-in, check the board actual IP address, to log in via SSH for smoother control.

**Not bad, but I prefer Debian.**

I work with mc (Midnight Commander). If you learn it you can be much faster than using just the command line.
And also for installing other tools I like better to use Debian/Ubuntu whatever... actually an easily extensible Linux environment without having enourmous compile times.

Luckily Mr. Ichiro Kawazome (https://github.com/ikwzm) published many of his works for FPGA-ARM SoC boards (including Zynq from Xilinx). And he made a howto for installing a Debian Linux with kernel version 5.4: https://github.com/ikwzm/FPGA-SoC-Linux

It is not the easiest thing to make, but I could successfully install Debian onto the SDCARD.

With the debian you can install the compilers you want, the tools you like. The armhf architecture is very well supported.

So I could develop Linux application for the ARM Linux. That's fine, but I have much better and cheaper boards for that, like the Raspberry PI 4.

# Developing FPGA + ARM Applications

This is much-much more challenging than I thought...
And unfortunately I began with not the right examples and tutorials.

So you want to communicate with your own FPGA code using your own ARM code on the HPS.
Usually there is some address range reserved in the 4G space where you can map your own FPGA registers. This is how it works here too.

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
The compiled device tree is to be found in thee boot partition at /mnt/boot. Fortunately there are the source files too (devicetree-5.4.105-socfpga.dts).
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
Reboot, and the LW bridge is active.
You can check with "dmesg".

## AXI Bus
The AXI bus is the standard internal hi-speed bus intern in the ARM processors, and this is used also for all the three HPS-FPGA bridges.
It seems to me so, that even the lightweight bus at 0xFF20 0000 uses the "normal" AXI bus specification instead of the AXI Lite.

An AXI bus consists of 5 unidirectional channels:
  - --> Read Address 
  - <-- Read Data 
  - --> Write Address
  - --> Write Data
  - <-- Write Result

All of these channels are handshaked with their VALID and READY signals. Data can be transferred in bursts, also with auto increment addresses.

So interfacing something to the AXI bus is much more complicated than a traditional ADDRESS, DATA, CSEL, RDEN, WREN bus.

## Aiding tool: Platform Designer aka. QSYS

In the Quartus at the **Tools** menu can be found the **Platform Designer**. With this tool you can add IP cores visually bind them and set their properties.
Then you can generate the VHDL code from it, which has to be then added to the project.

The minimal requiremenst for controlling the FPGA LEDS are:
  - Clock Source (automatically added for new projects)
  - Arria/Cyclone V Hard Processor System
  - PIO (Parallel IO) Intel FPGA IP

You have to click the connections, define the exports. There are some tutorials for that. Better take a working project and check their settings. I took the HPS_CONTROL_FPGA_LED from the official board examples. This project has more than the LEDS control. It took a while until I managed to have a minimal set only for the LEDs control.

The Intel/Altera suggests to add the generated QIP file to the project.

## Compile/Synthesis time of 6 minutes
I prefer the Altera/Intel tools (Quartus) over Xilinx because the synthesis runs much faster. Basically the Quartus runs twice as fast as the Xilinx ISE/Vivado.
So I did not understand why such a small project takes 6 minutes to compile. There is a step at the beginning called "Analysis & Synthesis" which stops around 93% and stays there for 3 minutes... (The classical progress bar :))

Ah, yeah I have fast AMD Ryzen 3700X (8 x 3.6GHz) PC, so that should not be a problem.

I definitely don't want to wait every time for 3-4 minutes to find out that I made a typing mistake at a signal name.

So this is a no-go. Sadly.

## Compile/Synthesis without QSYS
I made a test project "test4" (included in this repository). Took all the QSYS (Platform Designer) generated files and the necessary IP cores, and added only those to the project without the QIP file.
I had to delete the SDRAM signals from some places. It took a while until I identified all the necessary components (like altera_merlin_burst_adapter_13_1.sv).
But the end it worked.

**Compile Time: 0:35 (35 seconds)**

Thats ok.

## Loading the FPGA Bitstream
Later the bitstream should be used via Device Tree overlays, which also defines the coupled kernel module (driver).
For the developments it totally fine loading the bitstream manually using the Quartus/Tools/Programmer.
The on board USB Blaster must be connected. For Linux dev. env. some one-time UDEV rules setup also required: https://rocketboards.org/foswiki/Documentation/UsingUSBBlasterUnderLinux.

The Progammer Setup is a bit tricky here, because there is a HPS also in the JTAG chain. So first you have to press "Auto Detect", then add the .sof file to the FPGA part and delete the other FPGA part. And then the programming goes, the orange led beside the blue one signalises that the FPGA is configured.

## Test App
You can find the source codes of a test application at test4/app. There is a Makefile, so from linux, when the cross compiler arm-linux-gnueabihf-gcc installed, just compiles with "make".
Copy the "app_test4" to the board with scp/sftp. Set the execution right bit with "chmod +x app_test4" and start with ./app_test4.

Then hopefully the LEDs are displaying binary counting.

In case of "bus error" please check the part "Activating the LW Bridge" earlier.
