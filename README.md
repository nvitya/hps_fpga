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

