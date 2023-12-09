# The Sockets Interface
# 套接字接口

套接字接口（`socket interface`）是一组函数，它们和 Unix I/O 函数结合起来，用以创建网络应用。大多数现代系统上都实现套接字接口，包括所有的 Unix 变种、Windows 和 Macintosh 系统。

图 11-12 基于套接字接口的网络应用概述

> 旁注 - 套接字接口的起源
> 套接字接口是加州大学伯克利分校的研究人员在 20 世纪 80 年代早期提出的。因为这个原因，它也经常被叫做伯克利套接字。
> 伯克利的研究者使得套接字接口适用于任何底层的协议。第一个实现的就是针对 TCP/IP 协议的，他们把它包括在 Unix 4.2 BSD 的内核里，并且分发给许多学校和实验室。这在因特网的历史上是一个重大事件。几乎一夜之间，成千上万的人们接触到了 TCP/IP 和它的源代码。它引起了巨大的轰动，并激发了新的网络和网络互联研究的浪潮。

# 套接字地址结构

 - 从 Linux 内核的角度来看，一个套接字就是通信的一个端点。
 - 从 Linux 程序的角度来看，套接字就是一个有相应描述符的打开文件。

因特网的套接字地址存放在如图 11-13 所示的类型为 `sockaddr_in` 的 16 字节结构中。
对于因特网应用，`sin_family` 成员是 `AF_INET`，`sin_port` 成员是一个 16 位的端口号，而 `sin_addr` 成员就是一个 32 位的 IP 地址。
IP 地址和端口号总是以网络字节顺序（大端法）存放的。

~~~c
// netfragments.c

/* 
 * This structure is used in socket programming to specify an IP socket address.
 */
struct sockaddr_in {
    uint16_t       sin_family;   /* Identifies the address family (AF_INET for Internet Protocol v4). */
    uint16_t       sin_port;     /* Specifies the port number in network byte order (big-endian). */
    struct in_addr sin_addr;     /* Represents the IP address in network byte order. */
    unsigned char  sin_zero[8];  /* Padding to make the structure the same size as `struct sockaddr`. */
};

/* 
 * This is a more generic socket address structure used for various network functions 
 * like connect, bind, and accept. It's less specific than `struct sockaddr_in`.
 */
struct sockaddr {
    uint16_t  sa_family;    /* Specifies the address family (e.g., AF_INET for IPv4, AF_INET6 for IPv6). */
    char      sa_data[14];  /* Contains the address data; its format depends on the address family. */
};
~~~
套接字地址结构

> 旁注 - `_in`后缀意味着什么？
> in 后缀是互联网络（internet）的缩写，而不是输入（input）的缩写。

`connect`、`bind` 和 `accept` 函数要求一个指向与协议相关的套接字地址结构的指针。
套接字接口的设计者面临的问题是，如何定义这些函数，使之能接受各种类型的套接字地址结构。今天我们可以使用通用的 void* 指针，但是那时在 C 中并不存在这种类型的指针。

解决办法是定义套接字函数要求一个指向通用 `sockaddr` 结构（图 11-13）的指针，然后要求应用程序将与*协议特定的结构的指针强制转换成这个通用结构*。
为了简化代码示例，我们跟随 Steven 的指导，定义下面的类型：

~~~c
typedef struct sockaddr SA;
~~~

然后无论何时需要将 `sockaddr_in` 结构强制转换成通用 `sockaddr` 结构时，我们都使用这个类型。

# socket 函数

客户端和服务器使用 socket 函数来创建一个套接字描述符（socket descriptor）。

~~~c
#include <sys/types.h>
#include <sys/socket.h>

int socket(int domain, int type, int protocol);

// 返回：若成功则为非负描述符，若出错则为 -1。
~~~

如果想要使套接字成为连接的一个端点，就用如下硬编码的参数来调用 socket 函数：

~~~c
clientfd = Socket(AF_INET, SOCK_STREAM, 0);
~~~

其中，AF_INET 表明我们正在使用 32 位 IP 地址，而 SOCK_STREAM 表示这个套接字是连接的一个端点。

不过最好的方法是用 getaddrinfo 函数（11.4.7 节）来自动生成这些参数，这样代码就与协议无关了。我们会在 11.4.8 节中向你展示如何配合 socket 函数来使用 getaddrinfo。

socket 返回的 clientfd 描述符仅是部分打开的，还不能用于读写。如何完成打开套接字的工作，取决于我们是客户端还是服务器。下一节描述当我们是客户端时如何完成打开套接字的工作。

1. **函数声明**:
    ```c
    #include <sys/types.h>
    #include <sys/socket.h>

    int socket(int domain, int type, int protocol);
    ```
    这部分代码引入了系统头文件 `sys/types.h` 和 `sys/socket.h`，这些文件包含了进行网络编程所需的数据类型和函数声明。`socket` 函数用于创建一个新的套接字，它的三个参数分别是：
    - `domain`：指定套接字使用的协议族，比如 `AF_INET` 用于 IPv4 网络通信。
    - `type`：指定套接字的类型，比如 `SOCK_STREAM` 表示提供面向连接的稳定数据传输，即 TCP。
    - `protocol`：通常设置为0，让操作系统自动选择 `type` 和 `domain` 组合的默认协议。

