#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#define MAXLINE 8192  // Maximum length of the buffer for storing address strings

int main(int argc, char **argv) {
    struct addrinfo *p, *listp, hints;  // Pointers for address info and hints structure
    char buf[MAXLINE];  // Buffer for storing address strings
    int rc, flags;  // Variables for return code and flags

    // Check command line arguments
    if (argc != 2) {
        fprintf(stderr, "usage: %s <domain name>\n", argv[0]);
        exit(0);
    }

    // Initialize the hints structure to zero
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;       // Specify IPv4 internet protocols
    hints.ai_socktype = SOCK_STREAM; // Specify stream socket type for connections

    // Get a list of addrinfo records
    if ((rc = getaddrinfo(argv[1], NULL, &hints, &listp)) != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(rc));
        exit(1);
    }

    // Walk the list and display each IP address
    flags = NI_NUMERICHOST; // Use numeric address string format
    for (p = listp; p; p = p->ai_next) {
        // Get address info in string format
        if (getnameinfo(p->ai_addr, p->ai_addrlen, buf, MAXLINE, NULL, 0, flags) != 0) {
            fprintf(stderr, "getnameinfo error\n");
            continue;
        }
        printf("%s\n", buf); // Print the address string
    }

    // Free the linked list of addrinfo structures
    freeaddrinfo(listp);

    exit(0);
}

