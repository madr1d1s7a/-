#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h> 

#define BUFF_SIZE                   50
#define CLOSE_CONNECTION_MESSAGE    "Closing server connection..\n"

int main(int argc , char *argv[])
{
    char buffer[BUFF_SIZE]  = {'\0'};
    int phase               = 0;
    int sock, read_size;
    struct sockaddr_in server;

    //Create socket
    sock = socket(AF_INET , SOCK_STREAM , 0);
    if (sock == -1)
    {
        printf("Could not create socket");
    }

    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons( 8888 );

    //Connect to remote server
    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("Connect failed. Error");
        return 1;
    }

    //communicating with server
    while(1)
    {
        //Receive a message from the server
        if((read_size = recv(sock , buffer , BUFF_SIZE , 0)) < 0)
        {
            puts("recv failed");
            return 1;
        }
        else if(read_size == 0)
        {
            puts("Server shut down.");
            fflush(stdout);
            return 1;
        }

        if(phase == 0 && strncmp(buffer, "\n", 1) != 0)         
        {   
            puts(buffer);
            strcpy(buffer, "\0");

            printf("Enter barcode or press RETURN to proceed: ");
            fgets(buffer, BUFF_SIZE, stdin);

                //Send some data
            if( send(sock , buffer , strlen(buffer) +1, 0) < 0)
            {
                puts("Send failed.");
                return 1;
            }
        }
        else if(phase == 0)
        {
            int result;
            int payment;

            phase++;

            printf("Enter provided amount of money: ");
            while((result = scanf("%d", &payment)) == EOF || payment <= 0 || result == 0)
            {
                if (result == 0 || payment <= 0)
                {
                    while (fgetc(stdin) != '\n');
                }
                printf("Invalid input. Try again: ");
            }

            sprintf(buffer, "%d", payment);
                //Send some data
            if( send(sock , buffer, BUFF_SIZE, 0) < 0)
            {
                puts("Send failed.");
                return 1;
            }
        }
        else if(phase > 0)
        {
            printf("The change is: %s\n%s", buffer, CLOSE_CONNECTION_MESSAGE);
            break;
        }
        strcpy(buffer, "\0");
    }

    close(sock);

    return 0;
}