2. **函数调用**:
    ```c
    clientfd = Socket(AF_INET, SOCK_STREAM, 0);
    ```
    在这里，`socket` 函数被调用以创建一个新的套接字。注意这里使用的是 `Socket` 而不是 `socket`，这可能意味着使用了一个封装了错误处理的版本（这在许多书籍和高级代码中很常见）。调用参数是：
    - `AF_INET`：表示使用 IPv4 地址。
    - `SOCK_STREAM`：表示这个套接字是面向连接的 TCP 套接字。
    - `0`：让系统自动选择适合 `AF_INET` 和 `SOCK_STREAM` 的协议。

在网络编程中，这样的套接字通常用于客户端，用于建立与服务器的 TCP 连接。通过这个套接字，客户端可以发送请求并接收服务器的响应。


# connect 函数

客户端通过调用 connect 函数来建立和服务器的连接。

~~~c
#include <sys/socket.h>

int connect(int clientfd, const struct sockaddr *addr, socklen_t addrlen);

// 返回：若成功则为 0，若出错则为 -1。
~~~

connect 函数试图与套接字地址为 addr 的服务器建立一个因特网连接，其中 addrlen 是 `sizeof(sockaddr_in)`。
connect 函数会阻塞，一直到连接成功建立或是发生错误。

如果成功，`clientfd` 描述符现在就准备好可以读写了，并且得到的连接是由套接字对 `(x:y, addr.sin_addr:addr.sin_port)`

刻画的，其中 x 表示客户端的 IP 地址，而 y 表示临时端口，它唯一地确定了客户端主机上的客户端进程。
对于 socket，最好的方法是用 `getaddrinfo` 来为 connect 提供参数（见 11.4.8 节）。

====================================================================================================
 - `bind` 将一个本地地址（IP地址和端口号）绑定到套接字上。这通常用于服务器端，以便套接字可以监听来自客户端的连接请求。

 - `listen` 使套接字进入“监听”状态，等待客户端的连接请求。

 - `accept` 接受一个来自客户端的连接请求。
====================================================================================================

# bind 函数

剩下的套接字函数——bind、listen 和 accept，服务器用它们来和客户端建立连接。

~~~c
#include <sys/socket.h>

int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

// 返回：若成功则为 0，若出错则为 -1。
~~~

bind 函数告诉内核将 `addr` 中的服务器套接字地址和套接字描述符 `sockfd` 联系起来。参数 addrlen 就是 `sizeof(sockaddr_in)`。
对于 socket 和 connect，最好的方法是用 `getaddrinfo` 来为 bind 提供参数（见 11.4.8 节）。

# listen 函数

客户端是发起连接请求的主动实体。服务器是等待来自客户端的连接请求的被动实体。
默认情况下，内核会认为 socket 函数创建的描述符对应于主动套接字（`active socket`），它存在于一个连接的客户端。
服务器调用 listen 函数告诉内核，描述符是被服务器而不是客户端使用的。

~~~c
#include <sys/socket.h>

int listen(int sockfd, int backlog);

// 返回：若成功则为 0，若出错则为 -1。
~~~

`listen` 函数将 `sockfd` 从一个主动套接字转化为一个监听套接字（`listening socket`），该套接字可以接受来自客户端的连接请求。
backlog 参数暗示了内核在开始拒绝连接请求之前，队列中要排队的未完成的连接请求的数量。backlog 参数的确切含义要求对 TCP/IP 协议的理解，这超出了我们讨论的范围。通常我们会把它设置为一个较大的值，比如 1024。


# accept 函数

服务器通过调用 accept 函数来等待来自客户端的连接请求。

~~~c
#include <sys/socket.h>

int accept(int listenfd, struct sockaddr *addr, int *addrlen);

// 返回：若成功则为非负连接描述符，若出错则为 -1。
~~~

`accept` 函数等待来自客户端的连接请求到达侦听描述符 `listenfd`，然后在 `addr` 中填写客户端的套接字地址，并返回一个已连接描述符（`connected descriptor`），这个描述符可被用来利用 Unix I/O 函数与客户端通信。

监听描述符和已连接描述符之间的区别使很多人感到迷惑。
 - 监听描述符是作为客户端连接请求的一个端点。它通常被创建一次，并存在于服务器的整个生命周期。
 - 已连接描述符是客户端和服务器之间已经建立起来了的连接的一个端点。服务器每次接受连接请求时都会创建一次，它只存在于服务器为一个客户端服务的过程中。

