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
 * sys/socket.h: AF_INET, SOCK_STREAM, SHUT_WR, socket(), connect(), bind(), 
 *               listen(), shutdown()
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
    int iput; /* descriptor for input  */
    int oput; /* descriptor for output */
    int sock; /* socket descriptor     */
} recvsender_arg_t;

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

    opts->port = atoi(argv[argc - 1]);
    if (opts->port < 1)
        /* invalid port number */
        return -1;

    if (argc > 2)
        opts->host = argv[argc - 2];

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
 * Reads data from an input file/socket descriptor, and writes that data to an 
 * output file/socket descriptor. Takes arguments for a pointer to a 
 * recvsender_arg_t struct {arg} which has the following elements:
 *
 * {iput}: file/socket descriptor to read from
 * {oput}: file/socket descriptor to write to
 * {endt}: thread to cancel on exit
 *
 * Returns NULL.
 */
void *recvsender(void *arg) {
    int iput = ((recvsender_arg_t *)arg)->iput;
    int oput = ((recvsender_arg_t *)arg)->oput;
    int sock = ((recvsender_arg_t *)arg)->sock;

    char buf[BUFSIZE];
    int  rbytes = 0, wbytes = 0;

    rbytes = read(iput, buf, BUFSIZE);
    while (rbytes > 0) {
        wbytes = write(oput, buf, rbytes);
        while (wbytes < rbytes)
            wbytes += write(oput, buf + wbytes, rbytes - wbytes);
        rbytes = read(iput, buf, BUFSIZE);
    }

    if (oput == sock)
        shutdown(oput, SHUT_WR); /* shutdown socket for writing */

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
        /* act as server */
        int serv_sd = create_servsock(opts.port);
        if (serv_sd == -1) {
            fprintf(stderr, "netbat: error: failure listening on port %d\n", 
                    opts.port);
            exit(EXIT_FAILURE);
        }
        sd = accept(serv_sd, NULL, 0);
        if (sd == -1) {
            fprintf(stderr, "netbat: error: failure accepting connection on \
                    port %d\n", opts.port);
            exit(EXIT_FAILURE);
        }
        /* no further use for listening socket */
        close(serv_sd);
    }
    else {
        /* act as client */
        sd = create_clntsock(opts.host, opts.port);
        if (sd == -1) {
            fprintf(stderr, "netbat: error: failure connecting to %s:%d\n", 
                    opts.host, opts.port);
            exit(EXIT_FAILURE);
        }
    }

    if (sd < 0) {
        /* should never reach here, any socket errors should be caught 
           previously */
        fprintf(stderr, "netbat: error: failed to establish connection\n");
        exit(EXIT_FAILURE);
    }

    pthread_t rx_thread, tx_thread;

    /* build argument to receiver thread */
    recvsender_arg_t recver_arg;
    recver_arg.iput = sd;
    recver_arg.oput = STDOUT_FILENO;
    recver_arg.sock = sd;

    /* build argument to sender thread */
    recvsender_arg_t sender_arg;
    sender_arg.iput = STDIN_FILENO;
    sender_arg.oput = sd;
    sender_arg.sock = sd;

    int r = 0;

    /* start receiver thread */
    r = pthread_create(&rx_thread, NULL, recvsender, &recver_arg);
    if (r != 0) {
        fprintf(stderr, "netbat: error: failure creating receiver thread\n");
        exit(EXIT_FAILURE);
    }

    /* start sender thread */
    r = pthread_create(&tx_thread, NULL, recvsender, &sender_arg);
    if (r != 0) {
        fprintf(stderr, "netbat: error: failure creating sender thread\n");
        exit(EXIT_FAILURE);
    }

    pthread_join(rx_thread, NULL);
    pthread_cancel(tx_thread);
    pthread_join(tx_thread, NULL);

    close(sd);

    exit(EXIT_SUCCESS);
}

