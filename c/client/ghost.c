/* ghost.c ---
 *
 * Filename: ghost.c
 * Description: ghost proxy 客户端实现
 * Created: 2014年9月25日 22:28
 * Last-Updated: 2014年9月25日 22:48
 */

#include "ghost.h"

int main(int argc, char **argv)
{
    int          local_port;
    proxy_server *proxy;
    int          c;

    local_port        = DEFAULT_LOCAL_PORT;

    proxy             = (proxy_server *) malloc(sizeof(proxy_server));
    proxy->proxy_host = (char *) malloc(MAX_HOST_LENGTH);
    strcpy(proxy->proxy_host, "127.0.0.1");
    proxy->proxy_port = DEFAULT_PROXY_PORT;

    while ((c = getopt(argc, argv, "l:h:p:")) != -1) {
        switch(c) {
                case 'l':
                        local_port = atoi(optarg);
                        break;
                case 'h':
                        proxy->proxy_host = optarg;
                        break;
                case 'p':
                        proxy->proxy_port = atoi(optarg);
                        break;
                case '?':
                        break;
        }
    }

    /* TODO: add report function */
    LOG("start server on %d and next hop is %s:%d\n", local_port, proxy->proxy_host, proxy->proxy_port);
    start_server(local_port, proxy);
}

void start_server(int local_port, proxy_server *proxy)
{
    int server_fd;

    if ((server_fd = create_server_socket(local_port)) < 0) {
        LOG("Cannot run server on %d\n", local_port);
        exit(server_fd);
    }

    server_loop(server_fd, proxy);
}

void server_loop(int server_fd, proxy_server *proxy)
{
    int consumer_fd;
    struct sockaddr_in consumer_address;
    socklen_t consumer_address_length = sizeof(consumer_address);

    while (true) {
        consumer_fd = accept(server_fd, (struct sockaddr *) &consumer_address, &consumer_address_length);

        if (fork() == 0) {
            close(server_fd);
            handle_client(consumer_fd, consumer_address, proxy);
            exit(0);
        }

        close(consumer_fd);
    }
}

void handle_client(int consumer_fd, struct sockaddr_in consumer_address, proxy_server *proxy)
{
#ifdef DEBUG
    LOG("======== boil new established connection ========\n");
#endif

    int proxy_fd;

    proxy_fd = create_connection(proxy->proxy_host, proxy->proxy_port);
    if (proxy_fd < 0) {
        LOG("Cannot connect to host [%s: %d]\n", proxy->proxy_host, proxy->proxy_port);
        exit(0);
    }

    if (fork() == 0) {
        forward_data(CONSUMER, consumer_fd, proxy_fd);
        exit(0);
    }

    if (fork() == 0) {
        forward_data(PROXY_CLIENT, proxy_fd, consumer_fd);
        exit(0);
    }

    close(proxy_fd);
    close(consumer_fd);
}

/* ghost.c ends here */
