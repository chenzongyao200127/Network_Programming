# Web Servers
# Web 服务器

迄今为止，我们已经在一个简单的 echo 服务器的上下文中讨论了网络编程。
在这一节里，我们将向你展示如何利用网络编程的基本概念，来创建你自己的虽小但功能齐全的 Web 服务器。

# Web 基础
Web 客户端和服务器之间的交互用的是一个基于文本的应用级协议，叫做 HTTP（hypertext Transfer Protocol，超文本传输协议）。
HTTP 是一个简单的协议。一个 Web 客户端（即浏览器）打开一个到服务器的因特网连接，并且请求某些内容。服务器响应所请求的内容，然后关闭连接。浏览器读取这些内容，并把它显示在屏幕上。

Web 服务和常规的文件检索服务（例如 FTP）有什么区别呢？主要的区别是 Web 内容可以用一种叫做 HTML（Hypertext  MarkupLanguage，超文本标记语言）的语言来编写。一个 HTML 程序（页）包含指令（标记），它们告诉浏览器如何显示这页中的各种文本和图形对象。例如，代码

~~~html
<b> Make me bold! </b>
~~~

告诉浏览器用粗体字类型输出 <b> 和 </b> 标记之间的文本。然而，HTML 真正的强大之处在于一个页面可以包含指针（超链接），这些指针可以指向存放在任何因特网主机上的内容。例如，一个格式如下的 HTML 行

~~~html
<a href="http://www.cmu.edu/index.html">Carnegie Mellon</a>
~~~

告诉浏览器高亮显示文本对象 “Carnegie Mellon”，并且创建一个超链接，它指向存放在 CMU Web 服务器上叫做 index.html 的 HTML 文件。
如果用户单击了这个高亮文本对象，浏览器就会从 CMU 服务器中请求相应的 HTML 文件并显示它。

> 旁注 - 万维网的起源
> 万维网是 Tim Berners-Lee 发明的，他是一位在瑞典物理实验室 CERN（欧洲粒子物理研究所）工作的软件工程师。1989 年，Berners-Lee 写了一个内部备忘录，提出了一个分布式超文本系统，它能连接“用链接组成的笔记的网（web of notes with links）”。
> 提出这个系统的目的是帮助 CERN 的科学家共享和管理信息。在接下来的两年多里，Berners-Lee 实现了第一个 Web 服务器和 Web 浏览器之后，在 CERN 内部以及其他一些网站中，Web 发展出了小规模的拥护者。
> 1993 年一个关键事件发生了，Marc Andreesen（他后来创建了 Netscape）和他在 NCSA 的同事发布了一种图形化的浏览器，叫做 MOSAIC，可以在三种主要的平台上所使用：Unix、Windows 和 Macintosh。在 MOSAIC 发布后，对 Web 的兴趣爆发了，Web 网站以每年 10 倍或更高的数量增长。到 2015 年，世界上已经有超过 975 000 000 个 Web 网站了（源自 Netcraft Web Survey）。

# Web 内容

对于 Web 客户端和服务器而言，内容是与一个 `MIME`（Multipurpose Internet Mail Extensions，多用途的网际邮件扩充协议）类型相关的字节序列。
图 11-23 展示了一些常用的 MIME 类型。

MIME type               Description
-----------------------------------------------------------
text/html               HTML page
text/plain              Unformatted text
application/postscript  Postscript document
image/gif               Binary image encoded in GIF format
image/png               Binary image encoded in PNG format
image/jpeg              Binary image encoded in JPEG forma

Web 服务器以两种不同的方式向客户端提供内容：
 - 取一个磁盘文件，并将它的内容返回给客户端。磁盘文件称为静态内容（`static content`），而返回文件给客户端的过程称为服务静态内容（`serving static content`）。
 - 运行一个可执行文件，并将它的输出返回给客户端。运行时可执行文件产生的输出称为动态内容（`dynamic content`），而运行程序并返回它的输出到客户端的过程称为服务动态内容（`serving dynamic content`）。

每条由 Web 服务器返回的内容都是和它管理的某个文件相关联的。

这些文件中的每一个都有一个唯一的名字，叫做 URL（`Universal Resource Locator`，通用资源定位符）。例如，URL `http://www.google.com:80/index.html`
表示因特网主机 `www.google.com` 上一个称为 `/index.html` 的 `HTML` 文件，它是由一个监听端口 `80` 的 Web 服务器管理的。端口号是可选的，默认为知名的 HTTP 端口 80。可执行文件的 URL 可以在文件名后包括程序参数。“`?`” 字符分隔文件名和参数，而且每个参数都用 “`&`” 字符分隔开。例如，URL
 `http://bluefish.ics.cs.cmu.edu:8000/cgi-bin/adder?15000&213`
