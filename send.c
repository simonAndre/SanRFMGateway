// envoi d'infos au noeuds terminaux via le gateway

#include <stdio.h>
#include <stdlib.h>
#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include "gateway.h"

char strResponse [128] ;


SanGateway sangateway;
/*
convention pour les envois de commande : 
char1 = commande (entier codée en char)
la suite : chaine de commande.
*/


      // avec code s : envoie d'un état codé dans un char
      // avec : bits 1=IHM, 2=SendData, 3=ScrenLight, 4=greenled
      //ex : s:8 pour allumer l'écran  s:4 pour demander les données

int main(int argc, char **argv)
{
	int i2c_fd;	// File descrition
	if(sangateway.openi2cchanel(&i2c_fd)){
		if(strcmp(argv[1],"time")==0){
			sangateway.SendTime(i2c_fd);
		}
		else if(strcmp(argv[1],"status")==0){
			char t[15];
			char s1,s2;
			if(sangateway.readState(&s1,&s2)){
				sprintf(t,"%c%c",s1,s2);
				sangateway.SendCommandData(i2c_fd,(char)MASTERQRY_SENDSTATE,t,2); 
			}
		}
		else if(strcmp(argv[1],"st")==0){
			char t[15];
			char s1,s2;
			if(sangateway.readState(&s1,&s2)){
				sangateway.actForStatus(s1,s2);
			}
		}
		else {
			write(i2c_fd, argv[1], strlen(argv[1]));  	// envoie d'une commande à l'arduino
		}
		
		close(i2c_fd);
	}
	/*
	int fd;
	char *fileName = "/dev/i2c-1";// Name of the port we will be using
	int  address = 0x04;	// adresse de l'arduino sur le bus
	int i=0;
	
	if ((fd = open(fileName,O_WRONLY)) < 0) {	// Open port for reading and writing
		printf("Failed to open i2c port\n");
		exit(1);
	}
	
	if (ioctl(fd, I2C_SLAVE, address) < 0) {	// Set the port options and set the address of the device we wish to speak to
		printf("Unable to get bus access to talk to slave\n");
		exit(1);
	}
	close(fd);
	return 0;
	*/
}

