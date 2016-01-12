#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define BUFF_SIZE 1024

char buffer[BUFF_SIZE];
int pos = 0;
char *ptr;
int remaining = 0;

int read_line(int fd, char *str) {
    int len;
    while (1) {
        while (remaining > 0) {
            remaining -= 1;
            if (*ptr == '\n') {
                str[pos] = '\0';
                len = pos;
                pos = 0;
                return len;
            }

            if (*ptr != '\r') {
                str[pos] = *ptr;
                pos += 1;
            }

            ptr += 1;
        }

        remaining = read(fd, buffer, BUFF_SIZE);
        if(remaining <= 0) {
            return remaining;
        }
        ptr = buffer;
    }
}

int main() {
    socklen_t clilen;
    int portno = 8090;
    int sockfd, newsockfd;
    int pid;
    int rc;
    struct sockaddr_in serv_addr, cli_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("ERROR opening server socket");
        exit(1);
    }

    printf("df: %d\n", sockfd);

    // Initialize socket structure.  Standard Linux/Unix server socket
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    // Bind server socket
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR binding server socket");
        exit(1);
    }

    // Listen for HTTP clients wanting SSE RFID event feed
    rc = listen(sockfd, 5);
    if (rc) {
        perror("ERROR listening on server socket");
        exit(1);
    }
    clilen = sizeof(cli_addr);

    while (1) {
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        fprintf(stderr, "connection accepted.\n");

        if (newsockfd < 0) {
            perror("ERROR on accept");
            exit(1);
        }

        pid = fork();
        if (pid < 0) {
            perror("ERROR on fork");
            exit(1);
        }

        // Reap zombine children
        if (signal(SIGCHLD, SIG_IGN) == SIG_ERR) {
            perror(0);
            exit(1);
        }

        if (pid == 0) {
            char str[256];
            int read;
            while (1) {
                fprintf(stderr, "waiting data...\n");
                read = read_line(newsockfd, str);
                fprintf(stderr, "read: %d\n", read);

                if (read > 0) {
                    fprintf(stderr, "received: %s\n", str);
                }

                if (read <= 0  || strcmp(str, "exit") == 0) {
                    if(read < 0) {
                        perror("error");
                    }
                    fprintf(stderr, "exiting...\n");
                    break;
                }
            }
            // Cleanup
            close(newsockfd);
            return 0;
        } else {
            close(newsockfd);
        }
    }
    return 0;
}