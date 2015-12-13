receive:	receive.c gateway.h gateway.c
	g++ -g -o receive receive.c gateway.c 
send:   send.c gateway.h gateway.c
	g++ -g -o send send.c gateway.c  

status:   status.c gateway.h gateway.c
	gcc -g -o status status.c gateway.c  -std=gnu99
test:   test.c gateway.h gateway.c
	g++ -g -o test test.c gateway.c 

testi2c:   testi2c.c 
	g++ -g -o testi2c testi2c.c 