标识了一个叫做 `/cgi-bin/adder` 的可执行文件，会带两个参数字符串 `15000` 和 `213` 来调用它。

在事务过程中，客户端和服务器使用的是 URL 的不同部分。
例如，客户端使用前缀 `http://www.google.com:80` 来决定与哪类服务器联系，服务器在哪里，以及它监听的端口号是多少。

服务器使用后缀 `/index.html` 来发现在它文件系统中的文件，并确定请求的是静态内容还是动态内容。

关于服务器如何解释一个 URL 的后缀，有几点需要理解：

 1. 确定一个 URL 指向的是静态内容还是动态内容没有标准的规则。每个服务器对它所管理的文件都有自己的规则。一种经典的（老式的）方法是，确定一组目录，例如 cgi-bin，所有的可执行性文件都必须存放这些目录中。

 2. 后缀中的最开始的那个 “`/`” 不表示 Linux 的根目录。相反，它表示的是被请求内容类型的主目录。
 例如，可以将一个服务器配置成这样：所有的静态内容存放在目录 `/usr/httpd/html` 下，而所有的动态内容都存放在目录 `/usr/httpd/cgi-bin` 下。

 3. 最小的 URL 后缀是 “`/`” 字符，所有服务器将其扩展为某个默认的主页，例如 `/index.html`。这解释了为什么简单地在浏览器中键入一个域名就可以取出一个网站的主页。浏览器在 URL 后添加缺失的 “/”，并将之传递给服务器，服务器又把 “/” 扩展到某个默认的文件名。

====================================================================================================

URL（统一资源定位符）的各个部分具体如下：

1. **协议（Scheme）**:
   - 这是URL的起始部分，通常以“http://”或“https://”表示，指定了用于访问资源的网络协议。常见的协议还包括ftp（用于文件传输）、mailto（用于电子邮件地址）等。

2. **主机名（Host）**:
   - 这部分通常是一个服务器的域名（例如，“`www.example.com`”）或直接是IP地址。它指出了资源所在的服务器位置。

3. **端口号（Port）**（可选）:
   - 端口号紧跟在主机名后，由冒号隔开（例如，“`http://www.example.com:80”中的“:80`”）。如果省略，Web通常默认使用端口80（HTTP）或443（HTTPS）。

4. **路径（Path）**:
   - 路径指定了在服务器上特定资源的位置。例如，在URL“`http://www.example.com/index.html`”中，“/index.html”是路径，它指向服务器上名为“index.html”的文件。

5. **查询（Query）**（可选）:
   - 查询字符串以问号“?”开头，用于提供额外的参数，如“`http://www.example.com/search?q=keyword”中的“?q=keyword`”，其中“q=keyword”是查询参数。

6. **片段（Fragment）**（可选）:
   - 片段以井号“#”开头，通常用于指向网页内的一个标记位置。例如，“`http://www.example.com/index.html#section1`”中的“#section1”指的是“index.html”页面中的一个特定部分。

URL是互联网通信的基础，它们确保了我们可以准确无误地访问和分享网络上的资源。
====================================================================================================

# HTTP 事务

因为 HTTP 是基于在因特网连接上传送的文本行的，我们可以使用 Linux 的 TELNET 程序来和因特网上的任何 Web 服务器执行事务。
对于调试在连接上通过文本行来与客户端对话的服务器来说，TELNET 程序是非常便利的。

例如，图 11-24 使用 TELNET 向 AOL Web 服务器请求主页。

~~~shell
linux> telnet www.aol.com 80            # Client: open connection to server
Trying 205.188.146.23...                # Telnet prints 3 lines to the terminal
Connected to aol.com.
Escape character is '^]'.
GET / HTTP/1.1                          # Client: request line
Host: www.aol.com                       # Client: required HTTP/1.1 header
                                        # Client: empty line terminates headers
HTTP/1.0 200 OK                         # Server: response line
MIME-Version: 1.0                       # Server: followed by five response headers
Date: Mon, 8 Jan 2010 4:59:42 GMT
Server: Apache-Coyote/1.1
Content-Type: text/html                 # Server: expect HTML in the response body
Content-Length: 42092                   # Server: expect 42,092 bytes in the response body
                                        # Server: empty line terminates response headers
