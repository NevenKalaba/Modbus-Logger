#include <stdio.h>
#include <stdlib.h>


#include <unistd.h>
#include <stdint.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>
#include <modbus.h>
#include <netinet/in.h>


#define BCM_PIN_DE 22


modbus_t *ctx;
int socketfd;


//signal handler function
void handle_sigint(int sigint)
{	

	
	fprintf(stdout,"Program terminated!\n");
	exit(0);
fflush(stdout);

}
//function for closing sockets and freeing the modbus context 
void free_rec()
{
	modbus_rpi_pin_unexport_direction(ctx);
	modbus_close(ctx);
	modbus_free(ctx);
  	 close(socketfd);
	
	
}
int main (int argc, const char* argv[])
{
	
	uint8_t req[MODBUS_RTU_MAX_ADU_LENGTH]={0};
	char buffer[MODBUS_RTU_MAX_ADU_LENGTH]={0};
	struct sockaddr_in servaddr;
	char* send_address;
	
	//postavljanje handlera za terminirajuce signale
	//setting the signal handler
	signal(SIGINT,handle_sigint);
	signal(SIGTERM,handle_sigint);
	atexit(free_rec);
	
	

	//taking an ipv4 address as the command line argument otherwise setting it to be the broadcast adress
	if(argc>1){
	send_address=(char*)argv[1];
	}
	else{
	send_address="255.255.255.255";
	}
	printf("Destination: %s\n",send_address);
	
 	


	//Opening the socket for UDP comunication
	if((socketfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

	//Enabling broadcast comunication 
	int broadcastEnable=1;
	int ret=setsockopt(socketfd, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable));


	//setting the port number and the destination address 
	memset(&servaddr, 0, sizeof(servaddr));

	servaddr.sin_family    = AF_INET; // IPv4
    	servaddr.sin_addr.s_addr = inet_addr(send_address);
    	servaddr.sin_port = htons(1234);





	//Opening of the serial port for modbus rtu comunication
	ctx = modbus_new_rtu("/dev/ttyAMA0", 9600, 'N', 8, 1);
	
	if (ctx == NULL)
	{
	fprintf(stderr, "Unable to create the libmodbus context!\n");
	return -1;
	}
	
	
	
	
	if (modbus_connect(ctx) == -1)
	{

	fprintf(stderr, "Connection failed.\n");
	modbus_free(ctx);
	return -1;

	}

	
	//Functions of the Raspberry Pi fork of libmodbus
	modbus_enable_rpi(ctx, TRUE);
	modbus_configure_rpi_bcm_pin(ctx, BCM_PIN_DE);
	modbus_rpi_pin_export_direction(ctx);


	while(1)
	{
		//receiving an indication request 
		if(modbus_receive(ctx,req)<0)
			fprintf(stderr,"Error while recieving frame");
		
		//copying the request into the send buffer
		for(int i=0;i<MODBUS_RTU_MAX_ADU_LENGTH;i++)
			buffer[i]=req[i];
			


	//sending the UDP packet to the set ipv4 address		
 	sendto(socketfd,buffer,MODBUS_RTU_MAX_ADU_LENGTH ,
        0, (const struct sockaddr *) &servaddr, 
            sizeof(servaddr));
		
	}
	


}
