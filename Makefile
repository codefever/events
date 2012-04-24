all:	svr cli
svr:	event.h event.c server.c
	gcc event.c server.c -std=c99 -o svr
cli:	client.c
	gcc client.c -std=c99 -o cli
