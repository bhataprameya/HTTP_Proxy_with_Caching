all: proxyserver.c proxyserver.h
	gcc -g -pthread proxyserver.c -o proxy
clean:
	rm -f *.o *.out proxy
