#include <stdio.h>
#include <stdlib.h>
#include <linux/i2c-dev.h>
#include <inttypes.h>

#define STATEFILE	"/home/pi/electronique/i2c/SanRFMGateway/serverstate"

#ifndef gateway_h__
#define gateway_h__
 
#define FR_emontx  1
#define FR_glcd    2
#define FR_hygth   3
#define FR_commands   4
#define FR_solar   5
#define FR_test   6
#define FR_lifeframe   7
//commandes du serveur > glcd
#define CMD_PUSHSERVERSTATE 0    // envoie l'état courant du serveur
#define CMD_STATE 4    // positionne la variable d'atat du GLCD
#define CMD_SETTIME 6  // mise à l'heure
//actions externes
#define AC_REBOOTSERVER  1    // reboot le raspi
#define AC_WIFI  2    // switch wifi on / off sur freebox
#define AC_VMC  3    // switch VMC on / off
#define AC_AROSAGE1  4    // switch arosage zone 1 on / off
#define AC_AROSAGE2  5    // switch arosage zone 2 on / off
//états externes (position des bits) les états sont maintenus par le serveur
#define ST_NUMBERS	6	//nb d'états
#define ST_SERVERSTATERECEIVED  0    // à 1 si l'état du serveur est connu
#define ST_WIFI  1    // état wifi freebox
#define ST_IRRIG1  2    // état arrosage zone 1 
#define ST_IRRIG2  3    // état arrosage zone 2
#define ST_VMC  4    // état VMC
#define ST_SERVEROFF  5  //ordre d'arret du serveur

//protocole stockage de la valeur d'état dnas le fichier STATEFILE : position des bits
#define STB_value1	0	//stockage de l'octet 1, taille : 1
#define STB_value2	1	//stockage de l'octet 2, taille : 1
#define STB_date	2	//stockage de l'octet 2, taille : 1

//  codes de requettes i2c envoyées par le serveur (maitre)
#define MASTERQRY_LIFEFRAME  1  // demande d'envoi de trame de vie
#define MASTERQRY_SETTIME  2  // mise à l'heure
#define MASTERQRY_PUSHFRAME  3  // publie une trame fournie (dans la suite de la trame i2c)
#define MASTERQRY_PUSHSERVERSTATE 4  //publie l'état du serveur
#define MASTERQRY_SOLARSTATE 5
#define MASTERQRY_SETINTERVAL 6
#define MASTERQRY_SENDSTATE 7
#define MASTERQRY_ACKOWLEDGE	8

#define I2CADRESSGATEWAY 0x04	// adresse de l'arduino sur le bus


//macro de debug : affiche le texte si le debug est activé
//#define _debug_(fmt) if(DEBUG) {fprintf(stderr, "%s",fmt);}
#define _debug_(fmt, ...) if(debugmode) {fprintf(stdout, fmt,__VA_ARGS__);}

//macro de debug : affiche le texte avec le contexte si le debug est activé 
#define _debugc_(fmt, args...) \
        do { if (debugmode) { char *__debugbasebuf, *__debugbuf; \
							int basesize=strlen(__FILE__)+strlen(__func__)+15; \
							int fullsier=basesize+strlen(fmt)+ 200; \
							__debugbasebuf=(char*)malloc(basesize); \
							__debugbuf=(char*)malloc(fullsier); \
							sprintf(__debugbasebuf, "%s:%d:%s()", __FILE__, __LINE__, __func__); \
							sprintf(__debugbuf,fmt ,##args); \
							fprintf(stdout,"%s - %s\n",__debugbasebuf,__debugbuf); \
							free(__debugbasebuf); free(__debugbuf); \
						}} while (0)
						
// idem _debugc_ avec date/heure								
#define _debugct_(fmt, args...) \
        do { if (debugmode) { char *__debugbasebuf, *__debugbuf; \
							int basesize=strlen(__FILE__)+strlen(__func__)+25; \
							int fullsier=basesize+strlen(fmt)+ 200; \
							__debugbasebuf=(char*)malloc(basesize); \
							__debugbuf=(char*)malloc(fullsier); \
							char *t=gettime();	\
							sprintf(__debugbasebuf, "%s %s:%d:%s()",t, __FILE__, __LINE__, __func__); \
							sprintf(__debugbuf,fmt ,##args); \
							fprintf(stdout,"%s - %s\n",__debugbasebuf,__debugbuf); \
							free(__debugbasebuf); free(__debugbuf);	\
						}} while (0)

#define _logt___(fmt,withtime, args...) \
        do { if (logfile) { char *__debugbuf,*__debugbuf2; \
							int fullsiz=strlen(fmt)+ 220; \
							__debugbuf=(char*)malloc(fullsiz); \
							sprintf(__debugbuf,fmt ,##args); \
							if(withtime){ \
								__debugbuf2=(char*)malloc(fullsiz); \
								char *t=gettime();	\
								sprintf(__debugbuf2,"%s - %s",t,__debugbuf); \
								sangateway.writeDataToDisk(logfile,__debugbuf2);	\
								free(__debugbuf2);	\
							} else sangateway.writeDataToDisk(logfile,__debugbuf);	\
							free(__debugbuf);	\
						}} while (0)
						
#define _log_(fmt, args...) _logt___(fmt,0,##args)
#define _logt_(fmt, args...) _logt___(fmt,1,##args)
//#define _logt_(fmt, args...) do { if (logfile) {sangateway.writeDataToDisk(logfile,(char*)string);}} while (0)

extern bool debugmode;

class SanGateway{
	public:
	int writeState(char serverState1,char serverState2);
	int readState(char *serverState1,char *serverState2);
	int SendCommandData(int fd,char command,char *data,int length);
	int SendTime(int fd);
	int SendCommand(int fd,char command);
	int actForStatus(char s1,char s2);
	int openi2cchanel(int *fd);
	int writeDataToDisk(char *filename,char *data);
	uint32_t rc_crc32(uint32_t crc, const char *buf, size_t len);
	void rebootGateway();
	
	unsigned short checksum(char *s,unsigned char length);
	char* encodeUInt(unsigned v);
	unsigned decodeUInt(char *v);
	char* encodeUInt16(unsigned short v);
	unsigned short decodeUInt16(char *v);
	
	
	private:
	int none;
	char bufcmd[300];
};


char* gettime();


#endif  // gateway_h__



