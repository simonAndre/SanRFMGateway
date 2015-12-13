// gestion de l'état du server

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
const char statusList[ST_NUMBERS][30]={"ST_SERVERSTATERECEIVED","ST_WIFI","ST_IRRIG1","ST_IRRIG2","ST_VMC","ST_SERVEROFF"};




int main(int argc, char **argv)
{
	/*char sa=220,sb=3;
	
	writeState(sa,sb);
	if(readState(&s1,&s2))
		printf("s1=%i, s2=%i\n",s1,s2);
	int sc=15,sd=43;
	writeState(sc,sd);
	if(readState(&s1,&s2))
		printf("s1=%i, s2=%i\n",s1,s2);
	exit(0);
	*/
	char s1=0,s2=0;

	
	if(strcmp(argv[1],"get")==0){
		if(readState(&s1,&s2)){
			printf("s1=%i, s2=%i\n",s1,s2);
		}
		for(int i=0;i<ST_NUMBERS;i++){
			if(s1&(1<<i))
				printf("%s ON\n",statusList[i]);
			else
				printf("%s OFF\n",statusList[i]);
		}
		actForStatus(&s1,&s2);
	}
	if(strcmp(argv[1],"set")==0){
		//int *s1_,*s2_;
		s1=atoi(argv[2]);
		s2=atoi(argv[3]);
		if(writeState(s1,s2)){
			printf("ok : s1=%i, s2=%i\n",s1,s2);
		}
	}
	
	if(strcmp(argv[1],"setb")==0){
		char v=0;
		for(int i=0;i<ST_NUMBERS && i<strlen(argv[2]);i++){
			char vs=*(argv[2]+strlen(argv[2])-i-1);
			int v=atoi(&vs);
			//printf("%i\n",v);
			s1|=(v<<i);
		}
		if(writeState(s1,s2))	printf("ok : s1=%i, s2=%i\n",s1,s2);
	}
	return 0;
}

