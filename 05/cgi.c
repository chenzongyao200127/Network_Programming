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
