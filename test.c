//Prints in accuracy of seconds

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "gateway.h"
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#define BUFFSIZE	30	

char code[10],*buf;
int i2c_fd;
SanGateway sangateway;
int readi2c(int i2cfd);
bool debugmode=1;
int main(int argc, char **argv)
{
    char input;
    do{
        unsigned i, j;
        uint32_t val;
        printf("valeur à coder?  ");
        scanf("%i", &j);
        val=(uint32_t)j;
        for(int i=0;i<4;i++){
            char b=val>>(i*8);
			
            printf("octet %i : %i\n",i+1,(uint8_t)b);
        }
        _debugct_(" valeur: %i\n",val);
        _debugct_("lecture i2c\n");
		if(sangateway.openi2cchanel(&i2c_fd)){
			readi2c(i2c_fd);
			write(i2c_fd,(char*)val,4);
		}
        input=getchar();
    }
    while(input!='q');

    return EXIT_SUCCESS;
}

int readi2c(int i2cfd){
	buf=(char*)calloc(BUFFSIZE,1);
	int so=read(i2cfd, buf, BUFFSIZE);
	if (so==0) {	//écoute les données
		printf("Unable to read from slave\n");
		return 0;
	}
	printf("nb d'octets lus depuis i2c: %i\n",so);
	for(int p=0;p<so;p++){
		printf("%i ",buf[p]);
	}
	printf("\n");
	free(buf);
	return 1;
}
