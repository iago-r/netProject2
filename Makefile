dir := ./bin/

all:
	gcc -Wall -c common.c -o $(dir)common.o
	gcc -Wall -pthread client.c $(dir)common.o -o $(dir)client
	gcc -Wall -pthread server.c $(dir)common.o -o $(dir)server

 clean:
	rm -rf $(dir)common.o $(dir)client $(dir)server