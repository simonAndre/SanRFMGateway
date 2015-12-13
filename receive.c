// demo de lecture du port i2c de l'arduino


#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <regex.h>  
#include "gateway.h"
#include <inttypes.h>

#define BUFFSIZE	15	

#define EmonCmsUrl	"http://emon.azimut-monitoring.com/emoncms"
#define APIKEY		"8779a04214d628970b81eed27322c6cd"
#define GATEWAY_LIFESTATUSFQCY	15		//en secondes : frequence de demande d'envoi d'une trame de vie par la passerelle
#define GATEWAY_NBOFBADLIFEFRAMEFORREBOOT	20			//nb de lifeframe non reçues avant reboot de la passerelle 
#define WAITSENDSTATUS	20


 
int lastcurs=0;
char json[250]="";
time_t lasttime,lastsendserverstate;
char buf[BUFFSIZE];	// Buffer for data being read/ written on the i2c bus


// extrait un token depuis la chaine str via l'expression regu strregex et le met dans extracted
int extractRegexStr(char *str,char *strregex,char **extracted);
// envoie la chaine jsondata vers emoncms. jsondata est du style : {"FRID"=12,field1=2,field2=56} avec FRID : identifiant du node
void outputPost(char *jsondata);
//gère la chaine json
void handleJson(char *jsondata);
int readi2c(int i2cfd);
void gatewayWatcher(int i2cfd);
void SendServerStatusMgr(int i2cfd);
int openi2cchanel(int *fd);
char debugbuf[200];
int lfnotreceived=0;
char *logfile=NULL;
SanGateway sangateway;
bool debugmode=0;

int main(int argc, char **argv)
{
	printf("**** Gestionnaire de communication gateway arduino vers réseau SAN-emontx ****\n");
	sangateway.rebootGateway();
	_debugc_("nb arguments: %i",argc);
	if(argc>1){
		if(strcmp(argv[1],"debug")==0){
			debugmode=1;
			if(argc>2)
				logfile=argv[2];
		}
		else
			logfile=argv[1];
		if(logfile){
			FILE *f1;
			f1 = fopen(logfile, "w");
			if (f1 == NULL) {
				printf("I couldn't open %s for writing.\n",logfile);
				*logfile=NULL;
			}
			else
				fclose(f1);		
			
			
			_debugc_("fichier de log: %s",logfile);
			const char *d="Démarrage du log de reception des trames sansgateway \n";
			_log_("Démarrage du log de reception des trames sansgateway\n");
		}
	}
	// test des macro de debug
	_debugc_("debug avec param int: %i",45);
	_debugct_(" debug with date");
	
	
	int i2c_fd;	// File descrition
	while(1==1){
		if(sangateway.openi2cchanel(&i2c_fd)){
			readi2c(i2c_fd);
			gatewayWatcher(i2c_fd);		
			SendServerStatusMgr(i2c_fd);
			close(i2c_fd);
		}
		else
		{
			_debugct_("echec sur ouverture du canal i2c : %i",i2c_fd);
		}

			usleep(1000*100);		//attente 100ms pour libérer le proc
	}

	return 0;
}

// gestionnair d'envoi des status d'état serveur à la gateway
void SendServerStatusMgr(int i2cfd){
	double diff=0;
	if(lastsendserverstate)
		diff=difftime(time(NULL),lastsendserverstate);
	else
		time(&lastsendserverstate);

	if(diff>WAITSENDSTATUS)
	{
		_debugc_("Send server status");
		char s1=0,s2=0;
		if(sangateway.readState(&s1,&s2)){		//on récupère les valerus d'état
			char t[15];
			sprintf(t,"%i:%i:%i",MASTERQRY_PUSHSERVERSTATE,s1,s2);
			write(i2cfd,t, strlen(t)); 
		}
		lastsendserverstate=time(NULL);	//on réinitialise le compteur tempo
	}
}

/*
	surveille la passerelle et ordonne un reboot éventuellement
*/
bool settime=0;
void gatewayWatcher(int i2cfd){
	//watchdog like pour rebooter l'arduino s'il est planté
	double diffwd=0;
	time_t now;
	time(&now);
	if(lasttime)
		diffwd=difftime(now,lasttime);
	else
		time(&lasttime);
	
	if(diffwd>GATEWAY_LIFESTATUSFQCY){
		//char t[1];
		sangateway.SendCommand(i2cfd,(char)MASTERQRY_LIFEFRAME);
		_debugc_("the server ask for lifeframe");
		time(&lasttime);
		lfnotreceived++;
		settime=!settime;
		if(settime){
			usleep(1000*100);
			sangateway.SendTime(i2cfd);
		}
	}
	

	if(lfnotreceived>GATEWAY_NBOFBADLIFEFRAMEFORREBOOT)
	{
		// l'arduino ne répond pas depuis 30s => on le reboote (pin reset de l'arduino branché sur GPIO0 du raspi)
		lfnotreceived=0;
		//REBOOT
		_logt_(" REBOOT arduino");
		_debugct_(" REBOOT arduino");
		
		sangateway.rebootGateway();
	}
}

