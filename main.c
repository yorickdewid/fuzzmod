/**
 * Copyright (C) 2017 Quenza Inc.
 * All Rights Reserved
 *
 * This file is part of the Fuzzmod
 *
 * Content can not be copied and/or distributed without the express
 * permission of the author.
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <unistd.h>
#include <fcntl.h>

#include <arpa/inet.h>
#include <netdb.h>

static char *host = NULL;

struct http_response {
    char version;
    short code;
    char *redirect;
};

void data_block(char buffer[], size_t sz) {
    int fd = open("/dev/urandom", O_RDONLY);
    size_t readsz = 0;
    while (readsz < sz) {
        ssize_t result = read(fd, buffer + readsz, sz - readsz);
        if (result < 0)
            continue;
        readsz += result;
    }
    close(fd);
}

void parse_header(char buffer[]) {
    puts(buffer);
}

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

    FILE *fp = fopen("urllist.txt", "r");
    if (!fp) {
        perror("fopen()");
        return -1;
    }

    char *line = NULL; size_t len = 0;
    while ((getline(&line, &len, fp)) > 0) {
        char sndbuf[256];
        const char *buf = "GET /%s HTTP/1.1\r\nHost: %s\r\nUser-Agent: Fuzzmod/0.1\r\nConnection: Keep-Alive\r\n\r\n";

        data_block(sndbuf, 256);
        snprintf(sndbuf, 256, buf, line, host);

        printf("Fetching: /%s -> \n", line);

        send(sockfd, sndbuf, 256, 0);
        if (recv(sockfd, recvBuff, 1024, 0) < 0)
            break;

        parse_header(recvBuff);

        // puts(recvBuff);
        // while ((n = recv(sockfd, recvBuff, 1024, 0)) > 0) {
        //     recvBuff[n] = 0;
        //     if (fputs(recvBuff, stdout) == EOF)
        //         perror("fputs()");
        // }
    }

    fclose(fp);
    return 0;
}

void usage(const char *progname) {
    printf("%s is an network service fuzzer\n\n", progname);
    printf("Usage:\n  %s [OPTION]... HANDLE\n\n", progname);
    printf("General options:\n");
    printf("  -H, --http             http mode\n");
    printf("  -V, --verbose          verbose output\n");
    printf("  -v, --version          print version\n");
    printf("  -h, --help             this help\n");
    printf("\nHTTP options:\n");
    printf("      --hide-agent       hide user agent\n");
    printf("  -f, --uri-file=FILE    read uri requests from file\n");
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
        if (!host)
            host = argv[index];
    }

    if (!host) {
        usage(argv[0]);
        return 1;
    }

    fuzz_start();
    return 0;
}