<html>                                  # Server: first HTML line in response body
...                                     # Server: 766 lines of HTML not shown
</html>                                 # Server: last HTML line in response body
Connection closed by foreign host.      # Server: closes connection
linux>                                  # Client: closes connection and terminate
~~~

一个服务静态内容的 HTTP 事务

在第 1 行，我们从 Linux shell 运行 TELNET，要求它打开一个到 AOL Web 服务器的连接。TELNET 向终端打印三行输出，打开连接，然后等待我们输入文本（第 5 行）。每次输入一个文本行，并键入回车键，TELNET 会读取该行，在后面加上回车和换行符号（在 C 的表示中为 “\r\n”），并且将这一行发送到服务器。这是和 HTTP 标准相符的，HTTP 标准要求每个文本行都由一对回车和换行符来结束。为了发起事务，我们输入一个 HTTP 请求（第 5 ~ 7 行）。服务器返回 HTTP 响应（第 8 ~ 17 行），然后关闭连接（第 18 行）。

# 1. HTTP 请求

一个 HTTP 请求的组成是这样的：一个请求行（request line）（第 5 行），后面跟随零个或更多个请求报头（request header）（第 6 行），再跟随一个空的文本行来终止报头列表（第 7 行）。一个请求行的形式是

`method URI version`

HTTP 支持许多不同的方法，包括 GET、POST、OPTIONS、HEAD、PUT、DELETE 和 TRACE。我们将只讨论广为应用的 GET 方法，大多数 HTTP 请求都是这种类型的。GET 方法指导服务器生成和返回 URI（Uniform Resource Identifier，统一资源标识符）标识的内容。URI 是相应的 URL 的后缀，包括文件名和可选的参数。

> 实际上，只有当浏览器请求内容时，这才是真的。如果代理服务器请求内容，那么这个 URI 必须是完整的 URL。
====================================================================================================
HTTP中的GET和POST方法是两种常用的请求方式，它们在使用目的、数据传输方式以及一些特性上有所不同：

1. **使用目的**:
   - **GET**: 主要用于`请求数据`。它通常用于从服务器检索信息，如请求网页或查询数据。
   - **POST**: 主要用于`提交数据`。它通常用于向服务器发送信息以进行处理，如提交表单数据、上传文件等。

2. **数据传输方式**:
   - **GET**: 将请求的数据附加在URL之后，以查询字符串的形式存在。例如, `http://www.example.com/index.php?name1=value1&name2=value2`。
   - **POST**: 将数据作为HTTP消息的一部分发送。数据不会显示在URL中，提供了更好的隐私保护。

3. **数据大小限制**:
   - **GET**: 由于数据附加在URL中，因此受URL长度限制，通常不能超过2048个字符。
   - **POST**: 理论上没有数据大小限制，可以发送更大量的数据。

4. **数据类型限制**:
   - **GET**: 只支持ASCII字符，不适合传输二进制数据。
   - **POST**: 可以支持更复杂的数据类型，如二进制数据。

5. **安全性和隐私**:
   - **GET**: 由于数据直接暴露在URL中，更容易受到CSRF（跨站请求伪造）等安全风险的影响，且数据容易被保存在浏览器历史、服务器日志中。
   - **POST**: 相对更安全，因为数据不会被保存在浏览器历史或服务器日志中。

6. **幂等性**:
   - **GET**: 是幂等的，意味着多次执行同一个GET请求，资源状态不会改变。
   - **POST**: 非幂等的，多次执行同一个POST请求，可能会每次都产生不同的结果。

**选择建议**:
- 当你需要从服务器检索数据或执行幂等的操作时，应该使用GET。
- 当你需要向服务器提交数据或执行可能改变服务器状态的操作时，应该使用POST。

合理选择GET和POST对于确保HTTP请求的效率和安全性至关重要。
====================================================================================================

请求行中的 version 字段表明了该请求遵循的 HTTP 版本。最新的 HTTP 版本是 HTTP/1.1【37】。HTTP/1.0 是从 1996 年沿用至今的老版本【6】。HTTP/1.1 定义了一些附加的报头，为诸如缓冲和安全等高级特性提供支持，它还支持一种机制，允许客户端和服务器在同一条持久连接（`persistent connection`）上执行多个事务。在实际中，两个版本是互相兼容的，因为 HTTP/1.0 的客户端和服务器会简单地忽略 HTTP/1.1 的报头。

`GET / HTTP/1.1                          # Client: request line`

