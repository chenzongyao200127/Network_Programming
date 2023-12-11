# The Tiny Web Server
# 综合：TINY Web 服务器

我们通过开发一个虽小但功能齐全的称为 TINY 的 Web 服务器来结束对网络编程的讨论。

TINY 是一个有趣的程序。在短短 250 行代码中，它结合了许多我们已经学习到的思想，例如进程控制、Unix I/O、套接字接口和 HTTP。虽然它缺乏一个实际服务器所具备的功能性、健壮性和安全性，但是它足够用来为实际的 Web 浏览器提供静态和动态的内容。

我们鼓励你研究它，并且自己实现它。将一个实际的浏览器指向你自己的服务器，看着它显示一个复杂的带有文本和图片的 Web 页面，真是非常令人兴奋（甚至对我们这些作者来说，也是如此！）。

# 1. TINY 的 main 程序

图 11-29 展示了 TINY 的主程序。TINY 是一个迭代服务器，监听在命令行中传递来的端口上的连接请求。
在通过调用 open_listenfd 函数打开一个监听套接字以后，TINY 执行典型的无限服务器循环，不断地接受连接请求（第 32 行），执行事务（第 36 行），并关闭连接的它那一端（第 37 行）。

~~~c
// tiny.c
/*
* tiny.c - A simple, iterative HTTP/1.0 Web server that uses the
* GET method to serve static and dynamic content
*/
/* Include necessary headers from 'csapp.h' */
#include "csapp.h"

/* Function prototypes */
void doit(int fd);
void read_requesthdrs(rio_t *rp);
int parse_uri(char *uri, char *filename, char *cgiargs);
void serve_static(int fd, char *filename, int filesize);
void get_filetype(char *filename, char *filetype);
void serve_dynamic(int fd, char *filename, char *cgiargs);
void clienterror(int fd, char *cause, char *errnum,
                 char *shortmsg, char *longmsg);

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
        doit(connfd); /* Handle the client's request */
        Close(connfd); /* Close the connection socket */
    }
}
~~~

## 2. doit 函数
图 11-30 中的 doit 函数处理一个 HTTP 事务。

~~~c
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
~~~

首先，我们读和解析请求行（第 11 ~ 14 行）。注意，我们使用图 11-8 中的 rio_readlineb 函数读取请求行。

TINY 只支持 GET 方法。如果客户端请求其他方法（比如 POST），我们发送给它一个错误信息，并返回到主程序（第 15 ~ 19 行），主程序随后关闭连接并等待下一个连接请求。否则，我们读并且（像我们将要看到的那样）忽略任何请求报头（第 20 行）。

然后，我们将 URI 解析为一个文件名和一个可能为空的 CGI 参数字符串，并且设置一个标志，表明请求的是静态内容还是动态内容（第 23 行）。如果文件在磁盘上不存在，我们立即发送一个错误信息给客户端并返回。

最后，如果请求的是静态内容，我们就验证该文件是一个普通文件，而我们是有读权限的（第 31 行）。如果是这样，我们就向客户端提供静态内容（第 36 行）。相似地，如果请求的是动态内容，我们就验证该文件是可执行文件（第 39 行），如果是这样，我们就继续，并且提供动态内容（第 44 行）。

# 3. clienterror 函数

TINY 缺乏一个实际服务器的许多错误处理特性。然而，它会检査一些明显的错误，并把它们报告给客户端。

图 11-31 中的 clienterror 函数发送一个 HTTP 响应到客户端，在响应行中包含相应的状态码和状态消息，响应主体中包含一个 HTML 文件，向浏览器的用户解释这个错误。

~~~c
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
~~~

图 11-31 TINY clienterror 向客户端发送一个出错消息

回想一下，HTML 响应应该指明主体中内容的大小和类型。因此，我们选择创建 HTML 内容为一个字符串，这样一来我们可以简单地确定它的大小。
还有，请注意我们为所有的输出使用的都是图 10-4 中健壮的 rio_writen 函数。


# 4. read_requesthdrs 函数

TINY 不使用请求报头中的任何信息。它仅仅调用图 11-32 中的 `read_requesthdrs` 函数来读取并忽略这些报头。
注意，终止请求报头的空文本行是由回车和换行符对组成的，我们在第 6 行中检査它。

~~~c
void read_requesthdrs(rio_t *rp)
{
    char buf[MAXLINE];
    
    Rio_readlineb(rp, buf, MAXLINE);
    while (strcmp(buf, "\r\n")) {
        Rio_readlineb(rp, buf, MAXLINE);
        printf("%s", buf);
    }
    return;
}
~~~

# parse_uri 函数

