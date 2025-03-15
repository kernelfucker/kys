/* kys - real-time peer-to-peer program

   kys.c

   written by kernelfucker
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>. */

// this is just a example))
// examples;
// shell-0 - $ ./kys 4000 127.0.0.1 4001
// shell-1 - $ ./kys 4001 127.0.0.1 4000

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define BUFFER_SIZE 1024

int sockfd;
struct sockaddr_in peer_addr;

void *recv_thread(void *arg)
{
    char buffer[BUFFER_SIZE];
    struct sockaddr_in src_addr;
    socklen_t addrlen = sizeof(src_addr);
    while (1) {
        ssize_t bytes = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 0,
                                 (struct sockaddr *)&src_addr, &addrlen);
        if (bytes < 0) {
            perror("recvfrom");
            continue;
        }
        buffer[bytes] = '\0';
        printf("[peer] %s\n", buffer);
        printf(": ");
        fflush(stdout);
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    if (argc != 4) {
        fprintf(stderr, "usage: %s <my_port> <peer_ip> <peer_port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int my_port = atoi(argv[1]);
    char *peer_ip = argv[2];
    int peer_port = atoi(argv[3]);

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in my_addr;
    memset(&my_addr, 0, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_addr.s_addr = INADDR_ANY;
    my_addr.sin_port = htons(my_port);

    if (bind(sockfd, (const struct sockaddr *)&my_addr, sizeof(my_addr)) < 0) {
        perror("bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    memset(&peer_addr, 0, sizeof(peer_addr));
    peer_addr.sin_family = AF_INET;
    if (inet_aton(peer_ip, &peer_addr.sin_addr) == 0) {
        fprintf(stderr, "invalid peer ip address: %s\n", peer_ip);
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    peer_addr.sin_port = htons(peer_port);

    pthread_t tid;
    if (pthread_create(&tid, NULL, recv_thread, NULL) != 0) {
        perror("pthread_create");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    char send_buf[BUFFER_SIZE];
    printf("welcome to kys. type your messages and press enter to send.\n");
    printf(": ");
    fflush(stdout);
    while (1) {
        if (!fgets(send_buf, BUFFER_SIZE, stdin)) {
            perror("fgets");
            break;
       }

        size_t len = strlen(send_buf);
        if (len > 0 && send_buf[len - 1] == '\n') {
            send_buf[len - 1] = '\0';
        }
        ssize_t sent = sendto(sockfd, send_buf, strlen(send_buf), 0,
                              (struct sockaddr *)&peer_addr, sizeof(peer_addr));
        if (sent < 0) {
            perror("sendto");
        }

        printf(": ");
        fflush(stdout);
    }

    pthread_cancel(tid);
    pthread_join(tid, NULL);
    close(sockfd);
    return 0;
}