1. **监听描述符**:
    - **定义**: 监听描述符是一个套接字，它专门用于监听来自客户端的连接请求。当服务器准备接受新的客户端连接时，它会首先创建一个监听套接字。
    - **创建流程**: 在创建过程中，服务器首先调用 `socket()` 函数创建套接字，然后使用 `bind()` 函数将其绑定到一个本地地址（通常是一个IP地址和端口号），最后调用 `listen()` 函数使其进入监听状态。
    - **作用**: 监听描述符的主要作用是等待客户端的连接请求。它不处理实际的数据传输，而是在新的连接到来时通知服务器程序。
    - **使用场景**: 监听描述符通常在服务器端的事件循环或主循环中被检查，以确定是否有新的连接请求。

2. **已连接描述符**:
    - **定义**: 一旦服务器接受了一个来自客户端的连接请求，它会为这个特定的连接创建一个新的套接字，称为已连接描述符。
    - **创建流程**: 这是通过 `accept()` 函数调用实现的。当 `accept()` 被调用时，它从监听描述符的待处理连接队列中取出一个连接请求，创建一个新的套接字，并将这个新套接字与客户端的连接关联起来。
    - **作用**: 已连接描述符用于实际的数据传输。服务器通过这个套接字向客户端发送数据，并从客户端接收数据。
    - **使用场景**: 服务器会针对每个接受的连接创建一个新的已连接描述符，并在其生命周期内维护这些连接。这些描述符通常在服务器的多线程或多进程模型中被单独处理。

总结来说，监听描述符用于监听新的连接请求，而已连接描述符则是代表了一个已经建立的客户端连接，用于数据的接收和发送。
在服务器的套接字编程模型中，这两种描述符共同工作，以实现高效的网络通信。

1. 服务器阻塞在 `accept`，等待监听描述符 `listendf`上的连接请求
2. 客户端通过调用和阻塞在 `connect`, 创建链接请求
3. 服务器从`accept`返回`connfd`。客户端从`connect`返回。现在在`clientfd`和`connfd`之间已经建立起了链接


在第一步中，服务器调用 accept，等待连接请求到达监听描述符，具体地我们设定为描述符 3。回忆一下，描述符 0 ~ 2 是预留给了标准文件的。

在第二步中，客户端调用 connect 函数，发送一个连接请求到 listenfd。

第三步，accept 函数打开了一个新的已连接描述符 connfd（我们假设是描述符 4），在 clientfd 和 connfd 之间建立连接，并且随后返回 connfd 给应用程序。客户端也从 connect 返回，在这一点以后，客户端和服务器就可以分别通过读和写 clientfd 和 connfd 来回传送数据了。

> 注 - 为何要有监听描述符和已连接描述符之间的区别？
> 你可能很想知道为什么套接字接口要区别监听描述符和已连接描述符。乍一看，这像是不必要的复杂化。然而，区分这两者被证明是很有用的，因为它使得我们可以. 建立并发服务器，它能够同时处理许多客户端连接。
> 例如，每次一个连接请求到达监听描述符时，我们可以派生（fork）—个新的进程，它通过已连接描述符与客户端通信。在第 12 章中将介绍更多关于并发服务器的内容。

# 主机和服务的转换

Linux 提供了一些强大的函数（称为 `getaddrinfo` 和 `getnameinfo`）实现二进制套接字地址结构和主机名、主机地址、服务名和端口号的字符串表示之间的相互转化。当和套接字接口一起使用时，这些函数能使我们编写独立于任何特定版本的 IP 协议的网络程序。

## 1. getaddrinfo 函数

getaddrinfo 函数将主机名、主机地址、服务名和端口号的字符串表示转化成套接字地址结构。
它是已弃用的 gethostbyname 和 getservbyname 函数的新的替代品。和以前的那些函数不同，这个函数是可重入的（见 12.7.2 节），适用于任何协议。

~~~c
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

int getaddrinfo(const char *host, 
                const char *service,
                const struct addrinfo *hints,
                struct addrinfo **result);
// 返回：如果成功则为 0，如果错误则为非零的错误代码。

void freeaddrinfo(struct addrinfo *result);
// 返回：无。

const char *gai_strerror(int errcode);
// 返回：错误消息。
~~~

给定 `host` 和 `service`（套接字地址的两个组成部分），`getaddrinfo` 返回 `result`，result 一个指向 addrinfo 结构的链表，其中每个结构指向一个对应于 host 和 service 的套接字地址结构（图 11-15）。

在客户端调用了 getaddrinfo 之后，会遍历这个列表，依次尝试每个套接字地址，直到调用 socket 和 connect 成功，建立起连接。

类似地，服务器会尝试遍历列表中的每个套接字地址，直到调用 socket 和 bind 成功，描述符会被绑定到一个合法的套接字地址。

为了避免内存泄漏，应用程序必须在最后调用 freeaddrinfo，释放该链表。

如果 getaddrinfo 返回非零的错误代码，应用程序可以调用 gai_streeror，将该代码转换成消息字符串。

====================================================================================================
`getaddrinfo` 是一个用于解析主机名和服务名的网络编程函数。这个函数的目的是为了提供一个灵活、可重入的接口来获取主机的地址信息，它是用来替代旧的 `gethostbyname` 和 `getservbyname` 函数的。