总的来说，第 5 行的请求行要求服务器取出并返回 HTML 文件 `/index.html`。
它也告知服务器请求剩下的部分是 `HTTP/1.1` 格式的。

请求报头为服务器提供了额外的信息，例如浏览器的商标名，或者浏览器理解的 MIME 类型。请求报头的格式为

`header-name: header-data`


~~~shell
GET / HTTP/1.1                          # Client: request line
Host: www.aol.com                       # Client: required HTTP/1.1 header
                                        # Client: empty line terminates headers
~~~

针对我们的目的，唯一需要关注的报头是 Host 报头（第 6 行），这个报头在 HTTP/1.1 请求中是需要的，而在 HTTP/1.0 请求中是不需要的。
代理缓存（proxy cache）会使用 Host 报头，这个代理缓存有时作为浏览器和管理被请求文件的原始服务器（origin server）的中介。
客户端和原始服务器之间，可以有多个代理，即所谓的代理链（proxy chain）。
Host 报头中的数据指示了原始服务器的域名，使得代理链中的代理能够判断它是否可以在本地缓存中拥有一个被请求内容的副本。

继续图 11-24 中的示例，第 7 行的空文本行（通过在键盘上键入回车键生成的）终止了报头，并指示服务器发送被请求的 HTML 文件。

~~~shell
linux> telnet www.aol.com 80            # Client: open connection to server
Trying 205.188.146.23...                # Telnet prints 3 lines to the terminal
Connected to aol.com.
Escape character is '^]'.
GET / HTTP/1.1                          # Client: request line
Host: www.aol.com                       # Client: required HTTP/1.1 header
                                        # Client: empty line terminates headers
HTTP/1.0 200 OK                         # Server: response line
MIME-Version: 1.0                       # Server: followed by five response headers
Date: Mon, 8 Jan 2010 4:59:42 GMT
Server: Apache-Coyote/1.1
Content-Type: text/html                 # Server: expect HTML in the response body
Content-Length: 42092                   # Server: expect 42,092 bytes in the response body
                                        # Server: empty line terminates response headers
<html>                                  # Server: first HTML line in response body
...                                     # Server: 766 lines of HTML not shown
</html>                                 # Server: last HTML line in response body
Connection closed by foreign host.      # Server: closes connection
linux>                                  # Client: closes connection and terminate
~~~

## 2. HTTP 响应

HTTP 响应和 HTTP 请求是相似的。
一个 HTTP 响应的组成是这样的：
1. 一个响应行（response line）（第 8 行），
2. 后面跟随着零个或更多的响应报头（response header）（第 9 ~ 13 行），
3. 再跟随一个终止报头的空行（第 14 行），
4. 再跟随一个响应主体（response body）（第 15 ~ 17 行）。

一个响应行的格式是

`version status-code status-message`

version 字段描述的是响应所遵循的 HTTP 版本。状态码（status-code）是一个 3 位的正整数，指明对请求的处理。状态消息（status message）给出与错误代码等价的英文描述。图 11-25 列出了一些常见的状态码，以及它们相应的消息。

HTTP状态码是由服务器返回给客户端（通常是浏览器）的三位数字代码，用于表示特定的请求状态。以下是一些常见的HTTP状态码及其相应的消息：
   1. **1xx - 信息性状态码**
      - `100 Continue`：客户端应继续其请求
      - `101 Switching Protocols`：服务器根据客户端的请求切换协议

   2. **2xx - 成功状态码**
      - `200 OK`：请求成功
      - `201 Created`：请求已成功，并且服务器创建了新的资源
      - `202 Accepted`：服务器已接受请求，但尚未处理
      - `204 No Content`：服务器成功处理了请求，但没有返回任何内容

   3. **3xx - 重定向状态码**
      - `301 Moved Permanently`：请求的网页已永久移动到新位置
      - `302 Found`：服务器目前从不同位置的网页响应请求，但请求者应继续使用原有位置来进行后续请求
      - `304 Not Modified`：自上次请求后，请求的网页未修改过

   4. **4xx - 客户端错误状态码**
      - `400 Bad Request`：服务器无法理解请求的格式
      - `401 Unauthorized`：请求未经授权
      - `403 Forbidden`：服务器拒绝请求
      - `404 Not Found`：服务器找不到请求的网页
      - `408 Request Timeout`：请求超时

   5. **5xx - 服务器错误状态码**
      - `500 Internal Server Error`：服务器遇到错误，无法完成请求
      - `501 Not Implemented`：服务器不具备完成请求的功能
      - `503 Service Unavailable`：服务器目前无法使用（由于超载或停机维护）

   这些状态码有助于诊断和理解Web应用程序或API中的各种问题。

