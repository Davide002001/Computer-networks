/* CLIENT S*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include "fullwrapped.h"

struct green_pass {
  char TS[21];
  time_t Scad;
  int validita;
  int option;
};

int main(int argc, char **argv)
{
  int sockfd, n;
  
  struct sockaddr_in servaddr;
  struct green_pass gp_send;
  


  if (argc != 4) {
    fprintf(stderr,"usage: %s <IPaddress> <TS> <V/I>\n",argv[0]);
    exit (1);
  }
  
  if(strlen(argv[2]) != 20){
        fprintf(stderr,"Tessera Sanitaria non valida \n");
        exit(1);
  }

  if((strcmp(argv[3],"V")!=0)&&(strcmp(argv[3],"I")!=0)){
        fprintf(stderr,"Opzione non Valida \n");
        exit(1);
  }

  servaddr.sin_family = AF_INET;
  servaddr.sin_port   = htons(1025);
  servaddr.sin_addr.s_addr = inet_addr(argv[1]);

  if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    fprintf(stderr,"socket error\n");
    exit (1);
  }

  if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) < 0) {
    fprintf(stderr,"inet_pton error for %s\n", argv[1]);
    exit (1);
  }

  if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
    fprintf(stderr,"connect error\n");
    exit(1);
  }

  strcpy(gp_send.TS,argv[2]);

  if(strcmp(argv[3],"V")==0){
    gp_send.option = 4;
  }
  else{
    if(strcmp(argv[3],"I")==0){
      gp_send.option = 3;
    }
  }
  


  FullWrite(sockfd, &gp_send, sizeof(gp_send));
  

  /*if (fputs(recvline, stdout) == EOF) {
    fprintf(stderr,"fputs error\n");
    exit(1);
  }*/
  

  close(sockfd);
  exit(0);
}
