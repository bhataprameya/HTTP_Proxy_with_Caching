#include "proxyserver.h"

int startserver ()
{
	int sd;                 /* socket descriptor */	
	struct sockaddr_in addr,addr2;
	char *servhost=malloc(sizeof("remote**.cs.binghamton.edu"));    /* full name of this host */
	ushort servport;                /* port assigned to this server */
	struct hostent *host;

	if ((sd = socket (AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf ("socket error");
		return -1;
	}

	addr.sin_family=AF_INET;
	addr.sin_port=htons(0);
	addr.sin_addr.s_addr=htons(INADDR_ANY);
	if ((bind(sd,(struct sockaddr *)&addr, sizeof(addr)))==-1)
	{
		printf("Bind error");return -1;
	}

	/* we are ready to receive connections */
	listen (sd, 5);
	//servport=addr.sin_port;
	if (gethostname(servhost,sizeof(servhost)+1) == -1)
	{
		printf ("gethostname error");
		return -1;
	}
	if ((host = gethostbyname(servhost)) == NULL)
	{
		printf ("gethostbyname error");
		return -1;
	}

	int size = sizeof (struct sockaddr_in);
	if (getsockname(sd, (struct sockaddr *) &addr2, &size) == -1)
	{
		printf ("getsockname error\n");
	}
	servport=ntohs(addr2.sin_port);

	/* ready to accept requests */
	printf (" HTTP proxy started on server  '%s' at port '%hu'\n", host->h_name, servport);
	strcpy(data.name,host->h_name);
	data.port=servport;
	free (servhost);
	return (sd);
}

void createList()
{
	cache = malloc(sizeof(List));
	cache->head = NULL;
	cache->tail = NULL;
}

void insertData(request **data)
{
	if (cache->head == NULL && cache->tail == NULL)
	{
		Node *node = malloc(sizeof(Node));
		node->rq = *data;
		node->next = NULL;
		node->prev = NULL;
		cache->head = node;
		cache->tail = node;
	}
	else 
	{
		Node *current = cache->head;
		Node *new_node = malloc(sizeof(Node));
		cache->head->prev = new_node;
		new_node->next = cache->head;
		new_node->prev = NULL;
		cache->head = cache->head->prev;
		cache->head->rq = *data;

	}
}




int removeData()
{	
	Node *current = NULL;
	int i;
	current = cache->tail;
	free(current->rq->msg);
	current->rq->msg=NULL;
	for(i=0;i<=current->rq->packetSize;i++)
	{
		free(current->rq->responce[i]);
		current->rq->responce[i]=NULL;	
	}
	free(current->rq->responce);
	current->rq->responce=NULL;
	free(current->rq->len);
	current->rq->len=NULL;
	free(current->rq->path);
	current->rq->path=NULL;
	//free(current->rq->url);
	cachesize-=current-> rq->size;
	cache->tail = current->prev;
	free(current);
	//current = NULL;
	if(cache->tail!=NULL)
		cache->tail->next = NULL;
	//free(current);
	if(cache->tail==NULL)
		return -1;
	return 1;

}


void leastRecentOnTop(int index)
{
	int count = 0;
	Node *prev_node = NULL, *next_node = NULL;
	Node *current = NULL;
	if (index >= 0)
	{
		current = cache->head;
		if (current == cache->tail && cache->tail != NULL)
		{
		}
		else if (index == 0)
		{
		}
		else
		{
			while (current != NULL && count < index)
			{
				current = current->next;
				count++;

			}
			if (index == count && current != cache->tail)
			{
				next_node = current->next;
				prev_node = current->prev;
				insertData(&(current->rq));
				free(current);
				current = NULL;
				prev_node->next = next_node;
				next_node->prev = prev_node;
			}
			else if (index == count && current == cache->tail)
			{
				cache->tail = current->prev;
				insertData(&(current->rq));
				free(current);
				current = NULL;
				cache->tail->next = NULL;
			}

		}
	}
}

int searchForward(int fd, char * data, clock_t start)
{
	int count = 0;
	long psize;
	clock_t end;
	char *temp;
	Node *current = cache->head;
	data[MAXMSG - 1] = '\0';
	temp = strtok(data, " ");
	temp = strtok(NULL, " ");
	fprintf(stderr ,"%s | ",temp);
	while (current != NULL)
	{
		if((strcmp(current->rq->path,temp))==0)
		{
			psize=current->rq->clength;
			if(sendResponceToClient(fd,&(current->rq)))
			{	 
				end = clock();
				fprintf(stderr ,"CACHE_HIT | ");
				leastRecentOnTop(count);
				free(data);
				end = clock();
				fprintf(stderr ," %ld | %f\n",psize,((double) (end - start)*1000) / CLOCKS_PER_SEC);
				return 0;
			}
			else 
			{
				free(data);
				return -1;
			}
		}
		current = current->next;
		count++;
	}
	free(data);
	return -1;
}

int hooktoserver (char *servhost, int servport)
{
	int sd;                 /* socket descriptor */
	ushort clientport;              /* port assigned to this client */
	struct sockaddr_in sa;
	struct hostent *host;
	int size = sizeof (struct sockaddr);
	if ((sd = socket (AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf ("socket error");
		exit (1);
	}

	if ((host = gethostbyname(servhost)) == NULL)
	{
		printf ("gethostbyname error");
		exit (1);
	}
	sa.sin_family = AF_INET;
	sa.sin_port = htons (servport);
	memcpy (&sa.sin_addr.s_addr, host->h_addr, host->h_length);

	if (connect (sd, (struct sockaddr *) &sa, sizeof (sa)) == -1)
	{
		printf ("connect error");
		exit (1);
	}

	if (getsockname(sd, (struct sockaddr *) &sa, &size) == -1)
	{
		printf ("getsockname error\n");
	}
	clientport = ntohs (sa.sin_port);
	/* succesful. return socket descriptor */
	return (sd);
}

int sendRequest(request **ra,int fd,int msg_len, clock_t start,int connectport)
{
	int sock,x,len,hlength;
	clock_t end;
	char *responce,*temp=NULL,*temp2,*temp3,*temp4,*temp5,*temp6,*temp7,*temp8;
	fd_set rfds;
	int retval;
	int s=-1,t;
	long clength,remread=MAXMSG;
	request *rq=*ra;
	rq->size=0;

	// get hooked on to the server 
	sock = hooktoserver(rq->url, connectport);
	if (sock == -1) 
	{
		perror("Error: hook to server ");
		exit(1);
	}
	if ((x= write(sock,rq->msg,msg_len)) <= 0) 
	{
		printf("request sending to server failed");
		close(sock);
		close(fd);
		exit(1);

	}


	FD_ZERO(&rfds);
	FD_SET(sock, &rfds);

	retval=select(sock+1,&rfds,NULL,NULL,NULL);
	if (FD_ISSET(sock, &rfds)) 
	{
		rq->responce=(char **)malloc(MAXMSG-1*sizeof(char *));
		rq->len=malloc(MAXMSG-1*sizeof(int));
		while(remread>0)
		{
			temp= malloc(MAXMSG);
			memset(temp, '\0', MAXMSG-1);
			if(len= read(sock,temp,MAXMSG))
			{	
				if(s==-1)
				{
					insertData(&rq);
					temp2= malloc(MAXMSG);
					strncpy(temp2,temp,len);
					temp3=strstr(temp2,"HTTP/1.1");
					temp4=strstr(temp3,"\r\n\r\n");
					hlength=(temp4-temp3)+4;
					temp5=malloc(MAXMSG);
					strncpy(temp5,temp2,hlength);
					temp6=strtok(temp5,"\r\n");
					while(1)
					{

						if(temp7=strstr(temp6,"Content-Length:"))
						{	
							temp8=strtok(temp6," ");
							temp8=strtok(NULL," ");
							clength= atoi(temp8);
							rq->clength=clength;
							break;
						}
						temp6=strtok(NULL,"\r\n");
					}
					free(temp2);
					free(temp5);
					remread=clength-(len-hlength);
				}
				else remread-=len;
				rq->len[++s]=len;
				rq->responce[s]=temp;  
				cachesize+=len;
				write(fd,temp,len);
				rq->packetSize=s;
				rq->size+=len;

			}
			else break;
		}
		end = clock();
		fprintf(stderr ," %ld | %f\n",clength,((double) (end - start)*1000) / CLOCKS_PER_SEC);

		while(cachesize>cachelimit)
		{
			removeData();				
		}
	}
	close(sock);
	close(fd);
	return 1;

}
void sig_handler(int signo)
{
	while(cache->tail)
		removeData();
	free(cache);
	exit(0);
}

void* handleRequest(void* csd)
{
	int cachehit=0,port=80;
	int fd= *((int *)csd);
	char *elements,*msg,*temp,*msg2;
	clock_t start;
	request *rq;
	pthread_detach(pthread_self());
	msg=malloc(MAXMSG);
	msg2=malloc(MAXMSG);
	int x=read(fd,msg,MAXMSG);
	if(x)
	{
		strncpy(msg2,msg,x);	
		start = clock();
		cachehit=searchForward(fd,msg2,start);				
		if(cachehit!=0)
		{	
			rq=malloc(sizeof(request));	
			fprintf(stderr ,"CACHE_MISS | ");
			rq->msg=malloc(MAXMSG);
			strncpy(rq->msg,msg,x);
			msg[MAXMSG - 1] = '\0';
			temp = strtok(msg, " ");
			temp = strtok(NULL, " ");
			rq->path=malloc(MAXLINE);
			strcpy(rq->path,temp);
			elements = strtok(temp, "//");
			elements = strtok(NULL,"/");
			temp = strtok(elements,":");
			temp = strtok(NULL,":");
			if(temp)
				port=atoi(temp);
			rq->url=elements;

			sendRequest(&rq,fd,x,start,port);


		}
	}
	else
	{
		printf("request reading from client failed \n");
	}
	free(msg);


}

int sendResponceToClient(int fd,request **ra)
{
	request *rq=*ra;
	int j,i=rq->packetSize;
	for(j=0;j<=i;j++)
	{
		if(write(fd,rq->responce[j],rq->len[j]))
		{
			continue;
		}
		else
		{
			return -1;
		}
	}
	return 1;
}

int main(int argc, char *argv[])
{
	int fd,livesdmax;
	pthread_t tid1;
	fd_set livesdset,livesdset2;
	if(argc!=2)
	{
		printf("Invalid no of parameters\n");
		exit(1);
	}
	signal(SIGINT, sig_handler);
	cachelimit=atoi(argv[1]);
	cachesize=0;
	createList();
	fd= startserver();
	FD_ZERO(&livesdset);
	livesdmax=fd;
	FD_SET(fd, &livesdset);
	/* receive requests and process them */
	while (1) 
	{
		livesdset2=livesdset;
		if(select(livesdmax+1,&livesdset2,NULL,NULL,NULL)<0)
			continue;
		if (FD_ISSET(fd, &livesdset2)) 
		{
			struct hostent * host;
			struct sockaddr_in client;
			int csd, len=sizeof(struct sockaddr);
			csd=accept(fd,(struct sockaddr *)&client,&len);
			if(csd>0)
			{	fprintf(stderr ,"%s | ",inet_ntoa(client.sin_addr));
				pthread_create(&tid1,NULL,handleRequest,&csd);
			}

		}



	}


}