第 9 ~ 13 行的响应报头提供了关于响应的附加信息。针对我们的目的，两个最重要的报头是 `Content-Type`（第 12 行），它告诉客户端响应主体中内容的 `MIME 类型`；以及 `Content-Length`（第 13 行），用来指示`响应主体的字节大小`。

第 14 行的终止响应报头的空文本行，其后跟随着响应主体，响应主体中包含着被请求的内容。


# 服务动态内容

如果我们停下来考虑一下，一个服务器是如何向客户端提供动态内容的，就会发现一些问题。
 - 例如，客户端如何将程序参数传递给服务器？
 - 服务器如何将这些参数传递给它所创建的子进程？
 - 服务器如何将子进程生成内容所需要的其他信息传递给子进程？
 - 子进程将它的输出发送到哪里？

====================================================================================================
当考虑服务器如何向客户端提供动态内容时，涉及到客户端与服务器的交互，以及服务器如何处理这些请求并生成动态内容。下面是对您提出的问题的详细解答：

1. **客户端如何将程序参数传递给服务器？**
   - 通过**HTTP请求**：客户端（如浏览器或API调用者）可以通过HTTP请求（如GET或POST请求）向服务器传递参数。
   - **URL参数**：在GET请求中，参数通常附加在URL后面，形如`?key1=value1&key2=value2`。
   - **请求体**：在POST请求中，参数可以在请求体中发送，特别是对于表单提交或发送JSON/XML数据。

2. **服务器如何将这些参数传递给它所创建的子进程？**
   - **环境变量**：服务器可以将参数设置为环境变量，然后由子进程读取。
   - **命令行参数**：服务器可以在启动子进程时，将参数作为命令行参数传递。
   - **标准输入**：服务器可以通过标准输入流（stdin）将参数传递给子进程。

3. **服务器如何将子进程生成内容所需要的其他信息传递给子进程？**
   - 除了环境变量、命令行参数和标准输入，服务器还可以使用配置文件、数据库或共享内存等机制来传递其他所需信息给子进程。

4. **子进程将它的输出发送到哪里？**
   - **标准输出和错误输出**：子进程通常将其输出发送到标准输出（stdout）和标准错误（stderr）。服务器程序负责捕获这些输出。
   - **文件或数据库**：在某些情况下，子进程可能会将输出写入文件或数据库，服务器随后从中读取。
   - **直接响应**：在Web服务中，子进程的输出可以直接格式化为HTTP响应，然后发送回客户端。

这个过程涉及到多个层面的交互和数据传递，确保服务器能够处理请求并提供所需的动态内容。在现代Web开发中，这些过程通常是通过Web框架和应用程序接口自动处理的，使得开发更加高效和安全。
====================================================================================================

一个称为 CGI（`Common Gateway Interface`，通用网关接口）的*实际标准*的出现解决了这些问题。

1. 客户端如何将程序参数传递给服务器
GET 请求的参数在 URI 中传递。正如我们看到的，一个 “?” 字符分隔了文件名和参数，而每个参数都用一个 “&” 字符分隔开。参数中不允许有空格，而必须用字符串 “％20” 来表示。对其他特殊字符，也存在着相似的编码。

   - 通过**HTTP请求**：客户端（如浏览器或API调用者）可以通过HTTP请求（如GET或POST请求）向服务器传递参数。
   - **URL参数**：在GET请求中，参数通常附加在URL后面，形如`?key1=value1&key2=value2`。
   - **请求体**：在POST请求中，参数可以在请求体中发送，特别是对于表单提交或发送JSON/XML数据。

> 旁注 - 在 HTTP POST 请求中传递参数
> HTTP POST 请求的参数是在请求主体中而不是 URI 中传递的。

2. 服务器如何将参数传递给子进程
在服务器接收一个如下的请求后
   `GET /cgi-bin/adder?15000&213 HTTP/1.1`

它调用 fork 来创建一个子进程，并调用 execve 在子进程的上下文中执行 `/cgi-bin/adder` 程序。
像 adder 这样的程序，常常被称为 CGI 程序，因为它们遵守 CGI 标准的规则。
而且，因为许多 CGI 程序是用 Perl 脚本编写的，所以 CGI 程序也常被称为 CGI 脚本。
在调用 execve 之前，子进程将 CGI 环境变量 QUERY_STRING 设置为 “15000&213”，adder 程序在运行时可以用 Linux getenv 函数来引用它。

   - **环境变量**：服务器可以将参数设置为环境变量，然后由子进程读取。
   - **命令行参数**：服务器可以在启动子进程时，将参数作为命令行参数传递。
   - **标准输入**：服务器可以通过标准输入流（stdin）将参数传递给子进程。

