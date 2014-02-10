/*
 * Copyright (c) 2014 Damian Jason Lapidge
 *
 * The contents of this file are subject to the terms and conditions defined
 * within the file LICENSE.txt, located within this project's root directory.
 */

/*
 * stdio.h : stderr, printf(), fprintf()
 * stdlib.h: EXIT_FAILURE, EXIT_SUCCESS, exit()
 * unistd.h: STDIN_FILENO, STDOUT_FILENO
 * string.h: memset()
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

/*
 * sys/socket.h: AF_INET, SOCK_STREAM, SHUT_RD, SHUT_WR, socket(), connect(), 
 *               bind(), listen(), recv(), send(), shutdown()
 * arpa/inet.h : INADDR_ANY, struct sockaddr_in, htons(), htonl(), inet_addr()
 */
#include <sys/socket.h>
#include <arpa/inet.h>

/*
 * pthread.h: pthread_t, pthread_create(), pthread_cancel(), pthread_join()
 */
#include <pthread.h>

#define BUFSIZE 1024

static const char *prgname = "netbat";
static const char *version = "1.0";
static const char *authors = "Damian Jason Lapidge <grey@greydamian.org>";

/*
 * Stores parsed command-line arguments.
 */
struct options {
    char *host;
    int   port;
};

/*
 * Arguments struct for {recvsender()} function.
 */
typedef struct {
    int fd; /* file descriptor   */
    int sd; /* socket descriptor */
} receiver_sender_arg_t;

/*
 * Outputs usage information to {stderr}.
 */
void print_usage() {
    fprintf(stderr, "usage: netbat [host] <port>\n");
}

/*
 * Parses command-line arguments into their corresponding command options.
 *
 * On success, 0 is returned. On error, -1 is returned.
 */
int parse_args(struct options *opts, int argc, char *argv[]) {
    /* init options struct */
    opts->host = NULL;
    opts->port = -1;

    if (argc < 2)
        /* too few arguments */
        return -1;

    if (argc > 2)
        opts->host = argv[argc - 2];

    opts->port = atoi(argv[argc - 1]);
    if (opts->port < 1)
        /* invalid port number */
        return -1;

    return 0;
}

/*
 * Creates a TCP socket descriptor and configures it to listen to for incoming
 * connections. Takes arguments for the port number on which to listen for
 * incoming connections {port}.
 *
 * On success, returns successfully created socket descriptor. On error, 
 * returns -1.
 */
int create_servsock(int port) {
    int sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd == -1)
        return -1;

    struct sockaddr_in bindaddr;
    memset(&bindaddr, 0, sizeof(bindaddr));

    bindaddr.sin_family      = AF_INET;
    bindaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    bindaddr.sin_port        = htons(port);

    if (bind(sd, (struct sockaddr *)&bindaddr, sizeof(bindaddr)) != 0)
        return -1;

    if (listen(sd, 0) != 0)
        return -1;

    return sd;
}

/*
 * Creates a TCP  socket descriptor and connects it to a service. Takes 
 * arguments for IP address of the service {addr}, and port number of the 
 * service {port}.
 *
 * On success, returns successfully created socket descriptor. On error, 
 * returns -1.
 */
int create_clntsock(char *addr, int port) {
    int sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd == -1)
        return -1;

    struct sockaddr_in connaddr;
    memset(&connaddr, 0, sizeof(connaddr));

    connaddr.sin_family      = AF_INET;
    connaddr.sin_addr.s_addr = inet_addr(addr);
    connaddr.sin_port        = htons(port);

    if (connect(sd, (struct sockaddr *)&connaddr, sizeof(connaddr)) != 0)
        return -1;

    return sd;
}

/*
 * Reads data from a socket descriptor and then writes that data to a file 
 * descriptor. Takes arguments for a pointer to a receiver_sender_arg_t struct 
 * {arg} which has the following elements:
 *
 * {fd}: file descriptor to write to
 * {sd}: socket descriptor to read from
 *
 * Returns NULL.
 */
