#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/wait.h>

#define PORT 12345
#define BUFFER_SIZE 1024

int main() {
    // Check if already in background (child) or not
    if (getppid() != 1) { // If parent is not init (PID 1), we're in foreground
        pid_t pid = fork();

        if (pid > 0) {
            printf("Launched Educational_Backdoor in background (PID %d)\n", pid);
            exit(0); // Parent exits
        } else if (pid < 0) {
            perror("fork failed");
            exit(EXIT_FAILURE);
        }

        // Child (background) continues
    }

    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};
    char *welcome_msg = "Educational Safe backdoor active\n";

    // creates socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // binds to host:PORT
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; // or specific address
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 1) < 0) {
        perror("listen");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Educational_Backdoor (background) listening on port %d\n", PORT);

    // Accept 1 connection
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
        perror("accept");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    send(new_socket, welcome_msg, strlen(welcome_msg), 0);

    // receive command
    int valread = read(new_socket, buffer, BUFFER_SIZE);
    if (valread > 0) {
        buffer[valread] = '\0';
        printf("Received command: %s\n", buffer);

        // respond to commands
        if (strncmp(buffer, "ping", 4) == 0) {
            send(new_socket, "pong\n", 5, 0);
        } else {
            send(new_socket, "unknown command\n", 17, 0);
        }
    }

    close(new_socket);
    close(server_fd);
    printf("Connection closed, backdoor exiting\n");
    return 0;
}