TINY 假设静态内容的主目录就是它的当前目录，而可执行文件的主目录是 `./cgi-bin`。
任何包含字符串 cgi-bin 的 URI 都会被认为表示的是对动态内容的请求。默认的文件名是 `./home.html`。

图 11-33 中的 parse_uri 函数实现了这些策略。它将 URI 解析为一个文件名和一个可选的 CGI 参数字符串。如果请求的是静态内容（第 5 行），我们将清除 CGI 参数字符串（第 6 行），然后将 URI 转换为一个 Linux 相对路径名，例如 ./index.html（第 7 ~ 8 行）。如果 URI 是用结尾的（第 9 行），我们将把默认的文件名加在后面（第 10 行）。另一方面，如果请求的是动态内容（第 13 行），我们就会抽取出所有的 CGI 参数（第 14 ~ 20 行），并将 URI 剩下的部分转换为一个 Linux 相对文件名（第 21 ~ 22 行）。

~~~c
int parse_uri(char *uri, char *filename, char *cgiargs)
{
    char *ptr;
    
    if (!strstr(uri, "cgi-bin")) { /* Static content */
        strcpy(cgiargs, "");
        strcpy(filename, ".");
        strcat(filename, uri);
        if (uri[strlen(uri) - 1] == ’ / ’)
            strcat(filename, "home.html");
        return 1;
    }
    else { /* Dynamic content */
        ptr = index(uri, ’ ? ’);
        if (ptr) {
            strcpy(cgiargs, ptr + 1);
            *ptr = ’\0’;
        }
        else
            strcpy(cgiargs, "");
        strcpy(filename, ".");
        strcat(filename, uri);
        return 0;
    }
}
~~~

# 6. serve_static 函数

TINY 提供五种常见类型的静态内容：HTML 文件、无格式的文本文件，以及编码为 GIF、PNG 和 JPG 格式的图片。
图 11-34 中的 serve_static 函数发送一个 HTTP 响应，其主体包含一个本地文件的内容。
~~~c
void serve_static(int fd, char *filename, int filesize)
{
    int srcfd;
    char *srcp, filetype[MAXLINE], buf[MAXBUF];

    /* Send response headers to client */
    get_filetype(filename, filetype);
    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    sprintf(buf, "%sServer: Tiny Web Server\r\n", buf);
    sprintf(buf, "%sConnection: close\r\n", buf);
    sprintf(buf, "%sContent-length: %d\r\n", buf, filesize);
    sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, filetype);
    Rio_writen(fd, buf, strlen(buf));
    printf("Response headers:\n");
    printf("%s", buf);

    /* Send response body to client */
    srcfd = Open(filename, O_RDONLY, 0);
    srcp = Mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
    Close(srcfd);
    Rio_writen(fd, srcp, filesize);
    Munmap(srcp, filesize);
}

/*
* get_filetype - Derive file type from filename
*/
void get_filetype(char *filename, char *filetype)
{
    if (strstr(filename, ".html"))
        strcpy(filetype, "text/html");
    else if (strstr(filename, ".gif"))
        strcpy(filetype, "image/gif");
    else if (strstr(filename, ".png"))
        strcpy(filetype, "image/png");
    else if (strstr(filename, ".jpg"))
        strcpy(filetype, "image/jpeg");
    else
        strcpy(filetype, "text/plain");
}
~~~

首先，我们通过检査文件名的后缀来判断文件类型（第 7 行），并且发送响应行和响应报头给客户端（第 8 ~ 13 行）。注意用一个空行终止报头。

接着，我们将被请求文件的内容复制到已连接描述符 fd 来发送响应主体。这里的代码是比较微妙的，需要仔细研究。第 18 行以读方式打开 filename，并获得它的描述符。在第 19 行，Linux mmap 函数将被请求文件映射到一个虚拟内存空间。回想我们在第 9.8 节中对 remap 的讨论，调用 mmap 将文件 srcfd 的前 filesize 个字节映射到一个从地址 srcp 开始的私有只读虚拟内存区域。

一旦将文件映射到内存，就不再需要它的描述符了，所以我们关闭这个文件（第 20 行）。执行这项任务失败将导致潜在的致命的内存泄漏。第 21 行执行的是到客户端的实际文件传送。rio_writen 函数复制从 srcp 位置开始的 filesize 个字节（它们当然已经被映射到了所请求的文件）到客户端的已连接描述符。最后，第 22 行释放了映射的虚拟内存区域。这对于避免潜在的致命的内存泄漏是很重要的。

# 7. serve_dynamic 函数

