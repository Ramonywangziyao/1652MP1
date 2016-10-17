#include "minet_socket.h"
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/stat.h>

#define BUFSIZE 1024
#define FILENAMESIZE 100

int handle_connection(int sock);
int connection_socket = -1;
char buf[BUFSIZE];
int listener;
struct sockaddr_in addr;

int main(int argc, char * argv[]) {
    int server_port = -1;
    int rc          =  0;
    int sock        = -1;
    /* parse command line args */
    if (argc != 3) {
	fprintf(stderr, "usage: http_server1 k|u port\n");
	exit(-1);
    }

    server_port = atoi(argv[2]);

    if (server_port < 1500) {
	fprintf(stderr, "INVALID PORT NUMBER: %d; can't be < 1500\n", server_port);
	exit(-1);
    }

    /* initialize and make socket */
    if (toupper(*(argv[1])) == 'K') {
	minet_init(MINET_KERNEL);
    } else if (toupper(*(argv[1])) == 'U') {
	minet_init(MINET_USER);
    } else {
	fprintf(stderr, "First argument must be k or u\n");
	exit(-1);
    }
    sock = minet_socket(SOCK_STREAM);
    if(sock == -1){
      printf("cannot open socket. Terminating.");
      return -1;
    }
    /* set server address*/

    addr.sin_port = htons(server_port);
    addr.sin_addr.s_addr = 0;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_family = AF_INET;

    /* bind listening socket */
    if(minet_bind(sock,&addr) == -1){
      printf("error binding. Terminating.");
      return -1;
    }
    /* start listening */
    listener =  minet_listen(sock,1);
    /* connection handling loop: wait to accept connection */
    while(connection_socket == -1){
      connection_socket =  minet_accept(sock, &addr);
    }
    while (1) {    /* handle connections */
	    rc = handle_connection(sock);
    }
  }

int handle_connection(int sock) {
    bool ok = false;

    char * ok_response_f = "HTTP/1.0 200 OK\r\n"	\
	"Content-type: text/plain\r\n"			\
	"Content-length: %d \r\n\r\n";

    char * notok_response = "HTTP/1.0 404 FILE NOT FOUND\r\n"	\
	"Content-type: text/html\r\n\r\n"			\
	"<html><body bgColor=black text=white>\n"		\
	"<h2>404 FILE NOT FOUND</h2>\n"
	"</body></html>\n";
    int n;
    while((n = minet_read(connection_socket, buf, BUFSIZE)) > 0){    /* first read loop -- get request and headers*/
      printf("receiving from client.");
      puts(buf);
      printf(buf);
    }
    char fileName[strlen(buf)-12];
    memcpy( fileName, &buf[4], strlen(buf)-13);
    fileName[strlen(fileName)-1] = '\0';
    /* parse request to get file name */
    /* Assumption: this is a GET request and filename contains no spaces*/

    /* try opening the file */
    char *content = NULL;
    FILE *fp = fopen(fileName, "r");
    if (fp != NULL) {
      if (fseek(fp, 0L, SEEK_END) == 0) {
          long bufferSize = ftell(fp);
          if (bufferSize == -1) { printf("Error file!");}
          content = (char*)malloc(sizeof(char)*(bufferSize + 1));
          if (fseek(fp, 0L, SEEK_SET) != 0) { printf("Error file!"); }
          size_t flen = fread(content, sizeof(char), bufferSize, fp);
          if ( ferror(fp) != 0 ) {
              fputs("Error reading file", stderr);
          } else {
              content[flen++] = '\0';
          }
      }
        fclose(fp);
    }
    /* send response */
    int responseSent = minet_write(sock,ok_response_f,strlen(ok_response_f));
    if (responseSent > -1) {
	/* send headers */
      //minet_write(sock,ok_response_f,strlen(ok_response_f));
	/* send file */
      minet_write(sock,content,strlen(content));
    } else {
	// send error response
      minet_write(sock,notok_response,strlen(ok_response_f));
    }

    /* close socket and free space */
    minet_close(sock);
    if (ok) {
	     return 0;
    } else {
	     return -1;
    }
}
