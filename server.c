#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <pthread.h>

#define SERVER_KEY_PATHNAME "./server_key_path.txt"
#define PROJECT_ID 'M'

typedef struct message_text {
    int qid;
    char buffer1 [256];
} message_text;

typedef struct message {
    long message_type;
    message_text message_text;
}message;

typedef struct event {
    char nombre_evento[64];
    int interesados;
    int *presentes;
    int capacidad_maxima;
} event;

key_t msg_queue_key;
struct event *events;
int qid;
int registro_clientes[128];
int cant_eventos;
int cant_clientes;


void *conexion_cliente (void *parg) {
    message message_client, message_server;
    cant_clientes = 0;
    while (1) {
        printf ("Servidor %s\n", message_client.message_text.buffer1);
        int client_qid = message_client.message_text.qid;
        message_server.message_text.qid = qid;
        message_server.message_type = 1;
        if (cant_clientes == 128 - 1){
            sprintf(message_server.message_text.buffer1, "%s", "Acceso denegado. Se ha llegado a capacidad maxima\n");
        }
        else {
            sprintf(message_server.message_text.buffer1, "%s", "Acceso concedido");
        }
        registro_clientes [cant_clientes] = message_client.message_text.qid;
        cant_clientes ++;
        printf ("Servidor ha añadido un nuevo cliente\n");
    }
}

void *cliente_sub_unsub (void *parg) {
    message message_client, message_server;
    char *token;
    char *sep = " ";
    char *sub = "sub";
    char *unsub = "unsub";

    while (1) {
        int client_qid = message_client.message_text.qid;

        token = strtok(message_client.message_text.buffer1, sep);
        if (strcmp (token, sub) == 0) {
            sprintf(message_server.message_text.buffer1, "%s", "No se ha encontrado el evento");
            token = strtok(NULL, sep);
            if (cant_eventos == 0) 
                sprintf(message_server.message_text.buffer1, "%s", "No se ha encontrado ningun evento");
            for (int i = 0; i < cant_eventos; i++) {
                if (strcmp (token, events[i].nombre_evento) == 0) {
                    if (events[i].interesados == events[i].capacidad_maxima) {
                        sprintf(message_server.message_text.buffer1, "%s", "El evento se encuentra lleno. Intente mas tarde\n");
                        break;
                    }
                    events[i].presentes[events[i].interesados] = client_qid;
                    events[i].interesados ++;
                    sprintf(message_server.message_text.buffer1, "%s", "Se ha suscrito al evento\n");
                }
            }
        }
        else if (strcmp (token, unsub) == 0) {
            token = strtok(NULL, sep);
            int pos = -1;
            int posEvent = 0;
            sprintf(message_server.message_text.buffer1, "%s", "No esta suscrito a este evento");
            for (int i = 0; i < cant_eventos; i++) {
                if (strcmp (token, events[i].nombre_evento) == 0) {
                    for (int j = 0; j < events[i].interesados; j++) {
                        if (client_qid == events[i].presentes[j]) {
                            pos = j;
                            posEvent = i;
                            break;
                        }
                    }
                    if (pos == 0) {
                        events[i].presentes[0] = 0;
                    } else {
                        for (int j = pos; j < events[i].interesados; j++) {
                            events[i].presentes[j] = events[i].presentes[j + 1];
                        }
                    }
                }
            }
            events[posEvent].interesados --;
        }

        message_server.message_text.qid = qid;
        message_server.message_type = 1;
        printf ("Se ha enviado confirmacion de respuesta al cliente %d\n", client_qid);
    }
}

void *cliente_ask (void *pargs) {
    message message_client, message_server;
    while(1) {
        int client_qid = message_client.message_text.qid;
        message_server.message_text.qid = qid;
        message_server.message_type = 5;
        for (int i = 0; i < cant_eventos; i++) {
            if (events[i].interesados < events[i].capacidad_maxima) {
                sprintf(message_server.message_text.buffer1, "%s", events[i].nombre_evento);
            }
        }
        printf ("Se ha enviado confirmacion de respuesta al cliente %d.\n", client_qid);
    }
}

