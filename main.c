//
//  main.c
//  LeSerialPortServer
//
//  Created by uwe cerron on 11/22/12.
//  BSD licence
//

#include <math.h>
#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/unistd.h>
#include <pthread.h>
#include <getopt.h>
#include <termios.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <ctype.h>
#include <unistd.h>
#include <time.h>



#define RTPIS_SERVER_PORT        80
#define RTPIS_SERVER_NAME        "mainpage.us"
#define DIRSIZE     8192
#define HOSTNAME_SIZE     4096
#define SERIAL_READ_BUFFER_SIZE	1024 




/* ---- Private Constants and Types -------------------------------------- */

struct sockaddr_in   saddr;   /* build our address here */
struct hostent	*hp;   /* this is part of our    */


struct{ speed_t speed; 	unsigned baudRate;} gBaudTable[] = {
    { B50,          50 },{ B75,          75 },{ B110,        110 },{ B134,        134 },{ B150,        150 },{ B200,        200 },
    { B300,        300 },{ B600,        600 },{ B1200,      1200 },{ B1800,      1800 },{ B2400,      2400 },{ B4800,      4800 },
    { B9600,      9600 },{ B19200,    19200 },{ B38400,    38400 },{ B57600,    57600 },{ B115200,  115200 },{ B230400,  230400 } };


/* ---- Public Variables ------------------------------------------------- */

int gDebug = 0;

int runFlag = 1;

int gPortFd_0 = -1;	// ttyUSB0 thread

int i,j,k; 

//you can get the info below with ls /dev on unix based systems
char *devName_0 = "/dev/cu.usbmodem641";

char gHostname[HOSTNAME_SIZE];         /* address 	         */


/* ---- Private Function Prototypes -------------------------------------- */


void *ReaderThread_0( void *param );


void  Usage( void );

void initialization( void );

int requestGrant(char*);


/* ---- Private Variables ------------------------------------------------ */


/* ---- Functions -------------------------------------------------------- */


/***************************************************************************
 *
 *  main
 *
 ****************************************************************************/

int main( int argc, char **argv ){
	printf( "start Version - 0.04\n");
    
	int	rc_0;
    
	int	opt_0;   
	speed_t     baudRate_0 = B9600;
	pthread_t   readerThreadId_0;
	
	if (argc == 2){
		fprintf(stdout,"port = %s\n",argv[1]);
		devName_0 =argv[1];
	}else if (argc == 3){
		fprintf(stdout,"port = %s\n",argv[1]);
		devName_0 =argv[1];
		fprintf(stdout,"server name = %s\n",argv[2]);
		devName_0 =argv[1];		
	}else if (argc > 4){
		Usage();
	}else {
		fprintf(stdout,"no port is provided, use a default port (%s)\n\n no server name is provided, use a default server name (%s)\n\n", devName_0, RTPIS_SERVER_NAME);
		Usage();
	}
    
	struct termios attr_0;
    
	initialization();
    
    
	if (( gPortFd_0 = open( devName_0, O_RDWR | O_EXCL )) < 0 ) {
        fprintf( stderr, "0-0) Unable to open serial port '%s': %s\n", devName_0, strerror( errno ));
        //exit( 2 );
	}
    
	if ( tcgetattr( gPortFd_0, &attr_0) < 0 ){
		fprintf( stderr, "1-0) Call to tcgetattr failed: %s\n", strerror( errno ));
		//exit( 3 );
	}
	attr_0.c_iflag = 0;
	attr_0.c_oflag = 0;
	attr_0.c_cflag = CLOCAL | CREAD | CS8;
	attr_0.c_lflag = 0;
	attr_0.c_cc[ VTIME ] = 0; // timeout in tenths of a second
	attr_0.c_cc[ VMIN ] = 1;  // Only wait for a single char
    
	cfsetispeed( &attr_0, baudRate_0 );
	cfsetospeed( &attr_0, baudRate_0 );
    
	if ( tcsetattr( gPortFd_0, TCSAFLUSH, &attr_0 ) < 0 ) {
		fprintf( stderr, "2-0) Call to tcsetattr failed: %s\n", strerror( errno ));
		//exit( 4 );
	}
    
	// Put stdin & stdout in unbuffered mode.
    
	setbuf( stdin, NULL );
	setbuf( stdout, NULL );
    
	// Put stdin in raw mode (i.e. turn off canonical mode). Canonical mode
	// causes the driver to wait for the RETURN character so that line editing
	// can take place. We also want to turn off ECHO.
    
	{
		struct termios tio_0;
		if ( tcgetattr( fileno( stdin ), &tio_0 ) < 0 ) {
			fprintf( stderr, "0) Unable to retrieve terminal settings: %s\n", strerror( errno ));
			//exit( 5 );
		}
        
		tio_0.c_lflag &= ~( ICANON | ECHO );
		tio_0.c_cc[VTIME] = 0;
		tio_0.c_cc[VMIN] = 1;
        
		if ( tcsetattr( fileno( stdin ), TCSANOW, &tio_0 ) < 0 ){
			fprintf( stderr, "0) Unable to update terminal settings: %s\n", strerror( errno ));
            //			exit( 6 );
		}
	}
    
    
    // Kick off the serial port reader thread.
    
	printf( "starting thread_0\n");
	rc_0 = pthread_create( &readerThreadId_0, NULL, ReaderThread_0, NULL );
	if ( rc_0 != 0 ) {
		fprintf( stderr, "Error creating ReaderThread_0: %s\n", strerror( rc_0 ));
		exit( 7 );
	}else{
		printf( "completed to start thread_0\n");
	}
    
    
    
    
	while ( runFlag ){
		char    ch;
		int     chInt = fgetc( stdin );
		if ( chInt < 0 ){
			fprintf( stderr, "Exiting...\n" );
			break;
		}
		ch = (char)chInt;
        
		if ( gDebug ){
			if (( ch < ' ' ) || ( ch > '~' )) {
				fprintf( stderr, "stdin Read: 0x%02x '.'\n", ch );
			}else {
				fprintf( stderr, "stdin Read: 0x%02x '%c'\n", ch, ch );
			}
		}	
        
		if ( ch == 'a'){
			fprintf( stderr, "test\n");
		}else if ( ch == '1'){
		}else {
		}
		usleep(10);       
	}
    
	close( gPortFd_0);
	exit( 0 );
	return 0;   // Get rid of warning about not returning anything
}




