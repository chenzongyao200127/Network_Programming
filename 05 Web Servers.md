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
