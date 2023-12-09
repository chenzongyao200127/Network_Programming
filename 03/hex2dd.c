#include <stdio.h>
#include <arpa/inet.h>

int main() {
    const char *src = "128.2.194.242"; // 点分十进制的IP地址
    struct in_addr in_addr; // 用于存储网络字节序的IP地址

    // 将点分十进制的IP地址转换为网络字节序的二进制形式
    if (inet_pton(AF_INET, src, &in_addr) != 1) {
        fprintf(stderr, "inet_pton failed\n");
        return 1;
    }

    printf("原始IP地址: %s\n", src);
    printf("网络字节序的十六进制形式: ");

    // 打印网络字节序的IP地址的十六进制表示
    unsigned char *bytes = (unsigned char *)&in_addr;
    for (size_t i = 0; i < sizeof(in_addr); i++) {
        printf("%02x", bytes[i]);
    }
    printf("\n");

    return 0;
}
