#include "csapp.h"

#define MAXLINE 8192  // Maximum length of the buffer for storing address strings


/* Function prototypes */
void doit(int fd);
void read_requesthdrs(rio_t *rp);
int parse_uri(char *uri, char *filename, char *cgiargs);
void serve_static(int fd, char *filename, int filesize);
void get_filetype(char *filename, char *filetype);
void serve_dynamic(int fd, char *filename, char *cgiargs);
void clienterror(int fd, char *cause, char *errnum,
                 char *shortmsg, char *longmsg);
int open_listenfd(char *port);
void echo(int connfd);

int main(int argc, char **argv)
{
    int listenfd, connfd; /* File descriptors for listening and connection sockets */
    char hostname[MAXLINE], port[MAXLINE]; /* Buffers for storing hostname and port */
    socklen_t clientlen; /* Length of client's address */
    struct sockaddr_storage clientaddr; /* Client address structure */

    /* Validate command-line arguments */
    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }
    
    listenfd = Open_listenfd(argv[1]); /* Open a listening socket on the specified port */
    while (1) {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen); /* Accept a client connection */
        Getnameinfo((SA *) &clientaddr, clientlen, hostname, MAXLINE,
                    port, MAXLINE, 0); /* Retrieve and print client's hostname and port */
        printf("Accepted connection from (%s, %s)\n", hostname, port);
        // echo(connfd);
        doit(connfd); /* Handle the client's request */
        Close(connfd); /* Close the connection socket */
    }
}

void echo(int connfd) {
    size_t n;
    char buf[MAXLINE];
    rio_t rio;

    Rio_readinitb(&rio, connfd);
    while ((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0) {
        if (strcmp(buf, "\r\n") == 0)
        break;
        Rio_writen(connfd, buf, n);
    }
}

void doit(int fd)
{
    int is_static; /* Flag to determine if the request is for static content */
    struct stat sbuf; /* Structure to hold file statistics */
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE]; /* Buffers to store request components */
    char filename[MAXLINE], cgiargs[MAXLINE]; /* Buffers for filename and CGI arguments */
    rio_t rio; /* Robust I/O structure */

    /* Read request line and headers */
    Rio_readinitb(&rio, fd); /* Initialize robust I/O for the client's file descriptor */
    Rio_readlineb(&rio, buf, MAXLINE); /* Read the request line */
    printf("Request headers:\n");
    printf("%s", buf);
    sscanf(buf, "%s %s %s", method, uri, version); /* Parse method, URI, and version from the request line */
    if (strcasecmp(method, "GET")) { /* Check if the method is not GET */
        clienterror(fd, method, "501", "Not implemented",
                    "Tiny does not implement this method"); /* Send error for non-GET methods */
        return;
    }
    read_requesthdrs(&rio); /* Read and ignore request headers */

    /* Parse URI from GET request */
    is_static = parse_uri(uri, filename, cgiargs); /* Determine if the URI is for static or dynamic content */
    if (stat(filename, &sbuf) < 0) { /* Check if the file exists */
        clienterror(fd, filename, "404", "Not found",
                    "Tiny couldn’t find this file"); /* Send 404 if file not found */
        return;
    }

    if (is_static) { /* Serve static content */
        if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)) { /* Check if file is regular and readable */
            clienterror(fd, filename, "403", "Forbidden",
                        "Tiny couldn’t read the file"); /* Send 403 if file is not regular or not readable */
            return;
        }
        serve_static(fd, filename, sbuf.st_size); /* Serve the static file */
    }
    else { /* Serve dynamic content */
        if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode)) { /* Check if file is regular and executable */
            clienterror(fd, filename, "403", "Forbidden",
                        "Tiny couldn’t run the CGI program"); /* Send 403 if file is not executable */
            return;
        }
        serve_dynamic(fd, filename, cgiargs); /* Serve the dynamic content */
    }
}


void clienterror(int fd, char *cause, char *errnum,
                 char *shortmsg, char *longmsg)
{
    char buf[MAXLINE], body[MAXBUF]; /* Buffers for the response header and body */

    /* Build the HTTP response body */
    sprintf(body, "<html><title>Tiny Error</title>"); /* Start of the HTML response */
    sprintf(body, "%s<body bgcolor=""ffffff"">\r\n", body); /* HTML body with background color */
    sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg); /* Error number and short message */
    sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause); /* Long message and cause of the error */
    sprintf(body, "%s<hr><em>The Tiny Web server</em>\r\n", body); /* Footer with server name */
    
    /* Print the HTTP response */
    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg); /* Start of the HTTP response header with status code and message */
    Rio_writen(fd, buf, strlen(buf)); /* Send the status line */
    sprintf(buf, "Content-type: text/html\r\n"); /* Specify the content type as HTML */
    Rio_writen(fd, buf, strlen(buf)); /* Send the Content-Type header */
    sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body)); /* Send the Content-Length header */
    Rio_writen(fd, buf, strlen(buf)); /* Send the end of header section */
    Rio_writen(fd, body, strlen(body)); /* Send the actual HTML response body */
}

