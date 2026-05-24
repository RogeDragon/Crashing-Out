CC = gcc
CFLAGS = -Wall -Wextra -g -I./header -Ilibanimate/include

SERVER_SRCS = animate_server.c buffer.c client.c command.c message.c manager.c network.c struct.c
SERVER_OBJS = $(SERVER_SRCS:.c=.o)

CLIENT_SRCS = animate_client.c network.c
CLIENT_OBJS = $(CLIENT_SRCS:.c=.o)

.PHONY: default clean libanimate

default: animate_server animate_client

libanimate:
	@if [ ! -d libanimate ]; then \
		echo "ERROR: libanimate not found" >&2; \
		echo "Download and unzip libanimate.zip from P2 resources" >&2; \
		false; \
	fi

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

animate_server: libanimate $(SERVER_OBJS)
	$(CC) $(CFLAGS) $(SERVER_OBJS) -Llibanimate/lib -lanimate -o $@

animate_client: $(CLIENT_OBJS)
	$(CC) $(CFLAGS) $(CLIENT_OBJS) -o $@

clean:
	rm -f animate_server animate_client $(SERVER_OBJS) $(CLIENT_OBJS)