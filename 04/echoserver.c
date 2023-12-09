#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAXLINE 1024
#define LISTENQ 1024  // listen 函数的等待队列的大小

// open_listenfd 函数定义开始
int open_listenfd(char *port) {
    struct addrinfo hints, *listp, *p;  // hints 用于指定期望的地址信息，listp 和 p 用于遍历地址
    int listenfd, optval = 1;          // listenfd 是监听套接字描述符，optval 用于套接字选项
    int rv;                            // 用于存储 getaddrinfo 的返回值

    // 初始化 hints 结构体，并设置套接字的相关属性
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_socktype = SOCK_STREAM;             // 指定套接字类型为流式套接字
    hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG; // 自动填充本机 IP，支持 IPv4 和 IPv6
    hints.ai_flags |= AI_NUMERICSERV;            // 使用数值端口号

    // 获取地址信息列表
    if ((rv = getaddrinfo(NULL, port, &hints, &listp)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return -2; // getaddrinfo 错误，返回 -2
    }

    // 遍历地址列表，尝试绑定
    for (p = listp; p; p = p->ai_next) {
        // 创建套接字描述符
        if ((listenfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0)
            continue; // 创建套接字失败，尝试下一个地址

        // 设置套接字选项，避免地址占用错误
        setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval, sizeof(int));

        // 绑定套接字到地址
        if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0)
            break; // 绑定成功，退出循环
        close(listenfd); // 绑定失败，关闭套接字并尝试下一个地址
    }

    // 释放地址信息列表
    freeaddrinfo(listp);
    if (!p) // 所有地址均绑定失败
        return -1;

    // 设置套接字为监听状态
    if (listen(listenfd, LISTENQ) < 0) {
        close(listenfd);
        return -1;
    }
    return listenfd; // 返回监听套接字描述符
}

void echo(int connfd) {
    ssize_t n;
    char buf[MAXLINE];

    while ((n = read(connfd, buf, MAXLINE)) > 0) { // 读取数据直到没有更多数据可读
        printf("server received %zd bytes\n", n); // 打印接收到的字节数
        write(connfd, buf, n); // 将接收到的数据回写给客户端
    }

    if (n < 0) {
        perror("read error"); // 如果 read 函数返回负值，打印错误信息
    }
}


int main(int argc, char **argv)
{
    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    char client_hostname[MAXLINE], client_port[MAXLINE];

    // 检查命令行参数
    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(0);
    }

    listenfd = open_listenfd(argv[1]);
    if (listenfd < 0) {
        fprintf(stderr, "Error opening listening socket\n");
        exit(1);
    }

    while (1) {
        clientlen = sizeof(struct sockaddr_storage);
        connfd = accept(listenfd, (struct sockaddr *)&clientaddr, &clientlen);
        if (connfd < 0) {
            perror("accept error");
            continue;
        }

        // 获取客户端的主机名和端口号
        if (getnameinfo((struct sockaddr *)&clientaddr, clientlen, 
                        client_hostname, MAXLINE, client_port, MAXLINE, 0) != 0) {
            fprintf(stderr, "Error getting name info\n");
        } else {
            printf("Connected to (%s, %s)\n", client_hostname, client_port);
        }

        echo(connfd); // 执行回声操作

        close(connfd); // 关闭连接
    }
    // 这里不需要 exit(0)，因为程序会一直运行在 while 循环中
}
