#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER 4096

void send_response(int socket, const char *header, const char *content) {
    char response[BUFFER];
    snprintf(response, sizeof(response), "%s\r\n%s", header, content);
    send(socket, response, strlen(response), 0);
}

void send_file_response(int socket, const char *file_path, const char *content_type) {
    int file = open(file_path, O_RDONLY);
    if (file < 0) {
        const char *error_response = "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\n\r\n"
                                     "<html><body><h1>404 Not Found</h1></body></html>";
        send(socket, error_response, strlen(error_response), 0);
        return;
    }

    struct stat file_stat;
    fstat(file, &file_stat);

    char header[BUFFER];
    snprintf(header, sizeof(header), "HTTP/1.1 200 OK\r\nContent-Type: %s\r\nContent-Length: %ld\r\n\r\n", content_type, file_stat.st_size);
    send(socket, header, strlen(header), 0);

    char file_buffer[BUFFER];
    ssize_t read_bytes;
    while ((read_bytes = read(file, file_buffer, sizeof(file_buffer))) > 0) {
        send(socket, file_buffer, read_bytes, 0);
    }

    close(file);
}

int main(int argc, char const *argv[]) {
    int s, newSock;
    struct sockaddr_in address;
    socklen_t addr_size = sizeof(address);
    char buffer[BUFFER];

    if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    printf("Socket creation successful\n");

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr("127.0.0.1");
    address.sin_port = htons(PORT);
    memset(address.sin_zero, '\0', sizeof address.sin_zero);

    if (bind(s, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failure");
        close(s);
        exit(EXIT_FAILURE);
    }
    printf("Bind successful\n");

    if (listen(s, 3) < 0) {
        perror("Error listening");
        close(s);
        exit(EXIT_FAILURE);
    }
    printf("Listening\n");

    while ((newSock = accept(s, (struct sockaddr*)&address, &addr_size)) > 0) {
        printf("Accepting...\n");

        ssize_t received_bytes = recv(newSock, buffer, BUFFER, 0);
        if (received_bytes > 0) {
            buffer[received_bytes] = '\0';
            printf("Buffer val: %s\n", buffer);

            char *method = strtok(buffer, " ");
            char *uri = strtok(NULL, " ");

            if (method != NULL && uri != NULL) {
                printf("Requested URI: %s\n", uri);

                if (strcmp(method, "GET") == 0) {
                    if (strcmp(uri, "/") == 0) {
                        const char *response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"
                                               "<html><body><h1>Home Page</h1></body></html>";
                        send_response(newSock, "HTTP/1.1 200 OK", response);
                    } else if (strcmp(uri, "/test") == 0) {
                        const char *response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"
                                               "<html><body><h1>Test Page</h1></body></html>";
                        send_response(newSock, "HTTP/1.1 200 OK", response);
                    } else if (strcmp(uri, "/another") == 0) {
                        const char *response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"
                                               "<html><body><h1>Another Page</h1></body></html>";
                        send_response(newSock, "HTTP/1.1 200 OK", response);
                    } else if (strncmp(uri, "/images/", 8) == 0) {
                        printf("Lol:%s\n", uri);
                        char *image_name = strtok(uri + 8, "/"); 
                        if (image_name != NULL) {
                            char file_path[BUFFER];
                            snprintf(file_path, sizeof(file_path), "%s", image_name);

                            const char *content_type = "image/jpeg";
                            if (strstr(image_name, ".png") != NULL) {
                                content_type = "image/png";
                            } else if (strstr(image_name, ".gif") != NULL) {
                                content_type = "image/gif";
                            }

                            send_file_response(newSock, file_path, content_type);
                        }
                    } else {
                        const char *response = "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\n\r\n"
                                               "<html><body><h1>404 Not Found</h1></body></html>";
                        send_response(newSock, "HTTP/1.1 404 Not Found", response);
                    }
                } else {
                    const char *response = "HTTP/1.1 405 Method Not Allowed\r\nContent-Type: text/html\r\n\r\n"
                                           "<html><body><h1>405 Method Not Allowed</h1></body></html>";
                    send_response(newSock, "HTTP/1.1 405 Method Not Allowed", response);
                }
            } else {
                printf("Invalid HTTP request format.\n");
            }
        } else {
            printf("Nothing received\n");
        }
        close(newSock);
    }

    if (newSock < 0) {
        perror("Accept failed");
    }

    close(s);
    return 0;
}