这里是 `getaddrinfo` 函数的详细介绍：

1. **函数原型**:
   ```c
   int getaddrinfo(const char *host, const char *service, const struct addrinfo *hints, struct addrinfo **result);
   ```

2. **参数**:
   - `host`: 主机名或者是 IP 地址的字符串。如果这个参数是 `NULL`，`getaddrinfo` 将返回适用于绑定套接字的地址。
   - `service`: 服务名，可以是十进制的端口号或者是服务名称（如 "http"）。
   - `hints`: 一个指向 `addrinfo` 结构的指针，这个结构指定了服务类型和其他相关信息。例如，如果指定了 `AI_PASSIVE` 标志，返回的地址将适用于绑定套接字，用于接受新连接。如果 `hints` 为 `NULL`，则返回的地址适用于任何类型的套接字。
   - `result`: 这是一个指针的指针，指向 `addrinfo` 结构的链表，这个链表包含了所有匹配指定主机名和服务的地址信息。

3. **返回值**:
   - 成功时返回 `0`。
   - 失败时返回一个非零的错误码。这些错误码可以使用 `gai_strerror` 函数来转换为可读的错误信息。

4. **使用方式**:
   - 调用 `getaddrinfo` 时，将 `host`、`service` 和（可选的）`hints` 结构传入。
   - 函数执行成功后，`result` 指针指向一个 `addrinfo` 结构链表，其中包含了所有可用的地址信息。
   - 使用完地址信息后，应该调用 `freeaddrinfo` 函数来释放这些动态分配的结构。

5. **重要性**:
   - `getaddrinfo` 函数是网络编程中解析地址的首选方法，尤其是在处理 IPv4 和 IPv6 兼容性问题时。
   - 它提供了一个统一的接口来处理主机名和服务名的解析，有助于简化网络程序的编写。

6. **错误处理**:
   - 当 `getaddrinfo` 返回非零错误码时，可以通过调用 `gai_strerror` 函数获取错误描述。

这个函数的主要优点是它的可移植性和对 IPv6 的支持，使得网络程序更加健壮和未来证明。

1. **使用场景**:
   - **客户端**: 在客户端，`getaddrinfo` 用于获取与提供的 `host` 和 `service` 相对应的套接字地址列表。客户端程序遍历这个列表，尝试使用列表中的每个地址来创建套接字（使用 `socket` 函数）和建立连接（使用 `connect` 函数）。这个过程一直持续到成功建立连接为止。

   - **服务器**: 在服务器端，`getaddrinfo` 同样用于获取套接字地址列表。服务器会尝试这些地址，直到它能成功地创建套接字并将其绑定到一个有效的地址（使用 `bind` 函数）。

2. **重要性**:
   - 这种方法使得程序能够在不同的网络环境和地址类型（如 IPv4 和 IPv6）中更加灵活和健壮。

3. **内存管理**:
   - 由于 `getaddrinfo` 动态分配用于存储地址信息的内存，因此在使用完这些信息后，必须调用 `freeaddrinfo` 函数来释放这些内存资源。这是避免内存泄漏的重要步骤。

4. **错误处理**:
   - 如果 `getaddrinfo` 返回非零的错误代码，表明在解析过程中出现了错误。这时，可以通过调用 `gai_strerror` 函数，将错误代码转换为人类可读的错误消息。这对于调试和处理网络问题非常有帮助。

5. **示例流程**:
   - **客户端**:
     1. 调用 `getaddrinfo`。
     2. 遍历返回的地址列表。
     3. 对于每个地址，尝试创建套接字并连接。
     4. 成功连接后，进行通信。
     5. 结束后，释放地址列表。

   - **服务器**:
     1. 调用 `getaddrinfo`。
     2. 遍历返回的地址列表。
     3. 对于每个地址，尝试创建套接字并绑定。
     4. 成功绑定后，开始监听连接。
     5. 结束后，释放地址列表。

这种方法在现代网络编程中非常普遍，因为它为处理多种网络配置和协议提供了统一和高效的方式。
====================================================================================================

~~~c
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

int getaddrinfo(const char *host, 
                const char *service,
                const struct addrinfo *hints,
                struct addrinfo **result);
// 返回：如果成功则为 0，如果错误则为非零的错误代码。
~~~

getaddrinfo 的 host 参数可以是域名，也可以是数字地址（如点分十进制 IP 地址）。service 参数可以是服务名（如 http），也可以是十进制端口号。
如果不想把主机名转换成地址，可以把 host 设置为 NULL。对 service 来说也是一样。但是必须指定两者中至少一个。

可选的参数 hints 是一个 addrinfo 结构（见图 11-16），它提供对 getaddrinfo 返回的套接字地址列表的更好的控制。