3. 服务器如何将其他信息传递给子进程
CGI 定义了大量的其他环境变量，一个 CGI 程序在它运行时可以设置这些环境变量。图 11-26 给出了其中的一部分。

CGI（Common Gateway Interface）定义了一系列环境变量，这些变量在CGI程序运行时由Web服务器设置。
这些环境变量提供了关于HTTP请求和服务器环境的信息，使得CGI程序可以根据这些信息来生成适当的响应。
下面是一些常见的CGI环境变量的例子：

1. **`REQUEST_METHOD`**：显示提交HTTP请求的方法（如`GET`、`POST`）。
2. **`QUERY_STRING`**：包含URL中的查询字符串。在使用GET方法时，这些是附加在URL后的键值对。
3. **`CONTENT_TYPE`**：如果HTTP请求包含正文（如POST请求），这个变量描述了正文的媒体类型。
4. **`CONTENT_LENGTH`**：如果HTTP请求包含正文，这个变量描述了正文的长度。
5. **`REMOTE_ADDR`**：显示发起请求的客户端的IP地址。
6. **`REMOTE_HOST`**：显示发起请求的客户端的主机名。
7. **`SERVER_NAME`**：Web服务器的主机名。
8. **`SERVER_PORT`**：Web服务器监听的端口号。
9. **`SCRIPT_NAME`**：执行的CGI脚本的名称。
10. **`PATH_INFO`**：脚本之后的额外的路径信息。
11. **`PATH_TRANSLATED`**：`PATH_INFO`的文件系统路径版本。
12. **`HTTP_`前缀的变量**：对于任何HTTP请求头，Web服务器都会创建一个环境变量，其名称是请求头的名称，前面加上`HTTP_`。例如，`HTTP_USER_AGENT`环境变量包含了请求的`User-Agent`头的值。

这些环境变量为CGI程序提供了处理请求所需的上下文信息，从而允许程序根据不同的请求条件产生定制化的响应。由于这些变量是由Web服务器设置的，CGI程序可以跨不同的编程语言和平台使用这些信息。

   - 除了环境变量、命令行参数和标准输入，服务器还可以使用配置文件、数据库或共享内存等机制来传递其他所需信息给子进程。


4. 子进程将它的输出发送到哪里

一个 CGI 程序将它的动态内容发送到标准输出。在子进程加载并运行 CGI 程序之前，它使用 Linux dup2 函数将标准输出重定向到和客户端相关联的已连接描述符。因此，任何 CGI 程序写到标准输出的东西都会直接到达客户端。

注意，因为父进程不知道子进程生成的内容的类型或大小，所以子进程就要负责生成 `Content-type` 和 `Content-length` 响应报头，以及终止报头的空行。

   - **标准输出和错误输出**：子进程通常将其输出发送到标准输出（stdout）和标准错误（stderr）。服务器程序负责捕获这些输出。
   - **文件或数据库**：在某些情况下，子进程可能会将输出写入文件或数据库，服务器随后从中读取。
   - **直接响应**：在Web服务中，子进程的输出可以直接格式化为HTTP响应，然后发送回客户端。

图 11-27 展示了一个简单的 CGI 程序，它对两个参数求和，并返回带结果的 HTML 文件给客户端。

~~~c
#include "csapp.h"

int main(void) {
    char *buf, *p;
    char arg1[MAXLINE], arg2[MAXLINE], content[MAXLINE];
    int n1 = 0, n2 = 0;

    /* Extract the two arguments */
    if ((buf = getenv("QUERY_STRING")) != NULL) {
        p = strchr(buf, ’&’);
        *p = ’\0’;
        strcpy(arg1, buf);
        strcpy(arg2, p + 1);
        n1 = atoi(arg1);
        n2 = atoi(arg2);
    }
    
    /* Make the response body */
    sprintf(content, "QUERY_STRING=%s", buf);
    sprintf(content, "Welcome to add.com: ");
    sprintf(content, "%sTHE Internet addition portal.\r\n<p>", content);
    sprintf(content, "%sThe answer is: %d + %d = %d\r\n<p>",
            content, n1, n2, n1 + n2);
    sprintf(content, "%sThanks for visiting!\r\n", content);
    
    /* Generate the HTTP response */
    printf("Connection: close\r\n");
    printf("Content-length: %d\r\n", (int)strlen(content));
    printf("Content-type: text/html\r\n\r\n");
    printf("%s", content);
    fflush(stdout);

    exit(0);
}
~~~

