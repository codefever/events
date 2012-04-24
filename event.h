#ifndef EVENT_H_
#define EVENT_H_

#include <sys/time.h>

#define MAX_FD_NUM (1024*10)
#define EMPTY_EV 0
#define READ_EV 0x1
#define WRITE_EV 0x2

typedef void (*handle_t)(int fd, int mask, void *data);

struct event
{
    int fd;
    int mask;
    handle_t handler;
    void *data;
};

struct eventloop
{
    int ep;
    struct event evs[MAX_FD_NUM];
    int maxfd;

    //timer
};

struct eventloop* create_loop();
void delete_loop(struct eventloop *el);

int add_ev(struct eventloop* el, int fd, int mask, handle_t h, void* user);
int remove_ev(struct eventloop* el, int fd);

int runloop_once(struct eventloop *el, struct timeval *tv);

#endif // EVENT_H_
