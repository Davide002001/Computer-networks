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
    int list_fd,conn_fd;
    int i;
    struct sockaddr_in serverV_add,client;
    socklen_t len_c;

    /* var Thread*/
    time_t timeval;
    sem_t* sem_phore = sem_open("semaphore",O_CREAT,O_RDWR,1); 
    pid_t pid;

    /* var FILE Green Pass*/
    FILE *file = fopen("file","rb+");
  
    struct green_pass in_gp, temp_gp;

    
    /* SOCKET, BIND E LISTEN di ServerV */

    serverV_add.sin_family=AF_INET;
    serverV_add.sin_addr.s_addr=htonl(INADDR_ANY);
    serverV_add.sin_port=htons(1024);

    if( (list_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ){
         perror("socket"); exit(-1);
    } 

    if ( bind(list_fd, (struct sockaddr *) &serverV_add, sizeof(serverV_add))  < 0 ){
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
            printf("NUOVO CLIENT\n");
           
            FullRead(conn_fd,&in_gp,sizeof(in_gp)); // LETTURA GREEN PASS/RICHIESTA


            switch (in_gp.option)
            {


                case 1:    // CentroVaccinale operazione-> registrazione sul file del nuovo/aggiornato green pass
                    
                    fseek(file,0,SEEK_SET);
                    printf("CASO 1: \n");
                    int gp_trovato = 0;

                    while(fread(&temp_gp,sizeof(struct green_pass),1,file) == 1){
                        
                        if(strcmp(temp_gp.TS, in_gp.TS) == 0){
                            
                            fseek(file,-(sizeof(struct green_pass)),SEEK_CUR);
                            gp_trovato=1;
                            sem_wait(sem_phore); /* DECREMENTA IL VALORE DI access DA 1 A 0 (IN QUESTO MODO AUMENTA LA PRIORITA DEL THREAD) */
                            
                            gp_trovato=1;
                            in_gp.validita = 1;
                            fwrite(&in_gp,sizeof(struct green_pass),1,file);
                            printf(" P2 Ts %s Scad %.24s  validita %d \r\n",in_gp.TS,ctime(&in_gp.Scad),in_gp.validita);
                            sem_post(sem_phore); /* AUMENTA IL VALORE DI access DA 0 A 1 (IN QUESTO MODO DIMINUSCE LA PRIORITA DEL THREAD) */
                            break;
                        }
                    }

                    
                    if(gp_trovato==0){
                        fseek(file,0,SEEK_END);
                        sem_wait(sem_phore); // DECREMENTA IL VALORE DI access DA 1 A 0 (IN QUESTO MODO AUMENTA LA PRIORITA DEL THREAD) 
                        in_gp.validita = 1;
                        fwrite(&in_gp,sizeof(struct green_pass),1,file);
                        printf(" P1 Ts %s Scad %.24s validita %d\r\n",in_gp.TS,ctime(&in_gp.Scad),in_gp.validita);
                        sem_post(sem_phore);
                    }

                break;

                case 2:    // ClientS output-> risposta al client se il green pass è valido o meno
                    printf("CASO 2: ");
                    fseek(file,0,SEEK_SET);
                    gp_trovato = 0;

                    while(fread(&temp_gp,sizeof(struct green_pass),1,file) == 1){
                        
                        if(strcmp(temp_gp.TS, in_gp.TS) == 0){

                            sem_wait(sem_phore); /* DECREMENTA IL VALORE DI access DA 1 A 0 (IN QUESTO MODO AUMENTA LA PRIORITA DEL THREAD) */
                            FullWrite(conn_fd,&temp_gp,sizeof(temp_gp)); 
                            printf(" Ts %s Scad %.24s validita %d\r\n",temp_gp.TS,ctime(&temp_gp.Scad),temp_gp.validita);
                            sem_post(sem_phore); /* AUMENTA IL VALORE DI access DA 0 A 1 (IN QUESTO MODO DIMINUSCE LA PRIORITA DEL THREAD) */
                            break;
                        }
                    }

                    if(gp_trovato==0){
                        sem_wait(sem_phore); /* DECREMENTA IL VALORE DI access DA 1 A 0 (IN QUESTO MODO AUMENTA LA PRIORITA DEL THREAD) */
                        in_gp.validita=0;      
                        FullWrite(conn_fd,&in_gp,sizeof(in_gp)); 
                        printf("Ts %s ",in_gp.TS);
                        sem_post(sem_phore);
                    }
                    

                break;
                
                case 3:    // ClientT operazione-> Invalidamento greenPass da contagio
                    printf("CASO 3: \n");
                    fseek(file,0,SEEK_SET);
                    gp_trovato=0;

                    while(fread(&temp_gp,sizeof(struct green_pass),1,file) == 1){
                        
                        if(strcmp(temp_gp.TS, in_gp.TS) == 0){
                            
                            fseek(file,-(sizeof(struct green_pass)),SEEK_CUR);
                            gp_trovato=1;
                            sem_wait(sem_phore); /* DECREMENTA IL VALORE DI access DA 1 A 0 (IN QUESTO MODO AUMENTA LA PRIORITA DEL THREAD) */
                            in_gp.validita = 0;
                            fwrite(&in_gp,sizeof(struct green_pass),1,file);
                            printf("Ts %s validita %d INVALIDATO \r\n",in_gp.TS,in_gp.validita);
                            sem_post(sem_phore); /* AUMENTA IL VALORE DI access DA 0 A 1 (IN QUESTO MODO DIMINUSCE LA PRIORITA DEL THREAD) */
                            
                            break;
                        }
                    }

                    if(gp_trovato==0){
                        printf("TS NON PRESENTE (invalidamento)");
                    }
                    
                break;

                case 4:    // ClientT operazione-> Ripristino greenPass da guarigione
                    printf("CASO 4: \n");
                    fseek(file,0,SEEK_SET);
                    gp_trovato=0;

                    while(fread(&temp_gp,sizeof(struct green_pass),1,file) == 1){
                        
                        if(strcmp(temp_gp.TS, in_gp.TS) == 0){
                            
                            fseek(file,-(sizeof(struct green_pass)),SEEK_CUR);
                            gp_trovato=1;
                            sem_wait(sem_phore); /* DECREMENTA IL VALORE DI access DA 1 A 0 (IN QUESTO MODO AUMENTA LA PRIORITA DEL THREAD) */
                            in_gp.validita = 1;
                            in_gp.Scad = time(NULL) + SCADENZASEIMESI;
                            fwrite(&in_gp,sizeof(struct green_pass),1,file);
                            printf("Ts %s validita %d GUARITO\r\n",in_gp.TS,in_gp.validita);
                            sem_post(sem_phore); /* AUMENTA IL VALORE DI access DA 0 A 1 (IN QUESTO MODO DIMINUSCE LA PRIORITA DEL THREAD) */
                            
                            break;
                        }
                    }
                    if(gp_trovato==0){
                        printf("TS NON PRESENTE (guarigione)");
                    }

                    
                break;
      
                
                
            }

            

            
            fclose(file);
            sem_unlink("semaphore");
            close(conn_fd);

            exit (0);
        }
        else { 
            close ( conn_fd );
        }
    }

    exit (0);
}