~~~c
struct addrinfo {
    int             ai_flags;      /* Flags used to modify the behavior of the function call that uses this structure */
    int             ai_family;     /* Address family specification (e.g., AF_INET for IPv4, AF_INET6 for IPv6) */
    int             ai_socktype;   /* Type of socket (e.g., SOCK_STREAM for TCP or SOCK_DGRAM for UDP) */
    int             ai_protocol;   /* Protocol used on the socket (e.g., IPPROTO_TCP for TCP, usually set automatically based on ai_socktype) */
    char            *ai_canonname; /* Canonical name of the host, represents the official name of the host */
    size_t          ai_addrlen;    /* Length of the socket address pointed to by ai_addr */
    struct sockaddr *ai_addr;      /* Pointer to a socket address structure (e.g., sockaddr_in for IPv4) */
    struct addrinfo *ai_next;      /* Pointer to the next structure in a linked list, NULL if there is no next structure */
};
~~~

如果要传递 hints 参数，只能设置下列字段：`ai_family`、`ai_socktype`、`ai_protocol` 和 `ai_flags` 字段。其他字段必须设置为 0（或 NULL）。实际中，我们用 memset 将整个结构置零，然后有选择地设置一些字段：

 - getaddrinfo 默认可以返回 IPv4 和 IPv6 套接字地址。*ai_family* 设置为 AF_INET 会将列表限制为 IPv4 地址；设置为 AF_INET6 则限制为 IPv6 地址。

 - 对于 host 关联的每个地址，getaddrinfo 函数默认最多返回三个 addrinfo 结构，每个的 *ai_socktype* 字段不同：一个是连接，一个是数据报（本书未讲述），一个是原始套接字（本书未讲述）。ai_socktype 设置为 SOCK_STREAM 将列表限制为对每个地址最多一个 addrinfo 结构，该结构的套接字地址可以作为连接的一个端点。这是所有示例程序所期望的行为。

 - *ai_flags* 字段是一个位掩码，可以进一步修改默认行为。可以把各种值用 OR 组合起来得到该掩码。下面是一些我们认为有用的值：

    1. **AI_ADDRCONFIG** 如果在使用连接，就推荐使用这个标志【34】。它要求只有当本地主机被配置为 IPv4 时，getaddrinfo 返回 IPv4 地址。对 IPv6 也是类似。

    2. **AI_CANONNAME** ai_canonname 字段默认为 NULL。如果设置了该标志，就是告诉 getaddrinfo 将列表中第一个 addrinfo 结构的 ai_canonname 字段指向 host 的权威（官方）名字（见图 11-15）。

    3. **AI_NUMERICSERV** 参数 service 默认可以是服务名或端口号。这个标志强制参数 service 为端口号。

    4. **AI_PASSIVE** getaddrinfo 默认返回套接字地址，客户端可以在调用 connect 时用作主动套接字。这个标志告诉该函数，返回的套接字地址可能被服务器用作监听套接字。在这种情况中，参数 host 应该为 NULL。得到的套接字地址结构中的地址字段会是通配符地址（wildcard address），告诉内核这个服务器会接受发送到该主机所有 IP 地址的请求。这是所有示例服务器所期望的行为。

当 getaddrinfo 创建输出列表中的 addrinfo 结构时，会填写每个字段，除了 `ai_flags`。`ai_addr` 字段指向一个套接字地址结构，`ai_addrlen` 字段给出这个套接字地址结构的大小，而 `ai_next` 字段指向列表中下一个 addrinfo 结构。其他字段描述这个套接字地址的各种属性。

getaddrinfo 一个很好的方面是 addrinfo 结构中的字段是不透明的，即它们可以直接传递给套接字接口中的函数，应用程序代码无需再做任何处理。
例如，ai_family、ai_socktype 和 ai_protocol 可以直接传递给 socket。类似地，ai_addr 和 ai_addrlen 可以直接传递给 connect 和 bind。这个强大的属性使得我们编写的客户端和服务器能够独立于某个特殊版本的 IP 协议。

====================================================================================================
这段话的核心意思是，`getaddrinfo`函数的一个主要优点在于其返回的`addrinfo`结构的字段是“不透明的”。这里的“不透明”意味着这些字段可以直接用于套接字接口的函数调用，而无需应用程序对这些字段进行额外的处理或转换。具体来说：

1. **直接使用字段**：在`addrinfo`结构中，诸如`ai_family`、`ai_socktype`、`ai_protocol`等字段可以直接传递给`socket()`函数。这意味着你无需理解或修改这些字段的具体值，只需要将它们作为参数传递给`socket()`函数即可。

2. **地址和长度的直接传递**：同样地，`addrinfo`结构中的`ai_addr`（一个指向套接字地址结构的指针）和`ai_addrlen`（地址结构的大小）可以直接用于`connect()`和`bind()`等函数。这简化了网络编程中地址处理的复杂性。

3. **协议独立性**：由于`addrinfo`结构自动适应不同版本的IP协议（如IPv4和IPv6），使用`getaddrinfo`编写的客户端和服务器程序可以独立于特定版本的IP协议运行。这意味着同一套代码能够处理不同类型的网络地址，增加了程序的灵活性和未来兼容性。

