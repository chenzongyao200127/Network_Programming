#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#define MAXLINE 1024

int open_clientfd(char *hostname, char *port); // 假设之前定义的 open_clientfd 函数

int main(int argc, char **argv)
{
    int clientfd;
    char *host, *port, buf[MAXLINE];

    // 检查命令行参数
    if (argc != 3) {
        fprintf(stderr, "usage: %s <host> <port>\n", argv[0]);
        exit(0);
    }
    host = argv[1];
    port = argv[2];

    // 连接到服务器
    clientfd = open_clientfd(host, port);
    if (clientfd < 0) {
        fprintf(stderr, "Failed to connect to the server\n");
        exit(1);
    }

    // 从标准输入读取数据，并发送到服务器，然后读取响应
    while (fgets(buf, MAXLINE, stdin) != NULL) {
        write(clientfd, buf, strlen(buf)); // 发送数据到服务器
        if (read(clientfd, buf, MAXLINE) > 0) {
            fputs(buf, stdout); // 输出服务器响应
        }
    }

    close(clientfd); // 关闭连接
    exit(0);
}

// open_clientfd 函数定义开始
int open_clientfd(char *hostname, char *port) {
    int clientfd;                       // clientfd 是客户端套接字描述符
    struct addrinfo hints, *listp, *p;  // hints 用于指定期望的地址信息，listp 和 p 用于遍历地址
    int rv;                             // 用于存储 getaddrinfo 的返回值

    // 初始化 hints 结构体，并设置套接字的相关属性
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_socktype = SOCK_STREAM;    // 指定套接字类型为流式套接字
    hints.ai_flags = AI_NUMERICSERV;    // 使用数值端口号
    hints.ai_flags |= AI_ADDRCONFIG;    // 推荐设置，适用于连接

    // 获取地址信息列表
    if ((rv = getaddrinfo(hostname, port, &hints, &listp)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return -2; // getaddrinfo 错误，返回 -2
    }

    // 遍历地址列表，尝试连接
    for (p = listp; p; p = p->ai_next) {
        // 创建套接字描述符
        if ((clientfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0)
            continue; // 创建套接字失败，尝试下一个地址

        // 连接到服务器
        if (connect(clientfd, p->ai_addr, p->ai_addrlen) != -1)
            break; // 连接成功，退出循环
        close(clientfd); // 连接失败，关闭套接字并尝试下一个地址
    }

    // 释放地址信息列表
    freeaddrinfo(listp); 
    if (!p) // 所有连接尝试均失败
        return -1;
    else    // 最后一次连接尝试成功
        return clientfd; // 返回客户端套接字描述符
}