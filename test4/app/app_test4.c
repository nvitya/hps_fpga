#include <stdio.h>
#include <stdlib.h>
#include "stdint.h"
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/mman.h>
//#include "hps_0.h"
#include <stdbool.h>

/* ALTERA SOC Memory Map (Chapter 2-18 of Cyclone V HPS Reference Manual)

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
*/

#define HW_REGS_BASE          0xFF200000  // LW slaves start address
#define HW_REGS_SPAN          0x00200000  // LW slaves have 2M address space
#define HW_REGS_MASK ( HW_REGS_SPAN - 1 )

#define LED_PIO_BASE 0

volatile unsigned long *  h2p_lw_led_addr = NULL;

int main(int argc, char **argv)
{
	void *virtual_base;
	int fd;
	uint8_t ledcnt = 0;
	
	// map the address space for the LED registers into user space so we can interact with them.
	// we'll actually map in the entire CSR span of the HPS since we want to access various registers within that span
	if( ( fd = open( "/dev/mem", ( O_RDWR | O_SYNC ) ) ) == -1 ) 
	{
		printf( "ERROR: could not open \"/dev/mem\"...\n" );
		return( 1 );
	}
	virtual_base = mmap( NULL, HW_REGS_SPAN, ( PROT_READ | PROT_WRITE ), MAP_SHARED, fd, HW_REGS_BASE );	
	if( virtual_base == MAP_FAILED ) 
	{
		printf( "ERROR: mmap() failed...\n" );
		close( fd );
		return(1);
	}
	
	h2p_lw_led_addr = virtual_base + ( ( unsigned long  )( LED_PIO_BASE ) & ( unsigned long)( HW_REGS_MASK ) );
	
	printf("Displaying counter on the LEDS...\n");
	
	while(1)
	{
	  ++ledcnt;
	  
	  *h2p_lw_led_addr = ledcnt;
	  
		usleep(100*1000);
	}
	
	if( munmap( virtual_base, HW_REGS_SPAN ) != 0 ) 
	{
		printf( "ERROR: munmap() failed...\n" );
		close( fd );
		return( 1 );

	}
	close( fd );
	return 0;
}
