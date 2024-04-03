#include <sys/types.h> /* predefined types */
#include <unistd.h> /* include unix standard library */
#include <arpa/inet.h> /* IP addresses conversion utililites */
#include <sys/socket.h> /* socket library */
#include <stdio.h> /* include standard I/O library */
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <semaphore.h>
#include <fcntl.h>           /* For O_* constants utlizzato per semaphore.h */
#include <sys/stat.h>
#include <errno.h>
#include "fullwrapped.h"
#define SCADENZASEIMESI 15778800 /*VALIDITA GRENN PASS VACCINALE in secondi*/



struct green_pass {
  char TS[21];
  time_t Scad;
  int validita;
  int option;
};

int main (int argc , char *argv[])
{
    /* var Socket Server & client  */
    int list_fd,conn_fd,serverV_fd;
    int i;
    struct sockaddr_in CV_add,client,serverV_add;
    socklen_t len_c;
    pid_t pid;
  
    struct green_pass in_gp;

    
    /* SOCKET, BIND E LISTEN di CENTROVACCINALE */

    CV_add.sin_family=AF_INET;
    CV_add.sin_addr.s_addr=htonl(INADDR_ANY);
    CV_add.sin_port=htons(1026);

    if( (list_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ){
         perror("socket"); exit(-1);
    } 

    if ( bind(list_fd, (struct sockaddr *) &CV_add, sizeof(CV_add))  < 0 ){
        perror("error bind"); exit(-1);
    }

    if( listen(list_fd, 1024)< 0){
        perror("error listen"); exit(-1);
    }

    while(1){

        len_c = sizeof(client);

        conn_fd = accept(list_fd, (struct sockaddr*)&client, &len_c);
        

        if((pid= fork())<0){
            perror (" fork error ");
            exit ( -1);
        }

        if(pid==0){
            close(list_fd);
            
            FullRead(conn_fd,&in_gp,sizeof(in_gp)); // LETTURA GREEN PASS/RICHIESTA

            serverV_add.sin_family = AF_INET;
            serverV_add.sin_port   = htons(1024);
            serverV_add.sin_addr.s_addr = inet_addr(argv[1]);

            if ( (serverV_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
                fprintf(stderr,"socket error\n");
                exit (1);
            }

            if (inet_pton(AF_INET, argv[1], &serverV_add.sin_addr) < 0) {
                fprintf(stderr,"inet_pton error for %s\n", argv[1]);
                exit (1);
            }

            if (connect(serverV_fd, (struct sockaddr *) &serverV_add, sizeof(serverV_add)) < 0) {
                fprintf(stderr,"connect error\n");
                exit(1);
            }
            else{

                printf("connesso al server \n");
            }

            printf("TS %s option %d\n",in_gp.TS,in_gp.option);
            in_gp.Scad = time(NULL)+ SCADENZASEIMESI;
            FullWrite(serverV_fd, &in_gp, sizeof(in_gp));

            close(conn_fd);
            close(serverV_fd);
            exit (0);
        }
        else { 
            close ( conn_fd );
        }
    }

    exit (0);
}