// TINY 不使用请求报头中的任何信息。它仅仅调用图 11-32 中的 read_requesthdrs 函数来读取并忽略这些报头。
void read_requesthdrs(rio_t *rp)
{
    char buf[MAXLINE];
    
    Rio_readlineb(rp, buf, MAXLINE); // Read the first line from the request headers
    while (strcmp(buf, "\r\n")) { // Loop until an empty line is read (indicating the end of headers)
        Rio_readlineb(rp, buf, MAXLINE); // Read the next line from the request headers
        printf("%s", buf); // Print the header line (useful for logging or debugging)
    }
    return; // Return from the function once all headers are read
}


int parse_uri(char *uri, char *filename, char *cgiargs)
{
    char *ptr;
    
    if (!strstr(uri, "cgi-bin")) { // Check if the URI is for static content
        strcpy(cgiargs, ""); // Clear CGI arguments for static content
        strcpy(filename, "."); // Start building the file path
        strcat(filename, uri); // Append the URI to the file path
        if (uri[strlen(uri) - 1] == '/')
            strcat(filename, "home.html"); // Default to home.html if URI ends with '/'
        return 1; // Return 1 to indicate static content
    }
    else { // URI is for dynamic content
        ptr = index(uri, '?'); // Find the start of the query string
        if (ptr) {
            strcpy(cgiargs, ptr + 1); // Copy the CGI arguments
            *ptr = '\0'; // Terminate the URI string at the start of the query
        }
        else
            strcpy(cgiargs, ""); // If no query string, clear CGI arguments
        strcpy(filename, "."); // Start building the file path
        strcat(filename, uri); // Append the URI to the file path
        return 0; // Return 0 to indicate dynamic content
    }
}



void serve_static(int fd, char *filename, int filesize)
{
    int srcfd;
    char *srcp, filetype[MAXLINE], buf[MAXBUF];

    /* Send HTTP response headers to client */
    get_filetype(filename, filetype); // Determine the content type of the file
    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    sprintf(buf, "%sServer: Tiny Web Server\r\n", buf); // Append Server header to response
    sprintf(buf, "%sConnection: close\r\n", buf); // Append Connection header to response
    sprintf(buf, "%sContent-length: %d\r\n", buf, filesize); // Append Content-Length header to response
    sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, filetype); // Append Content-Type header to response
    Rio_writen(fd, buf, strlen(buf)); // Write the headers to the client
    printf("Response headers:\n");
    printf("%s", buf); // Print the response headers

    /* Send response body to client */
    srcfd = Open(filename, O_RDONLY, 0); // Open the file for reading
    srcp = Mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0); // Map the file into memory
    Close(srcfd); // Close the file descriptor
    Rio_writen(fd, srcp, filesize); // Write the file's contents to the client
    Munmap(srcp, filesize); // Unmap the file from memory
}

/*
* get_filetype - Derive file type from filename
*/
void get_filetype(char *filename, char *filetype)
{
    // Identify the file type based on file extension
    if (strstr(filename, ".html"))
        strcpy(filetype, "text/html");
    else if (strstr(filename, ".gif"))
        strcpy(filetype, "image/gif");
    else if (strstr(filename, ".png"))
        strcpy(filetype, "image/png");
    else if (strstr(filename, ".jpg"))
        strcpy(filetype, "image/jpeg");
    else
        strcpy(filetype, "text/plain"); // Default file type
}



void serve_dynamic(int fd, char *filename, char *cgiargs)
{
    char buf[MAXLINE], *emptylist[] = { NULL };

    // Send the HTTP response's initial part
    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    Rio_writen(fd, buf, strlen(buf));  // Write the OK status line to the client
    sprintf(buf, "Server: Tiny Web Server\r\n");
    Rio_writen(fd, buf, strlen(buf));  // Write the Server header to the client
    
    if (Fork() == 0) { // Child process
        // Set CGI (Common Gateway Interface) environment variable
        setenv("QUERY_STRING", cgiargs, 1); // Set QUERY_STRING for CGI script

        Dup2(fd, STDOUT_FILENO); // Redirect standard output to the client
        Execve(filename, emptylist, environ); // Execute the CGI script
    }
    Wait(NULL); // Parent process waits for the child to complete and reaps it
}
