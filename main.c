#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <netdb.h>

static char *host = NULL;

int fuzz_start() {
    int sockfd = 0, n = 0;
    char recvBuff[1024];
    struct sockaddr_in serv_addr; 

    memset(recvBuff, '\0',sizeof(recvBuff));
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket()");
        return -1;
    } 

    memset(&serv_addr, '\0', sizeof(serv_addr)); 
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(80); 

    struct hostent *he;
    if ((he = gethostbyname(host)) < 0) {
        perror("gethostbyname()");
        return -1;
    }

    memcpy(&serv_addr.sin_addr, he->h_addr_list[0], he->h_length);

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connect()");
        return -1;
    } 

    int i;
    for (i=0; i<100; ++i) {
        char _woe[128];
        const char *buf = "GET /%s HTTP/1.1\r\nHost: %s\r\nUser-Agent: Fuzzmod/0.1\r\nConnection: close\r\n\r\n";

        snprintf(_woe, 128, buf, "woei", host);

        printf("Fetching: %s\n", _woe);

        send(sockfd, _woe, strlen(_woe), 0);
        recv(sockfd, recvBuff, 1024, 0);
        // while ((n = recv(sockfd, recvBuff, 1024, 0)) > 0) {
        //     recvBuff[n] = 0;
        //     if (fputs(recvBuff, stdout) == EOF)
        //         perror("fputs()");
        // }
    }

    return 0;
}

void usage(const char *progname) {
    printf("%s is an network fuzzer\n\n", progname);
    printf("Usage:\n  %s [OPTION]... URL\n\n", progname);
    printf("General options:\n");
    printf("  -H, --http             http mode\n");
    printf("  -V, --verbose          verbose output\n");
    printf("  -v, --version          print version\n");
    printf("  -h, --help             this help\n");
}

int main(int argc, char *argv[]) {
    int aflag = 0;
    int bflag = 0;
    char *cvalue = NULL;
    int index;
    int c;

    opterr = 0;

    while ((c = getopt(argc, argv, "abc:")) != -1)
        switch (c) {
            case 'a':
                aflag = 1;
                break;
            case 'b':
                bflag = 1;
                break;
            case 'c':
                cvalue = optarg;
                break;
            case 'h':
            case '?':
            default: {
                usage(argv[0]);
                return 1;
            }
        }

    for (index = optind; index < argc; index++) {
        host = argv[index];
        break;
    }

    if (!host) {
        usage(argv[0]);
        return 1;
    }

    fuzz_start();

    return 0;
}
