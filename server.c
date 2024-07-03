#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#define PORT 8080
#define BUFFER 300
int main(int argc, char const *argv[]) {
    int s;
    int newSock;
    struct sockaddr_in address;
    socklen_t addr_size = sizeof(address);
	char* buffer=(char *)malloc(BUFFER*sizeof(char));
    // Correcting the socket creation condition
    if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    } else {
        printf("Socket creation successful\n");
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr("127.0.0.1");
    address.sin_port = htons(PORT);

    memset(address.sin_zero, '\0', sizeof address.sin_zero);

    if (bind(s, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failure");
        close(s);  // Close the socket before exiting
        exit(EXIT_FAILURE);
    } else {
        printf("Bind successful\n");
    }

    if (listen(s, 3) < 0) {
        perror("Error listening");
        close(s);
        exit(EXIT_FAILURE);
    } else {
        printf("Listening\n");
    }

    while ((newSock = accept(s, (struct sockaddr*)&address, &addr_size)) > 0) {
        printf("Accepting...\n");

        char *msg = "Simple HTTP server";
        int len = strlen(msg);
        int bytes_sent = send(newSock, msg, len, 0);

        if (bytes_sent < 0) {
            perror("Send failed");
        } else {
            printf("Message sent: %s\n", msg);
        }
		ssize_t received_bytes=recv(newSock,buffer,BUFFER,0);
        if(received_bytes>0){
        	printf("Buffer val:%s\n",buffer);
        	
        	char *method = strtok(buffer, " ");  
	        if (method != NULL) {
	            switch (method[0]) {
	                case 'G':
	                    printf("Received HTTP GET request:\n%s\n", buffer);
	                   
	                    break;
	                case 'P':
	                    printf("Received HTTP POST request:\n%s\n", buffer);
	                  
	                    break;
	             
	                default:
	                    printf("Received unsupported HTTP method:\n%s\n", buffer);
	                    break;
	            }
	        } else {
	            printf("Invalid HTTP request format.\n");
	        }
		}else{
			printf("Nothing");
		}
		close(newSock);
        free(buffer);
    }

    if (newSock < 0) {
        perror("Accept failed");
    }

    close(s);
    return 0;
}