void *cliente_list (void *pargs) {
    message message_client, message_server;
    while(1) {
        int client_qid = message_client.message_text.qid;
        message_server.message_text.qid = qid;

        message_server.message_type = 6;
        for (int i = 0; i < cant_eventos; i++) {
            for (int j = 0; j < events[i].interesados; j ++) {
                if (client_qid == events[i].presentes[j]) {
                    sprintf(message_server.message_text.buffer1, "%s", events[i].nombre_evento);
                }
            }
        }
    printf ("Se ha enviado confirmacion de respuesta al cliente %d.\n", client_qid);
    }
}

int main (int argc, char **argv) {
    message server_message;
    FILE *server_key_path_create = fopen(SERVER_KEY_PATHNAME,"w+");
    
    if (server_key_path_create == NULL){
        perror("open file fails: ");
        return(EXIT_FAILURE);
        
    }

    pthread_t threadID_conexion_cliente;
    pthread_t threadID_cliente_ask;
    pthread_t threadID_cliente_list;
    pthread_t threadID_cliente_sub_unsub;
    pthread_create (&threadID_conexion_cliente, NULL, conexion_cliente, NULL);
    pthread_create (&threadID_cliente_ask, NULL, cliente_ask, NULL);
    pthread_create (&threadID_cliente_list, NULL, cliente_list, NULL);
    pthread_create (&threadID_cliente_sub_unsub, NULL, cliente_sub_unsub, NULL);

    cant_eventos = 0;
    events = malloc(sizeof(event) * 128);
    printf ("Servidor habilitado\n");
    char *token;
    char *sep = " ";
    char *add = "add";
    char *list = "list";
    char *close = "exit";
    char *remove = "remove";
    char *trigger = "trigger";
    char entrada[256];

    while (fgets (entrada, 256, stdin)) {
        int length = strlen (entrada);
        if (entrada [length - 1] == '\n'){ 
            entrada [length - 1] = '\0';
        }
        token = strtok (entrada, sep);
        if (strcmp (token, add) == 0) {
            token = strtok(NULL, sep);
            events[cant_eventos].interesados = 0;
            events[cant_eventos].capacidad_maxima = 64;
            events[cant_eventos].presentes = malloc (sizeof(int) * events[cant_eventos].capacidad_maxima);
            strcpy (events[cant_eventos].nombre_evento, token);
            cant_eventos ++;
            printf("Servidor ha añadido un nuevo evento\n");
        }

        if (strcmp (token, remove) == 0) {
            int pos = 0;
            token = strtok (NULL, sep);
            for (int i = 0; i < cant_eventos; i++) {
                if (strcmp (token, events[i].nombre_evento) == 0) {
                    server_message.message_type = 4;
                    sprintf(server_message.message_text.buffer1, "%s",events[i].nombre_evento);
                    free(events[i].presentes);
                    pos = i;
                    break;
                }
            }
            for (int i = pos; i < cant_eventos; i++) {
                events[i] = events[i + 1];
                if (i == cant_eventos - 1) {
                    free (events[i].presentes);
                }
            }
            cant_eventos --;
            printf("Servidor ha removido un evento\n");
        }

        if (strcmp (token, trigger) == 0) {
            token = strtok (NULL, sep);
            for (int i = 0; i < cant_eventos; i++) {
                if (strcmp (token, events[i].nombre_evento) == 0) {
                    server_message.message_type = 3;
                    server_message.message_text.qid = qid;
                    for (int j = 0; j < events[i].interesados; j++) {
                        sprintf(server_message.message_text.buffer1, "%s", events[i].nombre_evento);
                        printf("Se ha enviado confirmacion de respuesta al cliente %d\n", events[i].presentes[j]);
                    }
                    break;
                }
            }
        }

        if (strcmp (token, list) == 0) {
            token = strtok( NULL, sep);
            for (int i = 0; i < cant_eventos; i++) {
                if (strcmp (token, events[i].nombre_evento) == 0) {
                    for (int j = 0; j < events[i].interesados; j++) {
                        printf("Servidor %s con cliente %d\n", events[i].nombre_evento, events[i].presentes[j]);
                    }
                break;
                }
            }
        }

        if (strcmp (token, close) == 0) {
            printf ("Cerrando servidor...\n");
            server_message.message_type = 2;
            server_message.message_text.qid = qid;
            for (int i = 0; i < cant_eventos; i++) { free(events[i].presentes); }
            for (int i = 0; i < cant_clientes; i++) {
                sprintf(server_message.message_text.buffer1, "Cerrando servidor...");
            }
            free(events);
        }
    }
}