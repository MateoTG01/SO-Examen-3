#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <pthread.h>

#define SERVER_KEY_PATHNAME .server_key_path.txt
#define PROJECT_ID 'M'

typedef struct message_text {
    int qid;
    char buffer1 [256];
}message_text;

typedef struct message {
    long message_type;
    message_text message_text;
}message;

key_t server_queue_key;
int server_qid, my_qid;

void servidor_ask (void *pargs) {
    message server_message;
    while (1) {
        printf("Servidor %s\n", server_message.message_text.buffer1);
    }
}

void servidor_list (void *pargs) {
    message server_message;
    while (1) {
        printf("Servidor %s\n", server_message.message_text.buffer1);
    }
}

void servidor_aux (void *pargs) {
    message server_message;
    while (1) {
        printf("server %s/n", server_message.message_text.buffer1);
    }
}
void servidor_trigger (void *pargs) {
    message server_message;
    while (1) {
        printf("Servidor %s", server_message.message_text.buffer1);
        printf(" se encuentra en linea\n");
    }
}

void servidor_remover_evento (void *pargs) {
    message server_message;
    while (1) {
        printf("Servidor %s", server_message.message_text.buffer1);
        printf(" ha sido removido\n");
    }
}

void servidor_close (void *pargs) {
    message server_message; 
    printf ("Cerrando servidor.../n");
    printf ("Cerrando cliente.../n");
    exit (EXIT_SUCCESS);
}


int main (int argc, char argv) {
    message client_message, server_message;
    char *buffer1[8];
    char *buffer2[16];
    char *sep = " ";
    char *sub = "sub";
    char *ask = "ask";
    char *list = "list";
    char *cerrar = "exit";
    char *unsub = "unsub";
    char *access_denied = "denied";
    char *token_command;
    client_message.message_type = 1;
    client_message.message_text.qid = my_qid;
    printf("Cliente conectando con servidor\n");
    sprintf (buffer2, "%s", "Cliente " );
    sprintf (buffer1, "%d", my_qid);
    strcat  (buffer2, buffer1);
    strcat  (buffer2, " esta conectando con servidor");
    stpcpy  (client_message.message_text.buffer1, buffer2);
   
    printf ("Servidor %s\n", server_message.message_text.buffer1);
    if (strncmp (server_message.message_text.buffer1, access_denied, 6) == 0) {
        printf ("Servidor lleno\n");
        exit (EXIT_SUCCESS);
    }

    pthread_t threadID_servidor_ask;
    pthread_t threadID_servidor_aux;
    pthread_t threadID_servidor_list;
    pthread_t threadID_servidor_close;
    pthread_t threadID_servidor_trigger;
    pthread_t threadID_servidor_remover_evento;

    pthread_create (&threadID_servidor_ask, NULL, servidor_ask, NULL);
    pthread_create (&threadID_servidor_aux, NULL, servidor_aux, NULL);
    pthread_create (&threadID_servidor_list, NULL, servidor_list, NULL);
    pthread_create (&threadID_servidor_close, NULL, servidor_close, NULL);
    pthread_create (&threadID_servidor_trigger, NULL, servidor_trigger, NULL);
    pthread_create (&threadID_servidor_remover_evento, NULL, servidor_remover_evento, NULL);

    while (fgets (client_message.message_text.buffer1, 256, stdin)) {
        char auxBuffer[256];
        strcpy(auxBuffer, client_message.message_text.buffer1);
        token_command  = strtok (auxBuffer, sep);

        if (strcmp (token_command, sub) == 0){           
            client_message.message_type = 2;}
        else if (strcmp (token_command, unsub) == 0){    
            client_message.message_type = 2;}
        else if (strcmp (token_command, ask) == 0){      
            client_message.message_type = 3;}
        else if (strcmp (token_command, list) == 0){     
            client_message.message_type = 4;}
        else if (strcmp (token_command, cerrar) == 0) {
            exit (EXIT_SUCCESS);
            break;
        }
    }
    printf ("Cliente cerrado");

    exit (EXIT_SUCCESS);
}