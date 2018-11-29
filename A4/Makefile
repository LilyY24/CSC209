PORT=50272
CFLAGS = -DPORT=\$(PORT) -Wall -g

hcq_server: hcq_server.o hcq.o server.o
	gcc $(CFLAGS) -o hcq_server hcq_server.o hcq.o server.o

helpcentre: helpcentre.o hcq.o 
	gcc $(CFLAGS) -o helpcentre helpcentre.o hcq.o

helpcentre.o: helpcentre.c hcq.h
	gcc $(CFLAGS) -c helpcentre.c

hcq.o: hcq.c hcq.h
	gcc $(CFLAGS) -c hcq.c

hcq_server.o: hcq_server.c server.h 
	gcc $(CFLAGS) -c hcq_server.c

server.o: server.c server.h
	gcc $(CFLAGS) -c server.c

clean: 
	rm helpcentre hcq_server *.o
