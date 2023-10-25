dir := ./bin/

all:
	gcc -Wall -c common.c -o $(dir)common.o
	gcc -Wall client.c $(dir)common.o -o $(dir)client
	gcc -Wall server_lib.c $(dir)server_lib.o
	gcc -Wall -pthread server.c $(dir)common.o $(dir)server_lib.o -o $(dir)server

 clean:
	rm -rf $(dir)common.o $(dir)client $(dir)server