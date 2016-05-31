#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

#define BUFF_SIZE                   50
#define INVALID_BARCODE_MESSAGE     "Barcode invalid or not in stock."
#define INVALID_PAYMENT_MESSAGE     "Payment not enough or invalid."
#define BILL_MESSAGE                "Amount to pay: "

#define WAFFLE                      11111
#define SPINACH                     22222
#define MEAT                        33333
#define MILK                        44444
#define CHEESE                      55555
#define SHIRT                       66666
#define BALL                        77777
#define DESK                        88888
#define ELECTRONICS                 99999
#define CUCUMBER                    73737

#define WAFFLE_PRICE                3
#define SPINACH_PRICE               2
#define MEAT_PRICE                  10
#define MILK_PRICE                  7
#define CHEESE_PRICE                6
#define SHIRT_PRICE                 13
#define BALL_PRICE                  31
#define DESK_PRICE                  60
#define ELECTRONICS_PRICE           220
#define CUCUMBER_PRICE              2
#define ONION_PRICE                 1

//the thread function
void *connection_handler(void *);
void match_barcode(int*, char*, char*);
void bill_response(int, char*);
int calc_change(int*, char*);

int main(int argc , char *argv[])
{
    int socket_desc , client_sock , client_length , *new_sock;
    struct sockaddr_in server , client;
    pthread_t client_thread;

    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        printf("Could not create socket");
    }

    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( 8888 );

    //Bind
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("Bind failed. Error");
        return 1;
    }
    puts("System initialized successfully.");

    //Listen
    listen(socket_desc , 20);

    //Accept and incoming connection
    puts("Waiting for incoming connections...");
    client_length = sizeof(struct sockaddr_in);
    while((client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&client_length)))
    {
        puts("Connection accepted.");

        new_sock = malloc(1);
        *new_sock = client_sock;

        if( pthread_create( &client_thread , NULL ,  connection_handler , (void*) new_sock) < 0)
        {
            perror("Could not create thread.");
            return 1;
        }

        puts("Handler assigned.");
    }

    if (client_sock < 0)
    {
        perror("Client connection failed.");
        return 1;
    }

    return 0;
}

void *connection_handler(void *socket_desc)
{
    //Get the socket descriptor
    char buffer[BUFF_SIZE]          = {'\0'};
    char curr_message[BUFF_SIZE]    = {'\0'};
    int sock                        = *(int*)socket_desc;
    int sum                         = 0;
    int phase                       = 0;
    int read_size;

    //Send some messages to the client
    strcpy(buffer, "Connection to system created successfully.");
    write(sock , buffer , strlen(buffer) + 1);
    strcpy(buffer, "\0");

    //Receive a message from client
    while( (read_size = recv(sock , curr_message , BUFF_SIZE , 0)) > 0 )
    {
        //Send the message back to client
        if(phase == 0 && strncmp(curr_message, "\n", 1) != 0)
        {
            match_barcode(&sum, curr_message, buffer);

            write(sock , buffer , strlen(buffer) + 1);
        }
        else if(phase == 0)
        {
            phase++;
            puts("Proceeding to second phase..");
            write(sock , curr_message , strlen(curr_message) + 1);
        }
        else if(phase > 0)
        {
            int change = calc_change(&sum, curr_message);
            if(change >= 0)
            {
                sprintf(buffer, "%d", change);
            }
            else
            {
                strcpy(buffer, INVALID_PAYMENT_MESSAGE);
            }
            write(sock, buffer, BUFF_SIZE);
        }

        strcpy(buffer, "\0");
    }

    if(read_size == 0)
    {
        puts("Client disconnected.");
        fflush(stdout);
    }
    else if(read_size == -1)
    {
        perror("Client connection failed.");
    }

    //Free the socket pointer
    free(socket_desc);

    return 0;
}

void match_barcode(int* sum, char* curr_message, char* buffer)
{
    int barcode = 0;

    sscanf(curr_message, "%d", &barcode);
    switch(barcode)
    {
        case WAFFLE:
        {
            *sum += WAFFLE_PRICE;
            bill_response(*sum, buffer);
            break;
        }
        case SPINACH:
        {
            *sum += SPINACH_PRICE;
            bill_response(*sum, buffer);
            break;
        }
        case MEAT:
        {
            *sum += MEAT_PRICE;
            bill_response(*sum, buffer);
            break;
        }
        case CUCUMBER:
        {
            *sum += CUCUMBER_PRICE;
            bill_response(*sum, buffer);
            break;
        }
        case ELECTRONICS:
        {
            *sum += ELECTRONICS_PRICE;
            bill_response(*sum, buffer);
            break;
        }
        case DESK:
        {
            *sum += DESK_PRICE;
            bill_response(*sum, buffer);
            break;
        }
        case SHIRT:
        {
            *sum += SHIRT_PRICE;
            bill_response(*sum, buffer);
            break;
        }
        case BALL:
        {
            *sum += BALL_PRICE;
            bill_response(*sum, buffer);
            break;
        }
        case MILK:
        {   
            *sum += MILK_PRICE;
            bill_response(*sum, buffer);
            break;
        }
        case CHEESE:
        {
            *sum += CHEESE_PRICE;
            bill_response(*sum, buffer);
            break;
        }
        default:
        {
            strcpy(buffer, INVALID_BARCODE_MESSAGE);
        }
    }
}

void bill_response(int sum, char* buffer)
{
    char bill[10] = {'\0'};
    strcpy(buffer, BILL_MESSAGE);
    sprintf(bill,"%d", sum);
    strcat(buffer, bill);
    puts(buffer);
}

int calc_change(int* sum, char* curr_message)
{
    int pay_amount;
    sscanf(curr_message, "%d", &pay_amount);
    return pay_amount - *sum;
}
