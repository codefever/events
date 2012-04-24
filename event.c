#include "event.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/epoll.h>

struct eventloop* create_loop()
{
    struct eventloop *el = NULL;

    el = malloc(sizeof(*el));
    if (!el)
    {
        return 0;
    }
    memset(el, 0, sizeof(*el));

    for (int i = 0; i < MAX_FD_NUM; ++i)
    {
        el->evs[i].fd = -1;
    }

    el->maxfd = -1;

    el->ep = epoll_create(1024);

    if (el->ep < 0)
    {
        free(el);
        return 0;
    }

    return el;
}

void delete_loop(struct eventloop *el)
{
    close(el->ep);
    free(el);
}

int add_ev(struct eventloop* el, int fd, int mask, handle_t h, void* user)
{
    struct epoll_event ev;
    memset(&ev, 0, sizeof(ev));
    ev.data.fd = fd;
    ev.events = 0;
    ev.events |= (mask & READ_EV) ? EPOLLIN : 0;
    ev.events |= (mask & WRITE_EV) ? EPOLLOUT : 0;

    if (epoll_ctl(el->ep, EPOLL_CTL_ADD, fd, &ev) < 0)
    {
        return -1;
    }

    el->evs[fd].fd = fd;
    el->evs[fd].mask = mask;
    el->evs[fd].handler = h;
    el->evs[fd].data = user;

    if (fd > el->maxfd)
    {
        el->maxfd = fd;
    }

    return 0;
}

int remove_ev(struct eventloop* el, int fd)
{
    if (epoll_ctl(el->ep, EPOLL_CTL_DEL, fd, 0) < 0)
    {
        return -1;
    }

    memset(&el->evs[fd], 0, sizeof(struct event));
    el->evs[fd].fd = -1;

    if (el->maxfd == fd)
    {
        for (int i = el->maxfd - 1; i >= 0; --i)
        {
            if (el->evs[i].fd >= 0)
            {
                el->maxfd = i;
                return 0;
            }
        }
        el->maxfd = -1;
    }

    return 0;
}

int runloop_once(struct eventloop *el, struct timeval *tv)
{
    if (el->maxfd < 0)
    {
        return 0;
    }

    int processed = 0;
    int timeout = -1;
    if (tv)
    {
        timeout = tv->tv_sec * 1000 + tv->tv_usec / 1000;
    }

    struct epoll_event ev[1024];

    int ret = epoll_wait(el->ep, ev, 1024, timeout);

    if (ret < 0)
    {
        perror("epoll_wait");
        return -1;
    }

    for (int i = 0; i < ret; ++i)
    {
        int fd = ev[i].data.fd;
        int mask = 0;
        mask |= (ev[i].events & EPOLLIN) ? READ_EV : 0;
        mask |= (ev[i].events & EPOLLOUT) ? WRITE_EV: 0;

        assert(el->evs[fd].fd >= 0);

        if (el->evs[fd].handler)
        {
            (*(el->evs[fd].handler))(fd, mask, el->evs[fd].data);
        }

        ++processed;
    }

    return processed;
}