int readi2c(int i2cfd){
	memset(buf,'\0',BUFFSIZE);
	int so=read(i2cfd, buf, BUFFSIZE);
	if (so==0) {	//écoute les données
		_debugc_("Unable to read from slave\n");
		return 0;
	}
	//printf("nb d'octets lus depuis i2c: %i\t",so);
	/*for(int p=0;p<so;p++){
		printf("%x ",buf[p]);
	}*/
	char z=0x00;
	unsigned char eofpos=0,i=0;
	unsigned short csr=0;
	
	if(buf[0]!=0x00 && buf[0]!=0xff){
		do{
			z=buf[i];
			if(z==0x2 && eofpos==0)
			{
				_debug_("||",0);
				eofpos=i;
			}
			_debug_("%x ",z);
			i++;
		} while(i<30 && (eofpos==0||i<eofpos+3));
		_debug_("\n");
		if(eofpos>0){
			buf[eofpos]='\0';
			_debug_("\t%s\t",buf);
			//csr=0|(char)buf[eofpos+2]|((char)buf[eofpos+1])<<8;
			csr=sangateway.decodeUInt16(&buf[eofpos+1]);
			unsigned short csc=sangateway.checksum(buf,(unsigned char)eofpos);
			_debug_("\t csr=%i\t csc=%i\teofpos=%i\n\n",csr,csc,(unsigned char)eofpos);
			if(csr==csc){
			;
				//sangateway.SendCommandData(i2cfd,(char)MASTERQRY_ACKOWLEDGE,&buf[eofpos+1],2);
			}
		}else{
		_debug_("bad frame\n");
		}
	}
	/*if((int)buf[0]!=0xff){
		_debugc_("valeur lue : %s",buf);
		int a=0;
		if(lastcurs>0 || *buf=='{')
		{
			while((int)*(buf+a)!=255 && a<=BUFFSIZE && (char)*(buf+a)!=2){//*(buf+a-1)!='}'){
				*(json+lastcurs+a)=*(buf+a);
				a++;
			}
			if((char)*(buf+a)==2){	//marque de fin de transmission début de checksum
				unsigned short csr=*(buf+a+1)<<8|*(buf+a+2);
				unsigned short csc=sangateway.checksum(json+lastcurs,(unsigned char)a-1);
				_debugc_("checksum received : %i, calc: %i",csr,csc);
				
			}
			lastcurs+=a-1;
			if(*(json+lastcurs)=='}')		//fin de la phrase json
			{
				*(json+lastcurs+1)='\0';
				_debugc_("frame: %s",json);
				
				
				handleJson(json);	
				
				fflush(stdout);
				lastcurs=0;
				lasttime=time(NULL);	//on réinitialise le compteur tempo
			}
		}
		else if(buf[0]>0 && buf[1]>0)
		{
			_debugc_("reçu signal de vie gateway");
			lasttime=time(NULL);	//on réinitialise le compteur tempo
		}


	return 1;
	}
	*/

return 1;
}