/***************************************************************************/
/**
 *   Thread which processes the incoming serial data from PORT 0.
 */







void *ReaderThread_0( void *param ){
	int dataSize = 0;
	long readFailS,readFailN,readFailP, readFail0, readSuccess = 0;
    
	int readFail_0=0;
    
	printf( "in Serial thread_0\n");  
	char readBuffer[SERIAL_READ_BUFFER_SIZE];
	char sendBuffer[SERIAL_READ_BUFFER_SIZE];
	char send_text[1024];
	int readBufferPointer = 0;
	send_text[0]='\0';
	readBuffer[SERIAL_READ_BUFFER_SIZE -1] = '\0'; 
	readBuffer[0] = '\0';
	
	int     bytesRead;
	char ch;
	int i,j;
    
	while ( 1 ){
        
		if (( bytesRead  = read( gPortFd_0, &ch, 1 )) < 0 ){
			fprintf( stderr, "Serial port read failed:\n");
			usleep(1000000);
            //			exit( 1 );
		}else if(bytesRead == 0){ 
			readFail_0++ ;
			fprintf( stderr, "err: %d\n",readFail_0 );
		}else{
            
            //			fprintf( stderr, "%s", readBuffer);
           	//	fprintf( stderr, "%d-", ch);
            
			if(ch == '{'){			
				readBufferPointer = 1;
				readBuffer[0] = '{';
			}else if (ch == '}'){
                //				readBuffer[readBufferPointer+1] = '\0';
				readBuffer[readBufferPointer] = '}';
                readBuffer[readBufferPointer+1] = 0x00;
                //checking the command
                
				if (readBufferPointer >= 12){
					sprintf (sendBuffer,"GET /rfid/status.php?number=%s HTTP/1.1\nHost: mainpage.us:80\n\n",readBuffer);
                    
                    
                    
					fprintf(stderr,"[---%s---]\n",  sendBuffer);
					if (requestGrant(sendBuffer) ){
                        //						fprintf(stderr,"i got it ok\n");
                        
					}else{
                        //						fprintf(stderr,"i got it error\n");
					}	
				}else{
                    //					fprintf(u0,"i got error\n");
                    //					fprintf(u0,"[%s]",  u0_Buf);
					readBufferPointer = 0;
				}
				memset(readBuffer,0,SERIAL_READ_BUFFER_SIZE);
			}else if ((ch == 10) || (ch ==13) ){
                //				readBufferPointer = 0;
                //				memset(readBuffer,0,SERIAL_READ_BUFFER_SIZE);
			}else {
				if ((readBufferPointer >= 0) && (readBufferPointer < 510 )){
					readBuffer[readBufferPointer] = ch;
					readBufferPointer++;
				}else{
					readBufferPointer = 0;
				    memset(readBuffer,0,SERIAL_READ_BUFFER_SIZE);
				}
			}
		}
        
	}
    
} // ReaderThread_0




/***************************************************************************
 *
 *  Usage
 *
 ****************************************************************************/

void Usage(){
	fprintf(stdout,"=============================================================\n" );
	fprintf(stdout,"rtpis [serial-port-name] [serial-port-name server-name]  \n");
	fprintf(stdout,"=============================================================\n" );
} // Usage 



/* ---- Private Function Prototypes -------------------------------------- */
void initialization(){
}






int requestGrant(char *argv){
	char hostname[100];
	char dir[DIRSIZE];
	int	sd;
	struct sockaddr_in pin;
	struct hostent *hp;
	
	
	printf("Sending the event\n");
    strcpy(hostname,RTPIS_SERVER_NAME);
	/* go find out about the desired host machine */
	if ((hp = gethostbyname(hostname)) == 0) {
		perror("gethostbyname");
		return 0;
	}
    
	/* fill in the socket structure with host information */
	memset(&pin, 0, sizeof(pin));
	pin.sin_family = AF_INET;
	pin.sin_addr.s_addr = ((struct in_addr *)(hp->h_addr))->s_addr;
	pin.sin_port = htons(RTPIS_SERVER_PORT);
    
	/* grab an Internet domain socket */
	if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		return 0;
	}
    
	/* connect to PORT on HOST */
	if (connect(sd,(struct sockaddr *)  &pin, sizeof(pin)) == -1) {
		perror("connect");
		return 0;
	}
    
	/* send a message to the server PORT on machine HOST */
	if (send(sd, argv, strlen(argv), 0) == -1) {
		perror("send");
		return 0;
	}else{
        printf("--server message start-- \n%s \n-- server message end--\n", argv);
	}
	
    //	dir[0]='\0';
    
    /* wait for a message to come back from the server */
    memset(dir,0,128);
	if (recv(sd, dir, DIRSIZE, 0) == -1) {
        perror("recv");
		return 1;
    }else{
		printf("Recived = %s\n",dir);
	}
    
    
	close(sd);
}





