dir := ./bin/

all:
	gcc -Wall -c common.c -o $(dir)common.o
	gcc -Wall client.c $(dir)common.o -o $(dir)client
	gcc -Wall -lpthread server.c $(dir)common.o -o $(dir)server

 clean:
	rm -rf ./bin/common.o ./bin/client ./bin/server