#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdbool.h>
#include "hwlib.h"
#include "soc_cv_av/socal/socal.h"
#include "soc_cv_av/socal/hps.h"
#include "soc_cv_av/socal/alt_gpio.h"

#define HW_REGS_BASE ( ALT_STM_OFST )
#define HW_REGS_SPAN ( 0x04000000 )
#define HW_REGS_MASK ( HW_REGS_SPAN - 1 )

#define USER_IO_DIR     (0x01000000)
#define BIT_LED         (0x01000000)
#define BUTTON_MASK     (0x02000000)

#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wimplicit-function-declaration"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wformat="

int main(int argc, char **argv) {
	void *virtual_base;
	int fd;
	char *data;
	char ledstatus[5]="";

	// map the address space for the LED registers into user space so we can interact with them.
	// we'll actually map in the entire CSR span of the HPS since we want to access various registers within that span
	if( ( fd = open( "/dev/mem", ( O_RDWR | O_SYNC ) ) ) == -1 ) {
		printf( "ERROR: could not open \"/dev/mem\"...\n" );
		return( 1 );
	}

	virtual_base = mmap( NULL, HW_REGS_SPAN, ( PROT_READ | PROT_WRITE ), MAP_SHARED, fd, HW_REGS_BASE );
	
	if( virtual_base == MAP_FAILED ) {
		printf( "ERROR: mmap() failed...\n" );
		close( fd );
		return( 1 );
	}
	// initialize the pio controller
	// led: set the direction of the HPS GPIO1 bits attached to LEDs to output
	alt_setbits_word( ( virtual_base + ( ( uint32_t )( ALT_GPIO1_SWPORTA_DDR_ADDR ) & ( uint32_t )( HW_REGS_MASK ) ) ), USER_IO_DIR );

	printf("%s%c%c\n","Content-Type:text/html;charset=iso-8859-1",13,10);
	printf("<html>");
	printf("<header>");
	printf("<TITLE>CGI</TITLE>\n");
	printf("</header>");

	printf("<body>");
	printf("<H1>Hello, World!</H1>\n");
	printf("<h2>About this Server</h2>\n");
	printf("Server Name: %s <BR>\n",getenv("SERVER_NAME"));
	printf("Server Name: CGI test with DE0-Nano-SoC <BR>\n");
	printf("Running on Port: %s <BR>\n",getenv("SERVER_PORT"));
	printf("Server Software: %s <BR>\n",getenv("SERVER_SOFTWARE"));
	printf("Server Protocol: %s <BR>\n",getenv("SERVER_PROTOCOL"));
	printf("CGI Revision: %s <BR>\n",getenv("GATEWAY_INTERFACE"));
	
	printf("<H2>Test CGI</H2>\n");
	printf("<form action='/cgi-bin/hps.cgi' method='GET'><br>"); // add target='_blank' >> open new tab
	printf("<input type='submit' name='led' value='on'>");
	printf("<input type='submit' name='led' value='off'><br>");
	printf("</form>");

	data = getenv("QUERY_STRING");
	if(data == NULL)
  	printf("<h3> LED status is on</h3>");
	else {	
		sscanf(data,"led=%s",&ledstatus);
		if(strcmp(ledstatus,"on")) {
			alt_clrbits_word( ( virtual_base + ( ( uint32_t )( ALT_GPIO1_SWPORTA_DR_ADDR ) & ( uint32_t )( HW_REGS_MASK ) ) ), BIT_LED );
		}
		else {
			alt_setbits_word( ( virtual_base + ( ( uint32_t )( ALT_GPIO1_SWPORTA_DR_ADDR ) & ( uint32_t )( HW_REGS_MASK ) ) ), BIT_LED );
		}
		printf("<h3> LED status is %s</h3>",ledstatus);
	}
	printf("</body>");
	printf("</html>");

	return 0;
}