总的来说，这段话强调了`getaddrinfo`和`addrinfo`结构在简化网络编程中地址和协议处理方面的优势，特别是在编写同时支持IPv4和IPv6的程序时。
====================================================================================================

## 2. getnameinfo 函数

`getnameinfo` 函数和 `getaddrinfo` 是相反的，将一个套接字地址结构转换成相应的主机和服务名字符串。
它是已弃用的 gethostbyaddr 和 getservbyport 函数的新的替代品，和以前的那些函数不同，它是可重入和与协议无关的。

~~~c
#include <sys/socket.h>
#include <netdb.h>

int getnameinfo(const struct sockaddr *sa, 
                socklen_t salen,
                char *host,
                size_t hostlen,
                char *service, 
                size_t servlen, 
                int flags);

// 返回：如果成功则为 0，如果错误则为非零的错误代码。 
~~~

 - 参数 sa 指向大小为 `salen` 字节的套接字地址结构
 - host 指向大小为 `hostlen` 字节的缓冲区
 - service 指向大小为 `servlen` 字节的缓冲区。
 `getnameinfo` 函数将套接字地址结构 `sa` 转换成对应的主机和服务名字符串，并将它们复制到 `host` 和 `serveice` 缓冲区。
 如果 getnameinfo 返回非零的错误代码，应用程序可以调用 gai_strerror 把它转化成字符串。

如果不想要主机名，可以把 `host` 设置为 `NULL`，`hostlen` 设置为 `0`。对服务字段来说也是一样。不过，两者必须设置其中之一。

参数 flags 是一个位掩码，能够修改默认的行为。可以把各种值用 OR 组合起来得到该掩码。下面是两个有用的值：

 - **NI_NUMERICHOST** getnameinfo 默认试图返回 host 中的域名。设置该标志会使该函数返回一个数字地址字符串。
 - **NI_NUMERICSERV** getnameinfo 默认会检查 /etc/services，如果可能，会返回服务名而不是端口号。设置该标志会使该函数跳过査找，简单地返回端口号。

图 11-17 给出了一个简单的程序，称为 HOSTINFO，它使用 getaddrinfo 和 getnameinfo 展示出域名到和它相关联的 IP 地址之间的映射。该程序类似于 11.3.2 节中的 NSLOOKUP 程序。


~~~c
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
~~~
====================================================================================================
`getaddrinfo(argv[1], NULL, &hints, &listp)` 这个函数调用是用于将域名（如 "example.com"）转换为一组IP地址。这个过程称为域名解析，涉及以下几个步骤：

1. **解析域名**：
   - 当程序调用 `getaddrinfo` 并传递一个域名时，这个函数首先检查这个域名是否是一个本地定义的主机名（比如在 `/etc/hosts` 文件中定义的）。如果是，它就直接返回这个主机名对应的地址。

2. **DNS查询**：
   - 如果域名不是本地定义的，`getaddrinfo` 就会进行DNS（域名系统）查询。DNS是互联网上用于将域名转换为IP地址的系统。每当你访问一个域名，比如 "example.com"，你的计算机实际上是先问DNS服务器这个域名对应的IP地址是什么。

3. **联系DNS服务器**：
   - 你的计算机通常会配置有一个或多个DNS服务器的地址。这些可以是由你的互联网服务提供商(ISP)提供，或者是公共DNS服务，如Google的8.8.8.8。
   - `getaddrinfo` 会向配置的DNS服务器发送一个请求，询问提供的域名的IP地址。

4. **接收响应**：
   - DNS服务器查找域名对应的IP地址。如果找到，它会将这个信息作为响应发送回请求的客户端。
   - 如果DNS查询成功，`getaddrinfo` 会接收到一个或多个IP地址。这些地址可能包括IPv4和IPv6地址。

5. **返回地址信息**：
   - `getaddrinfo` 然后将这些IP地址封装在一系列的 `addrinfo` 结构中，这些结构通过 `listp` 指针返回给调用者。
   - 每个 `addrinfo` 结构包含了关于一个特定IP地址的详细信息，如地址家族(AF_INET对于IPv4, AF_INET6对于IPv6)和套接字类型。

通过这个过程，`getaddrinfo` 函数能够将一个人类可读的域名转换为一个或多个可以用于网络通信的IP地址。这个过程对于在互联网上进行数据传输至关重要，因为数据传输依赖于IP地址，而非域名。
====================================================================================================

 - 首先，初始化 hints 结构，使 getaddrinfo 返回我们想要的地址。在这里，我们想查找 32 位的 IP 地址（第 16 行），用作连接的端点（第 17 行）。因为只想 getaddrinfo 转换域名，所以用 service 参数为 NULL 来调用它。

 - 调用 getaddrinfo 之后，会遍历 addrinfo 结构，用 getnameinfo 将每个套接字地址转换成点分十进制地址字符串。遍历完列表之后，我们调用 freeaddrinf。小心地释放这个列表（虽然对于这个简单的程序来说，并不是严格需要这样做的）。