// traite la chaine json jsondata est du style : {"fr"=12,"id"=23,field1=2,field2=56} 
void handleJson(char *jsondata){
	char *frameifstr=NULL,*cmdstr=NULL,*v1str=NULL,*v2str=NULL;
	int frameid=0,cmd=0,v1=0,v2=0;
	int crc=sangateway.rc_crc32(0, jsondata, strlen(jsondata));
	_debugc_("json reçu: %s",jsondata);
	//_debugc_("crc: 0x%08",crc);
	if(extractRegexStr(jsondata,(char*)"\"fr\":([0-9]+),",&frameifstr)){
		frameid=atoi(frameifstr);
		if(frameid>0){
			
			_logt_("%i  %s",frameid,jsondata);

			_debugc_("trame de type %i",frameid);

			switch(frameid){
				case FR_lifeframe:
				{
					char *nodeidstr=NULL,*freeramstr=NULL;
					int nodeid=0,freeram=0;
					if(extractRegexStr(jsondata,(char*)"\"id\":([0-9]+),",&nodeidstr) && extractRegexStr(jsondata,(char*)"\"freeram\":([0-9]+)[,\}]",&freeramstr))
					{
						nodeid=atoi(nodeidstr);
						freeram=atoi(freeramstr);
						_debugc_("trame de vie reçue du noeud %i, mémoire dispo : %i",nodeid,freeram);
						lfnotreceived=0;
					}
					free(nodeidstr);free(freeramstr);
				}
				break;
				case FR_emontx:
				{
					char *nodeidstr=NULL,*p1str=NULL;
					int nodeid=0,p1=0;
					if(extractRegexStr(jsondata,(char*)"\"id\":([0-9]+),",&nodeidstr) && extractRegexStr(jsondata,(char*)"\"p1\":([0-9]+)[,\}]",&p1str))
					{
						nodeid=atoi(nodeidstr);
						p1=atoi(p1str);
						_debugc_("trame emontx reçue du noeud %i, p1 : %i",nodeid,p1);
					}
					free(nodeidstr);free(p1str);
				}
				break;
				case FR_hygth:
				{
					char *nodeidstr=NULL,*tstr=NULL,*hstr=NULL;
					int nodeid=0,t=0,h=0;
					if(extractRegexStr(jsondata,(char*)"\"id\":([0-9]+),",&nodeidstr) && extractRegexStr(jsondata,(char*)"\"temperature\":([0-9]+)[,\}]",&tstr)
					 && extractRegexStr(jsondata,(char*)"\"hygro\":([0-9]+)[,\}]",&hstr))
					{
						nodeid=atoi(nodeidstr);
						t=atoi(tstr);
						h=atoi(hstr);
						_debugc_("trame HYGTH reçue du noeud %i, temp : %i, hygro : %i",nodeid,t,h);
					}
					free(nodeidstr);free(tstr);free(hstr);
				}
				break;			
				case FR_commands:
					if(extractRegexStr(jsondata,(char*)"\"cmd\":([0-9]+),",&cmdstr)){
						cmd=atoi(cmdstr);
						switch(cmd){
						case CMD_PUSHSERVERSTATE:
							if(extractRegexStr(jsondata,(char*)"\"v1\":([0-9]+),",&v1str) && extractRegexStr(jsondata,(char*)"\"v2\":([0-9]+)[,}]",&v2str)){
								v1=atoi(v1str);
								v2=atoi(v2str);
								sangateway.writeState(v1,v2);
								sangateway.actForStatus(v1,v2);
								_debugc_("trame commande server v1=%i, v2=%i",v1,v2);
							}
							free(v1str);free(v2str);
							break;
						default:
							break;
						}
					}
					free(cmdstr);
					break;
				default:
					outputPost(jsondata);
					break;
			}
		}
	}
	free(frameifstr);

}



// envoie la chaine jsondata vers emoncms. jsondata est du style : {"fr"=12,field1=2,field2=56} avec id : identifiant du node
void outputPost(char *jsondata){
	char strcurl[300];
	char *nodeidstr=NULL,*endjsonstr=NULL,*newjsonstr=NULL;
	if(extractRegexStr(jsondata,(char*)"\"id\":([0-9]+),",&nodeidstr)){
		int nodeid=atoi(nodeidstr);
		if(extractRegexStr(jsondata,(char*)"\"id\":[0-9]+,([^}]+})",&endjsonstr)){
			//on retire la paremiere partie FRID= de lachaine json
			newjsonstr=(char*)malloc(strlen(jsondata));
			newjsonstr[0]='{';
			strcpy(&newjsonstr[1],endjsonstr);
			sprintf(strcurl,"curl -G %s/input/post.json -d \"apikey=%s\" -d \"node=%i\" -d \"json=%s\"",EmonCmsUrl,APIKEY,nodeid,newjsonstr);
			//printf("%s\n",strcurl);
			system(strcurl);
			free(newjsonstr);
		}
	}
	free(endjsonstr);
}

// extrait un token depuis la chaine str via l'expression regu strregex et le met dans extracted
int extractRegexStr(char *str,char *strregex,char **extracted){
	//printf("resolution regex sur %s avec %sn",str,strregex);
	regex_t regex;
	regmatch_t *matchptr=NULL;
	int reti= regcomp(&regex,strregex ,REG_EXTENDED);
	if(reti){
		_debugc_("compilation impossible de la regex %s",strregex);
		return 0;
	}
	if(regex.re_nsub==0){	
		_debugc_("capture non définie dnas l'expression régulière (pas de parenthèses)");      
        return 0;
	}
	int nmatch=2;
    matchptr = (regmatch_t*)malloc (sizeof (*matchptr) * nmatch);
	reti=regexec(&regex,str,nmatch,matchptr,0);
    int start=matchptr[1].rm_so;
    int end=matchptr[1].rm_eo;
    size_t size = end - start;
    char *outstr = (char*)malloc (sizeof (*outstr) * (size + 1));
    strncpy(outstr,&str[start],size);
    outstr[size]='\0';
    *extracted=outstr;
	free(matchptr);
	//free(&regex);
	//free(outstr);
    return 1;
}


