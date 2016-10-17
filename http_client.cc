#include "minet_socket.h"
#include <stdio.h>
#include <cstring>
#include <stdlib.h>
#include <ctype.h>

#define BUFSIZE 1024

int main(int argc, char * argv[]) {

    char * server_name = NULL;
    int server_port    = -1;
    char * server_path = NULL;
    char * req         = NULL;
    bool ok            = false;
	fd_set rfds;
	
	char dataread[5920000];
	int len = 5920000;
	hostent * host;
	
    /*parse args */
    if (argc != 5) {
	fprintf(stderr, "usage: http_client k|u server port path\n");
	exit(-1);
    }


    server_name = argv[2];
    server_port = atoi(argv[3]);
    server_path = argv[4];
	struct sockaddr_in server_addr;

    req = (char *)malloc(strlen("GET  HTTP/1.0\r\n\r\n") 
			 + strlen(server_path) + 1);  

    /* initialize */
    if (toupper(*(argv[1])) == 'K') { 
	minet_init(MINET_KERNEL);
    } else if (toupper(*(argv[1])) == 'U') { 
	minet_init(MINET_USER);
    } else {
	fprintf(stderr, "First argument must be k or u\n");
	exit(-1);
    }

    /* make socket */  //OK
	int socket = minet_socket(SOCK_STREAM); 
	if(socket < 0) 
	{
		fprintf(stderr, "ERROR opening socket\n");
		exit(-1);
	}
		

    /* get host IP address  */
    /* Hint: use gethostbyname() */
	host = gethostbyname(server_name);
	if (host == NULL)
	{
		fprintf(stderr, "ERROR, no such host");
		exit(-1);
	}
	//hostip = host->h_addr_list;

    /* set address */
	memset(&server_addr, '0', sizeof(server_addr));

	server_addr.sin_family = AF_INET;

    bcopy((char *)host->h_addr, (char *)&server_addr.sin_addr.s_addr, host->h_length); 
    
    server_addr.sin_port = htons(server_port); 

    /* connect to the server socket */
	
	int connection = minet_connect(socket,  &server_addr);
	if (connection < 0)
	{
		fprintf(stderr, "Error connecting\n"); 
		exit(-1);
	}
	

    /* send request message */
    sprintf(req, "GET /%s HTTP/1.0\r\n\r\n", server_path);
	minet_write(socket, req, len);

    /* wait till socket can be read. */
    /* Hint: use select(), and ignore timeout for now. */
	FD_ZERO(&rfds);
    FD_SET(socket, &rfds);
	select(socket + 1, &rfds, NULL, NULL, NULL);
	
	int numbytes = minet_read(socket, dataread, len);
	int count;

    /* first read loop -- read headers */
	for(count = 0; count < numbytes; ++count)
	{
		if(dataread[count] == 'H' && dataread[count+1] == 'T' && dataread[count+2] == 'T' && dataread[count+3] == 'P' && dataread[count+4] == '/' && dataread[count+5] == '1' && dataread[count+6] == '.' && dataread[count+7] == '0')
		{
			if(dataread[count+9] == '2' && dataread[count+10] == '0' && dataread[count+11] == '0' )
			{
				ok = true;
				break;
			}
		}
		else if(dataread[count] == 'H' && dataread[count+1] == 'T' && dataread[count+2] == 'T' && dataread[count+3] == 'P' && dataread[count+4] == '/' && dataread[count+5] == '1' && dataread[count+6] == '.' && dataread[count+7] == '1')
		{
			if(dataread[count+9] == '2' && dataread[count+10] == '0' && dataread[count+11] == '0' )
			{
				ok = true;
				break;
			}
		}
	}
    /* examine return code */   

	//char *strstr(const char *s1, const char *s2) ?
    //Skip "HTTP/1.0"
    //remove the '\0'
	if(ok)
	{
		for(count = 0; count < numbytes; ++count)
		{
			if(dataread[count] == '\r' && dataread[count+1] == '\n' && dataread[count+2] == '\r' && dataread[count+3] == '\n')
			{
				fprintf(stdout, dataread + (count + 4));
				break;
			}
		}
	}
	else
	{
		fprintf( stderr, dataread);
	}
	
	//these comments don't seem to match the writeup
	//writeup says to print only the data for a 200 OK

    // Normal reply has return code 200

    /* print first part of response: header, error code, etc. */

    /* second read loop -- print out the rest of the response: real web content */

    /*close socket and deinitialize */
	minet_close(socket);

    if (ok) {
	return 0;
    } else {
	return -1;
    }
}