====================================================================================================
函数 getaddrinfo 和 getnameinfo 分别包含了 inet_pton 和 inet_ntop 的功能，提供了更高级别的、独立于任何特殊地址格式的抽象。想看看这到底有多方便，编写 HOSTINFO（图 11-17）的一个版本，用 inet_pton 而不是 getnameinfo 将毎个套接字地址转换成点分十进制地址字符串。

~~~c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#define MAXLINE 8192  // Maximum length of the buffer for storing address strings

int main(int argc, char **argv) {
    struct addrinfo *p, *listp, hints;  
    char buf[MAXLINE];  
    int rc;

    if (argc != 2) {
        fprintf(stderr, "usage: %s <domain name>\n", argv[0]);
        exit(0);
    }

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;       
    hints.ai_socktype = SOCK_STREAM; 

    if ((rc = getaddrinfo(argv[1], NULL, &hints, &listp)) != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(rc));
        exit(1);
    }

    for (p = listp; p; p = p->ai_next) {
        // 使用 inet_ntop 而不是 getnameinfo
        if (inet_ntop(AF_INET, &(((struct sockaddr_in *)(p->ai_addr))->sin_addr), buf, MAXLINE) == NULL) {
            fprintf(stderr, "inet_ntop error\n");
            continue;
        }
        printf("%s\n", buf); 
    }

    freeaddrinfo(listp);

    exit(0);
}
~~~


# 套接字接口的辅助函数

初学时，`getnameinfo` 函数和套接字接口看上去有些可怕。用高级的辅助函数包装一下会方便很多，称为 `open_clientfd` 和 `open_listenfd`，客户端和服务器互相通信时可以使用这些函数。

# open_clientfd 函数

客户端调用 open_clientfd 建立与服务器的连接。

~~~c
#include "csapp.h"

int open_clientfd(char *hostname, char *port);

// 返回：若成功则为描述符，若出错则为 -1。
~~~

open_clientfd 函数建立与服务器的连接，该服务器运行在主机 `hostname` 上，并在端口号 `port` 上监听连接请求。
它返回一个打开的套接字描述符，该描述符准备好了，可以用 Unix I/O 函数做输入和输出。图 11-18 给出了 open_clientfd 的代码。

~~~c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>  // 导入 close 函数
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

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
~~~

我们调用 getaddrinfo，它返回 addrinf 结构的列表，每个结构指向一个套接字地址结构，可用于建立与服务器的连接，该服务器运行在 hostname 上并监听 port 端口。

然后遍历该列表，依次尝试列表中的每个条目，直到调用 socket 和 connect 成功。如果 connect 失败，在尝试下一个条目之前，要小心地关闭套接字描述符。
如果 connect 成功，我们会释放列表内存，并把套接字描述符返回给客户端，客户端可以立即开始用 Unix I/O 与服务器通信了。
====================================================================================================
遍历 `getaddrinfo` 返回的地址列表并尝试连接可能涉及一些时间成本，但这是必要的步骤，尤其是在处理具有多个IP地址（例如，多个A记录或AAAA记录）的域名时。
以下是一些关键点，用于理解这个过程的时间成本和为何它是必要的：

1. **多IP地址的情况**：
   - 一个域名可能解析为多个IP地址，这些地址可能对应不同的服务器或不同的网络接口。这种情况在大型网站和服务中很常见，它们为了负载均衡和容错，会有多个入口点。

2. **连接尝试的时间成本**：
   - 每次尝试建立连接时，`connect` 函数可能会阻塞一段时间，等待网络响应。如果尝试的地址不可达或服务器不响应，这可能会导致显著的延迟。
   - 对于每个地址，如果连接失败，程序会关闭当前套接字并尝试列表中的下一个地址，这个过程会一直持续，直到成功建立连接或地址列表遍历完成。

3. **超时和性能**：
   - 在某些情况下，可能需要考虑为 `connect` 调用设置超时，这可以通过非阻塞套接字和选择性等待（如使用 `select` 或 `poll`）来实现。这样做可以减少连接尝试的时间。
   - 一般来说，只有在第一个可用地址失败时，才会尝试列表中的下一个地址。大多数情况下，第一个或前几个地址就会成功，因此总体延迟可能是可接受的。

4. **错误处理**：
   - 关闭无法连接的套接字是防止资源泄露的重要步骤。在尝试下一个地址之前确保释放资源是良好的编程实践。

5. **最终成功连接**：
   - 一旦成功建立连接，就会释放地址列表的内存，并返回套接字描述符供客户端使用。

虽然这个过程可能看起来时间成本较高，但它是网络编程中的标准实践，旨在提高连接的成功率并处理多地址的情况。在实际应用中，这种方法通常效率足够高，不会造成明显的性能问题。
====================================================================================================

注意，所有的代码都与任何版本的 IP 无关。socket 和 connect 的参数都是用 getaddrinfo 自动产生的，这使得我们的代码干净可移植。

