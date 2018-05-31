#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <time.h>
#define MAXMSG 65534
#define MAXLINE 1024

typedef struct
{
	int size;
	int packetSize;
	int *len;
	char *msg;
        char *url;
	char *path;
	long clength;
	char **responce;
}request;


long cachelimit;
long cachesize;



typedef struct
{
	char name[50];
	int port;
}mydata;

mydata data;


typedef struct node
 {
 request *rq;
 struct node * next;
 struct node * prev;
}Node;

typedef struct
 {
 Node * head;
 Node * tail;
}List;
List *cache;

int startserver ();
void createList();
void insertData();
int removeData();
void leastRecentOnTop();
int searchForward();
int hooktoserver();
int sendRequest();
void* handleRequest();
int sendResponceToClient();
void sig_handler(int signo);