TINY 通过派生一个子进程并在子进程的上下文中运行一个 CGI 程序，来提供各种类型的动态内容。
图 11-35 中的 serve_dynamic 函数一开始就向客户端发送一个表明成功的响应行，同时还包括带有信息的 Server 报头。CGI 程序负责发送响应的剩余部分。注意，这并不像我们可能希望的那样健壮，因为它没有考虑到 CGI 程序会遇到某些错误的可能性。

~~~c
void serve_dynamic(int fd, char *filename, char *cgiargs)
{
    char buf[MAXLINE], *emptylist[] = { NULL };

    /* Return first part of HTTP response */
    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Server: Tiny Web Server\r\n");
    Rio_writen(fd, buf, strlen(buf));
    
    if (Fork() == 0) { /* Child */
        /* Real server would set all CGI vars here */
        setenv("QUERY_STRING", cgiargs, 1);
        Dup2(fd, STDOUT_FILENO); /* Redirect stdout to client */
        Execve(filename, emptylist, environ); /* Run CGI program */
    }
    Wait(NULL); /* Parent waits for and reaps child */
}
~~~

在发送了响应的第一部分后，我们会派生一个新的子进程（第 11 行）。子进程用来自请求 URI 的 CGI 参数初始化 QUERY_STRING 环境变量（第 13 行）。注意，一个真正的服务器还会在此处设置其他的 CGI 环境变量。为了简短，我们省略了这一步。

接下来，子进程重定向它的标准输出到已连接文件描述符（第 14 行），然后加载并运行 CGI 程序（第 15 行）。
因为 CGI 程序运行在子进程的上下文中，它能够访问所有在调用 execve 函数之前就存在的打开文件和环境变量。因此，CGI 程序写到标准输出上的任何东西都将直接送到客户端进程，不会受到任何来自父进程的干涉。
其间，父进程阻塞在对 wait 的调用中，等待当子进程终止的时候，回收操作系统分配给子进程的资源（第 17 行）。

> 旁注 - 处理过早关闭的连接
> 尽管一个 Web 服务器的基本功能非常简单，但是我们不想给你一个假象，以为编写一个实际的 Web 服务器是非常简单的。构造一个长时间运行而不崩溃的健壮的 Web 服务器是一件困难的任务，比起在这里我们已经学习了的内容，它要求对 Linux 系统编程有更加深入的理解。
> 例如，如果一个服务器写一个已经被客户端关闭了的连接（比如，因为你在浏览器上单击了 “Stop” 按钮），那么第一次这样的写会正常返回，但是第二次写就会引起发送 SIGPIPE 信号，这个信号的默认行为就是终止这个进程。如果捕获或者忽略 SIGPIPE 信号，那么第二次写操作会返回值 -1，并将 errno 设置为 EPIPE。strerr 和 perror 函数将 EPIPE 错误报告为 “Broken pipe”，这是一个迷惑了很多人的不太直观的信息。
> 总的来说，一个健壮的服务器必须捕获这些 SIGPIPE 信号，并且检查 write 函数调用是否有 EPIPE 错误。

1. **Web 服务器的基本功能与复杂性**：尽管Web服务器的基本功能（例如处理HTTP请求和响应）相对简单，但构建一个稳定、可靠的服务器需要对Linux系统编程有深入的理解。这意味着，除了处理基本的网络通信之外，还需要考虑资源管理、错误处理、安全问题等更复杂的方面。

2. **处理SIGPIPE信号**：这是Linux系统编程中的一个特定问题。当服务器尝试写入一个已经被客户端关闭的连接时，操作系统会向服务器进程发送一个SIGPIPE信号。默认情况下，这个信号会终止进程。但是，为了保持服务器的稳定性，需要特别处理这个信号。通常的做法是捕获或忽略SIGPIPE信号，然后检查每次写操作的结果。

3. **EPIPE错误的处理**：如果服务器程序忽略了SIGPIPE信号，那么在尝试写入一个已关闭连接时，write函数会返回-1，并设置errno为EPIPE。这个错误条件需要被正确识别和处理。错误消息“Broken pipe”通常用于报告这种情况，尽管这个信息可能对于初学者来说不太直观。

4. **编写健壮的服务器**：综上所述，一个健壮的Web服务器必须能够处理各种网络异常和系统信号。这包括正确地处理SIGPIPE信号和EPIPE错误，以及其他可能导致服务器崩溃或行为不当的情况。

总的来说，编写一个健壮的Web服务器不仅仅是处理HTTP请求和响应那么简单，它涉及到深入的系统编程技巧，特别是在错误处理和资源管理方面。这要求开发者对底层的系统行为有清晰的理解，以确保服务器的稳定性和可靠性。