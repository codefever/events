#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include "event.h"

static void func(int sig)
{
    printf("signal: %d\n", sig);
}

static int set_nonblocking(int fd)
{
    int flag = fcntl(fd, F_GETFL);
    return fcntl(fd, F_SETFL, flag|O_NONBLOCK);
}

static void recv_handler(int fd, int mask, void *data)
{
    struct eventloop *el = data;
    char buf[1024];

    while (1)
    {
        ssize_t nret = read(fd, buf, sizeof(buf));
        if (nret < 0)
        {
            if (errno == EINTR || errno == EAGAIN)
            {
                continue;
            }
        }
        else if (nret == 0)
        {
            remove_ev(el, fd);
            close(fd);
            printf("close: %d\n", fd);
            return;
        }

        break;
    }

    printf("%s\n", buf);
}

static void listen_handler(int fd, int mask, void *data)
{
    struct eventloop *el = data;
    int conn = -1;

    while (1)
    {
        conn = accept(fd, 0, 0);

        if (conn < 0)
        {
            if (errno == EINTR || errno == EAGAIN)
            {
                continue;
            }
            else
            {
                return;
            }
        }
        break;
    }

    add_ev(el, conn, READ_EV, recv_handler, el);
}

int main(int argc, const char *argv[])
{
    signal(SIGUSR1, func);

    struct eventloop *el = create_loop();
    if (!el)
    {
        return -1;
    }

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        delete_loop(el);
        return -1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("0.0.0.0");
    addr.sin_port = htons(8888);

    if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0)
    {
        delete_loop(el);
        close(sock);
        return -1;
    }

    set_nonblocking(sock);
    listen(sock, 10);

    add_ev(el, sock, READ_EV, listen_handler, (void*)el);

    while (1)
    {
        runloop_once(el, 0);
    }

    delete_loop(el);
    close(sock);
    return 0;
}

