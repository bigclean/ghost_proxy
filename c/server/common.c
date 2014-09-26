/* common.c ---
 *
 * Filename: common.c
 * Description: ghost proxy 公用函数实现
 * Created: 2014年9月25日 22:28
 * Last-Updated: 2014年9月25日 22:48
 */

#include "common.h"

int create_server_socket(int bind_port)
{
    int    server_fd;
    int    reuseaddr = true;
    struct sockaddr_in server_address;

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        return SERVER_SOCKET_ERROR;
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (const char *) &reuseaddr, sizeof(reuseaddr)) < 0) {
        return SERVER_SETSOCKOPT_ERROR;
    }

    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family      = AF_INET;
    server_address.sin_port        = htons(bind_port);
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(server_fd, (struct sockaddr *) &server_address, sizeof(server_address)) != 0) {
        return SERVER_BIND_ERROR;
    }

    if (listen(server_fd, BACK_LOG) < 0) {
        return SERVER_LISTEN_ERROR;
    }

    return server_fd;
}

int create_connection(char *remote_host, int remote_port)
{
    int sock_fd;
    struct sockaddr_in server_address;
    struct hostent *host;

    if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        return CLIENT_SOCKET_ERROR;
    }

    if ((host = gethostbyname(remote_host)) == NULL) {
        errno = EFAULT;
        return CLIENT_RESOLVE_ERROR;
    }

#ifdef DEBUG
    LOG("connect to remote server: [%s: %d]\n", remote_host, remote_port);
#endif

    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port   = htons(remote_port);
    server_address.sin_addr   = *((struct in_addr *) host->h_addr_list[0]);

    if (connect(sock_fd, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) {
        return CLIENT_CONNECT_ERROR;
    }

    return sock_fd;
}

void forward_data(flow_roler submitter, int source_fd, int destination_fd)
{
    char buffer[BUFFER_SIZE];
    int  num;

    if (submitter == CONSUMER || submitter == TERMINAL) {
        while ((num = receive_data(source_fd, buffer, BUFFER_SIZE, DIRECT)) > 0) {
#ifdef DEBUG
            if (submitter == CONSUMER) {
                LOG("current node roler is consumer\n");
                LOG("consumer request data length is %d\n", num);
            } else if (submitter == TERMINAL) {
                LOG("current node roler is terminal\n");
                LOG("terminal data length is %d\n", num);
            }
#endif

            send_data(destination_fd, buffer, num, ENCODE);
        }
    } else if (submitter == PROXY_SERVER || submitter == PROXY_CLIENT) {
        while ((num = receive_data(source_fd, buffer, BUFFER_SIZE, DECODE)) > 0) {
#ifdef DEBUG
            if (submitter == PROXY_SERVER) {
                LOG("current node roler is ghost proxy server\n");
                LOG("mirrored consuner request data length is %d\n", num);
            } else if (submitter == PROXY_CLIENT) {
                LOG("current node roler is ghost proxy client\n");
                LOG("mirrored terminal data length is %d\n", num);
            }
#endif
            send_data(destination_fd, buffer, num, DIRECT);
        }
    }

    shutdown(source_fd, SHUT_RDWR);
    shutdown(destination_fd, SHUT_RDWR);
}

/*
 * process of encoding to be receieved data
 * TODO: implement encoding function as callbacked
 */
ssize_t receive_data(int receive_fd, char *buffer, int length, flow_flag flag)
{
    int  num            = 0;
    int  index          = 0;
    char *buffer_cursor = NULL;

    num = recv(receive_fd, buffer, length, 0);
    buffer_cursor = buffer;

    if (flag == DECODE && num > 0) {
        for (index = 0; index < num; index++) {
            *buffer_cursor = *buffer_cursor - 1;
            buffer_cursor++;
        }
    }

    return num;
}

ssize_t send_data(int send_fd, char *buffer, int length, flow_flag flag)
{
    int  index          = 0;
    char *buffer_cursor = buffer;

    if (flag == ENCODE && length > 0) {
        for (index = 0; index < length; index++) {
            *buffer_cursor = *buffer_cursor + 1;
            buffer_cursor++;
        }
    }

    return send(send_fd, buffer, length, 0);
}

void strtrim(const char *src, char *dst)
{
    do {
        if (!isspace(*src)) {
            *dst++ = *src;
        }
    } while (*src++);

    *dst = '\0';
}

/* common.c ends here */