~~~c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXLINE 4096

int main(void) {
    char *buf, *p;
    char arg1[MAXLINE], arg2[MAXLINE], content[MAXLINE];
    int n1 = 0, n2 = 0;

    /* Extract the two arguments */
    buf = getenv("QUERY_STRING");
    if (buf != NULL) {
        p = strchr(buf, '&');
        if (p != NULL) {
            *p = '\0';
            strncpy(arg1, buf, MAXLINE - 1);
            strncpy(arg2, p + 1, MAXLINE - 1);
            n1 = atoi(arg1);
            n2 = atoi(arg2);
        }
    }


    char temp[MAXLINE];

    /* Make the response body */
    snprintf(temp, MAXLINE, "QUERY_STRING=%s", buf);
    strncat(content, temp, MAXLINE - strlen(content) - 1);

    snprintf(temp, MAXLINE, "Welcome to add.com: ");
    strncat(content, temp, MAXLINE - strlen(content) - 1);

    snprintf(temp, MAXLINE, "%sTHE Internet addition portal.\r\n<p>", content);
    strncpy(content, temp, MAXLINE);

    snprintf(temp, MAXLINE, "%sThe answer is: %d + %d = %d\r\n<p>",
            content, n1, n2, n1 + n2);
    strncpy(content, temp, MAXLINE);

    snprintf(temp, MAXLINE, "%sThanks for visiting!\r\n", content);
    strncpy(content, temp, MAXLINE);

    /* Generate the HTTP response */
    printf("Connection: close\r\n");
    printf("Content-length: %d\r\n", (int)strlen(content));
    printf("Content-type: text/html\r\n\r\n");
    printf("%s", content);
    fflush(stdout);

    return 0;
}
~~~

图 11-28 展示了一个 HTTP 事务，它根据 adder 程序提供动态内容。

~~~powershell
linux> telnet kittyhawk.cmcl.cs.cmu.edu 8000        # Client: open connection
Trying 128.2.194.242...
Connected to kittyhawk.cmcl.cs.cmu.edu.
Escape character is ’^]’.
GET /cgi-bin/adder?15000&213 HTTP/1.0               # Client: request line
                                                    # Client: empty line terminates headers
HTTP/1.0 200 OK                                     # Server: response line
Server: Tiny Web Server                             # Server: identify server
Content-length: 115                                 # Adder: expect 115 bytes in response body
Content-type: text/html                             # Adder: expect HTML in response body
                                                    # Adder: empty line terminates headers
Welcome to add.com: THE Internet addition portal.   # Adder: first HTML line
<p>The answer is: 15000 + 213 = 15213               # Adder: second HTML line in response body
<p>Thanks for visiting!                             # Adder: third HTML line in response body
Connection closed by foreign host.                  # Server: closes connection
linux>                                              # Client: closes connection and terminates
~~~

图 11-28 一个提供动态 HTML 内容的 HTTP 事务

> 旁注 - 将 HTTP POST 请求中的参数传递给 CGI 程序
> 对于 POST 请求，子进程也需要重定向标准输入到已连接描述符。然后，CGI 程序会从标准输入中读取请求主体中的参数。

====================================================================================================
Q: 在 10.11 节中，我们警告过你关于在网络应用中使用 C 标准 I/O 函数的危险。然而，图 11-27 中的 CGI 程序却能没有任何问题地使用标准 I/O。为什么呢？

A: 标准 I/O 能在 CGI 程序里工作的原因是，在子进程中运行的 CGI 程序不需要显式地关闭它的输入输出流。当子进程终止时，内核会自动关闭所有描述符。

这个问答是关于在网络应用中使用C标准I/O函数的一些注意事项。在网络编程中，尤其是涉及到套接字（sockets）的情况下，直接使用C标准I/O（例如`printf`、`scanf`、`fgets`、`fputs`等）可能会引起一些问题。这是因为标准I/O库是为了处理标准输入输出和文件I/O设计的，它在内部使用缓冲区来提高文件I/O的效率。**然而，当涉及到网络套接字时，这种缓冲机制可能导致数据在被发送或接收之前延迟，从而引起通信问题。**

