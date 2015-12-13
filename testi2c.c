#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
 
// The PiWeather board i2c address
#define ADDRESS 0x04
 
// The I2C bus: This is for V2 pi's. For V1 Model B you need i2c-0
static const char *devName = "/dev/i2c-1";
 
int main(int argc, char** argv) {
 
	int file;
	char *filename = "/dev/i2c-1";
	while(1==1){
	if ((file = open(filename, O_RDWR)) < 0) {
		/* ERROR HANDLING: you can check errno to see what went wrong */
		perror("Failed to open the i2c bus");
		exit(1);
	}
	int addr = 0x04;        
	if (ioctl(file, I2C_SLAVE, addr) < 0) {
		printf("Failed to acquire bus access and/or talk to slave.\n");
		/* ERROR HANDLING; you can check errno to see what went wrong */
		exit(1);
	}
		
	char buf[10] = {0};
	float data;
	char channel;

	for (int i = 0; i<40; i++) {
		// Using I2C Read
		if (read(file,buf,1) != 1) {
			/* ERROR HANDLING: i2c transaction failed */
			printf("Failed to read from the i2c bus.\n");
			//buffer = g_strerror(errno);
			//printf(buffer);
			printf("\n\n");
		} else {
			
			printf("%i",buf[0]);
		}
	}	
	printf("\n");
	close(file);}
	usleep(1000*30);
}