void *receiver(void *arg) {
    int fd = ((receiver_sender_arg_t *)arg)->fd;
    int sd = ((receiver_sender_arg_t *)arg)->sd;

    char buf[BUFSIZE];
    int rbytes = 0, wbytes = 0;

    rbytes = recv(sd, buf, BUFSIZE, 0);
    while (rbytes > 0) {
        wbytes = write(fd, buf, rbytes);
        while (wbytes < rbytes)
            wbytes += write(fd, buf + wbytes, rbytes - wbytes);
        rbytes = recv(sd, buf, BUFSIZE, 0);
    }

    shutdown(sd, SHUT_RD); /* shutdown socket for reading */

    return NULL;
}

/*
 * Reads data from a file descriptor and then writes that data to a socket 
 * descriptor. Takes arguments for a pointer to a receiver_sender_arg_t struct 
 * {arg} which has the following elements:
 *
 * {fd}: file descriptor to read from
 * {sd}: socket descriptor to write to
 *
 * Returns NULL.
 */
void *sender(void *arg) {
    int fd = ((receiver_sender_arg_t *)arg)->fd;
    int sd = ((receiver_sender_arg_t *)arg)->sd;

    char buf[BUFSIZE];
    int rbytes = 0, wbytes = 0;

    rbytes = read(fd, buf, BUFSIZE);
    while (rbytes > 0) {
        wbytes = send(sd, buf, rbytes, 0);
        while (wbytes < rbytes)
            wbytes += send(sd, buf + wbytes, rbytes - wbytes, 0);
        rbytes = read(fd, buf, BUFSIZE);
    }

    shutdown(sd, SHUT_WR); /* shutdown socket for writing */

    return NULL;
}

int main(int argc, char *argv[]) {
    struct options opts;

    if (parse_args(&opts, argc, argv) == -1) {
        print_usage();
        exit(EXIT_FAILURE);
    }

    int sd = -1;
    if (opts.host == NULL) {
        /* server action */
        int serv_sd = create_servsock(opts.port);
        if (serv_sd == -1) {
            fprintf(stderr, "error: failure listening on port %d\n", opts.port);
            exit(EXIT_FAILURE);
        }
        sd = accept(serv_sd, NULL, 0);
        if (sd == -1) {
            fprintf(stderr, "error: failure accepting connection on port %d\n", 
                    opts.port);
            exit(EXIT_FAILURE);
        }
        /* no further use for listening socket */
        close(serv_sd);
    }
    else {
        /* client action */
        sd = create_clntsock(opts.host, opts.port);
        if (sd == -1) {
            fprintf(stderr, "error: failure connecting to %s:%d\n", 
                    opts.host, opts.port);
            exit(EXIT_FAILURE);
        }
    }

    if (sd < 0) {
        /* should never reach here, any socket errors should be caught 
           previously */
        fprintf(stderr, "error: failure establishing connection\n");
        exit(EXIT_FAILURE);
    }

    pthread_t rx_thread, tx_thread;

    /* build argument to receiver thread */
    receiver_sender_arg_t receiver_arg;
    receiver_arg.fd = STDOUT_FILENO;
    receiver_arg.sd = sd;

    /* build argument to sender thread */
    receiver_sender_arg_t sender_arg;
    sender_arg.fd = STDIN_FILENO;
    sender_arg.sd = sd;

    int r = 0;

    /* start receiver thread */
    r = pthread_create(&rx_thread, NULL, receiver, &receiver_arg);
    if (r != 0) {
        fprintf(stderr, "error: failure creating receiver thread\n");
        exit(EXIT_FAILURE);
    }

    /* start sender thread */
    r = pthread_create(&tx_thread, NULL, sender, &sender_arg);
    if (r != 0) {
        fprintf(stderr, "error: failure creating sender thread\n");
        exit(EXIT_FAILURE);
    }

    pthread_join(rx_thread, NULL);
    pthread_cancel(tx_thread);
    pthread_join(tx_thread, NULL);

    close(sd);

    exit(EXIT_SUCCESS);
}

