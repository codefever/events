#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

static int set_nonblocking(int fd)
{
    int flag = fcntl(fd, F_GETFL);
    return fcntl(fd, F_SETFL, flag|O_NONBLOCK);
}

int main(int argc, const char *argv[])
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        perror("socket");
        return -1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(8888);

    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0)
    {
        perror("connect");
        close(sock);
        return -1;
    }

    char *str = "hello world";
    ssize_t nret = write(sock, str, strlen(str)+1);
    if (nret < 0)
    {
        perror("write");
    }

    close(sock);
    return 0;
}