然而，在CGI程序的上下文中，情况有所不同。CGI程序通常以子进程的形式运行，在Web服务器和客户端之间充当中间人。这些子进程读取环境变量和标准输入（来自Web服务器），执行一些处理，然后将输出写入标准输出，这通常会被Web服务器捕获并发送回客户端。

现在，为什么在CGI程序中使用标准I/O是安全的呢？原因有两个：

1. **子进程的生命周期**：一个CGI子进程通常处理单个请求，然后终止。在子进程终止时，操作系统（内核）会自动关闭该进程打开的所有文件描述符（包括标准输入、输出和错误）。这意味着不需要显式地关闭流或刷新输出缓冲区，因为这将在进程结束时自动完成。

2. **简化的通信模式**：与直接在网络套接字上进行读写相比，CGI程序的输入输出模式相对简单。它们通常只是接收请求数据，处理它，然后生成响应。这种单向流动的数据通常不会受到标准I/O缓冲机制的负面影响。

综上所述，虽然在网络套接字编程中直接使用标准I/O可能存在问题，但在CGI程序中使用标准I/O是可行的，因为操作系统会在子进程结束时处理相关的资源清理和缓冲区刷新。



在讨论网络编程时，特别是涉及到使用套接字（sockets）时，使用C标准I/O函数（如`printf`, `fgets`, `fread`, `fwrite`等）可能会引起数据传输的延迟问题。这主要是因为标准I/O函数使用了内部缓冲机制，旨在提高文件读写操作的效率。

然而，当这些函数用于网络通信时，它们的缓冲特性可能会导致一些不期望的行为，尤其是数据传输的延迟。

### 标准I/O的缓冲机制
- **完全缓冲**：数据被存储在缓冲区中，直到缓冲区满了才进行实际的I/O操作。
- **行缓冲**：当缓冲区遇到换行符时，进行I/O操作。
- **无缓冲**：数据立即进行I/O操作，没有延迟。

对于标准文件I/O，这种缓冲是合适的，因为它减少了磁盘操作的次数，提高了效率。但是，当用于套接字时，这种缓冲可能导致问题。

### 延迟的示例
假设你正在编写一个基于套接字的聊天程序，你希望用户输入的每行文本立即发送到另一端。

**使用 `fgets` 和 `printf`**：
```c
// 服务器从客户端读取数据
fgets(buffer, sizeof(buffer), socket_stream);
// 服务器处理数据并响应
printf("Echo: %s", buffer);
```

这里，服务器使用 `fgets` 从套接字读取数据，并使用 `printf` 发送回复。但是，由于 `printf` 是行缓冲的，如果没有遇到换行符，输出可能不会立即发送。这意味着如果服务器试图发送没有换行符的数据，该数据可能会停留在输出缓冲区中，直到缓冲区满或遇到换行符才会实际发送。这就造成了延迟。

### 解决方法
在处理套接字时，更好的做法是使用直接的套接字I/O函数，如 `send`, `recv` (在C语言中)。这些函数不使用标准I/O的缓冲机制，而是直接将数据发送或接收到套接字，这可以减少延迟，提高网络通信的实时性。

```c
// 直接使用套接字发送数据
send(socket_fd, buffer, strlen(buffer), 0);
```

在这个示例中，`send` 函数直接将数据发送到套接字，无需等待缓冲区填满或遇到特定的字符。这就避免了上述提到的延迟问题，使得网络通信更加及时和可靠。

通俗解释：

想象一下，标准I/O函数（比如`printf`或`fgets`）就像是一个有弹簧的信箱。当你往里面投信时，信并不是立刻被送出去的。这个信箱会等到它满了或者到了特定的时间（比如每天下午三点清空一次），才会一次性把所有信件送出去。这在平时邮寄信件时没什么问题，因为我们不需要信件立刻就被送达。

但是，如果你用这样的信箱来做即时通讯，就会遇到问题。比如你用微信发消息，你希望你的消息一发出去，对方就立刻收到，而不是等信箱满了或者到了特定时间才发送。如果用了类似那个有弹簧的信箱，你的消息可能就会延迟发送，对方也就迟迟收不到。

在网络编程中，使用标准I/O函数就像是在用那个有弹簧的信箱。数据可能会在一个缓冲区（信箱）里等待，直到满了或达到某个条件（比如换行符）才会发送。这在网络通信中会造成延迟，因为我们通常希望数据一产生就立即发送。

所以，在做网络通信时，我们更倾向于使用像`send`和`recv`这样的直接套接字操作函数。这就像是直接把消息递给对方，没有等待，没有延迟，通信更及时。