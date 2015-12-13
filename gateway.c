#include <stdio.h>
#include <stdlib.h>
#include "gateway.h"
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
int openi2cchanel(int *fd){
	const char *i2cfilename = "/dev/i2c-1"; // Name of the port we will be using
	if ((*fd = open(i2cfilename, O_RDWR)) < 0) {	// Open port for reading and writing
		printf("Failed to open i2c port\n");
		return 0;
	}
	if (ioctl(*fd, I2C_SLAVE, I2CADRESSGATEWAY) < 0) {// Set the port options and set the address of the device we wish to speak to
		printf("Unable to get bus access to talk to slave\n");
		return 0;
	}
	return 1;
}

int SendCommandData(int fd,char command,char *data){
	/*if ((fd = open(fileName,O_WRONLY)) < 0) {	// Open port for reading and writing
		printf("Failed to open i2c port for writing\n");
		return 0;
	}*/
	char *t=NULL;
	t=malloc(strlen(data)+1);
	sprintf(t,"%c%s",command,data);
	write(fd,t, strlen(t)); 	
	//close(fd);
	return 1;
}
int SendCommand(int fd,char command){
	char *t=NULL;
	t=malloc(1);
	sprintf(t,"%c",command);
	write(fd,t, strlen(t)); 	
	return 1;
}


//utilise le fichier STATEFILE pour stocker les valerus d'état du système. 
int writeState(char serverState1,char serverState2)
{
  FILE *fp;
  int i;

  fp = fopen(STATEFILE, "w");
  if (fp == NULL) {
	 printf("I couldn't open %s for writing.\n",STATEFILE);
	 return 0;
  }
  char statestr[2];
  statestr[STB_value1]=serverState1;
  statestr[STB_value2]=serverState2;
  fwrite(statestr,1,sizeof(statestr),fp);
  //fprintf(fp, "%c,%c", serverState1,serverState2);

  fclose(fp);
  return 1;
}
int readState(char *serverState1,char *serverState2)
{
  FILE *fp;
  int i;

  fp = fopen(STATEFILE, "r");
  if (fp == NULL) {
	 printf("I couldn't open %s for reading.\n",STATEFILE);
	 return 0;
  }
  char a,b;
  
  
  
  //int nbread=fscanf(fp, "%c,%c", &a,&b);
  if(feof(fp))
	return 0;
  *serverState1=fgetc(fp);
  if(feof(fp))
	return 0;
  *serverState2=fgetc(fp);
  fclose(fp);
  return 1;
}

// execute les actions définies dans le status serveur
int actForStatus(char s1,char s2){
	printf("s1=%i, s2=%i\n",s1,s2);
	char cmd[300]="";
	for(int i=0;i<ST_NUMBERS;i++){
		char ison=(s1&(1<<i));
		/*if(ison)
			printf("%s ON\n",statusList[i]);
		else
			printf("%s OFF\n",statusList[i]);*/
		switch (i) {
			case ST_WIFI:
				sprintf(cmd,"curl http://serenandre.hd.free.fr:81/switchWIFI/%s",ison?"on":"off");
				printf("%s",cmd);
				system(cmd);
				break;
			default:
				break;
		}
	}
}