# open_listenfd 函数

调用 open_listenfd 函数，服务器创建一个监听描述符，准备好接收连接请求。

~~~c
#include "csapp.h"

int open_listenfd(char *port);

// 返回：若成功则为描述符，若出错则为 -1。
~~~

`open_listenfd` 函数打开和返回一个监听描述符，这个描述符准备好在端口 `port_h` 接收连接请求。

~~~c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>  // 导入 close 函数
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

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
~~~

 `open_listenfd` 的风格类似于 `open_clientfd`。调用 getaddrinfo，然后遍历结果列表，直到调用 socket 和 bind 成功。
 注意，在第 20 行，我们使用 `setsockopt` 函数（本书中没有讲述）来配置服务器，使得服务器能够被终止、重启和立即开始接收连接请求。
 一个重启的服务器默认将在大约 30 秒内拒绝客户端的连接请求，这严重地阻碍了调试。

 因为我们调用 `getaddrinfo` 时，使用了 AI_PASSIVE 标志并将 host 参数设置为 NULL，每个套接字地址结构中的地址字段会被设置为通配符地址，这告诉内核这个服务器会接收发送到本主机所有 IP 地址的请求。

 最后，我们调用 listen 函数，将 listenfd 转换为一个监听描述符，并返回给调用者。如果 listen 失败，我们要小心地避免内存泄漏，在返回前关闭描述符。

# echo 客户端和服务器的示例

学习套接字接口的最好方法是研究示例代码。图 11-20 展示了一个 echo 客户端的代码。

~~~c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#define MAXLINE 1024

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
~~~

在和服务器建立连接之后，客户端进入一个循环，反复从标准输入读取文本行，发送文本行给服务器，从服务器读取回送的行，并输出结果到标准输出。
当 fgets 在标准输入上遇到 EOF 时，或者因为用户在键盘上键入 Ctrl+D，或者因为在一个重定向的输入文件中用尽了所有的文本行时，循环就终止。

```c
    // 从标准输入读取数据，并发送到服务器，然后读取响应
    while (fgets(buf, MAXLINE, stdin) != NULL) {
        write(clientfd, buf, strlen(buf)); // 发送数据到服务器
        if (read(clientfd, buf, MAXLINE) > 0) {
            fputs(buf, stdout); // 输出服务器响应
        }
    }
```

循环终止之后，客户端关闭描述符。这会导致发送一个 EOF 通知到服务器，当服务器从它的 reo_readlineb 函数收到一个为零的返回码时，就会检测到这个结果。
在关闭它的描述符后，客户端就终止了。既然客户端内核在一个进程终止时会自动关闭所有打开的描述符，第 24 行的 close 就没有必要了。
不过，显式地关闭已经打开的任何描述符是一个良好的编程习惯。

```c
    // 释放地址信息列表
    freeaddrinfo(listp); 
    if (!p) // 所有连接尝试均失败
        return -1;
    else    // 最后一次连接尝试成功
        return clientfd; // 返回客户端套接字描述符
```


图 11-21 展示了 echo 服务器的主程序。

~~~c
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
~~~

在打开监听描述符后，它进入一个无限循环。每次循环都等待一个来自客户端的连接请求，输出已连接客户端的域名和 IP 地址，并调用 echo 函数为这些客户端服务。

在 echo 程序返回后，主程序关闭已连接描述符。一旦客户端和服务器关闭了它们各自的描述符，连接也就终止了。

第 9 行的 clientaddr 变量是一个套接字地址结构，被传递给 accept。在 accept 返回之前，会在 clientaddr 中填上连接另一端客户端的套接字地址。注意，我们将 clientaddr 声明为 `struct sockaddr_storage` 类型，而不是 `struct sockaddr_in` 类型。
根据定义，sockaddr_storage 结构足够大能够装下任何类型的套接字地址，以保持代码的协议无关性。

注意，简单的 echo 服务器一次只能处理一个客户端。这种类型的服务器一次一个地在客户端间迭代，称为迭代服务器（iterative server）。
在第 12 章中，我们将学习如何建立更加复杂的并发服务器（concurrent server），它能够同时处理多个客户端。

最后，图 11-22 展示了 echo 程序的代码，该程序反复读写文本行，直到 rio_readlineb 函数在第 10 行遇到 EOF。

~~~c
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
~~~
图 11-22 读和回送文本行的 echo 函数

> 旁注 - 在连接中 EOF 意味什么？
> EOF 的概念常常使人们感到迷惑，尤其是在因特网连接的上下文中。首先，我们需要理解其实并没有像 EOF 字符这样的一个东西。进一步来说，EOF 是由内核检测到的一种条件。应用程序在它接收到一个由 read 函数返回的零返回码时，它就会发现出 EOF 条件。对于磁盘文件，当前文件位置超出文件长度时，会发生 EOF。对于因特网连接，当一个进程关闭连接它的那一端时，会发生 EOF。连接另一端的进程在试图读取流中最后一个字节之后的字节时，会检测到 EOF