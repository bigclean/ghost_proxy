/* ghost.h ---
 *
 * Filename: ghost.h
 * Description: ghost proxy 客户端头文件
 * Created: 2014年9月25日 22:28
 * Last-Updated: 2014年9月25日 22:48
 */

#ifndef __GHOST_CLIENT_H__
#define __GHOST_CLIENT_H__

#include "common.h"

#define MAX_HOST_LENGTH    256
#define DEFAULT_LOCAL_PORT 8040
#define DEFAULT_PROXY_PORT 4020

struct proxy_server_t {
    char *proxy_host;
    int  proxy_port;
};
typedef struct proxy_server_t proxy_server;

void start_server(int local_port, proxy_server *proxy);

void server_loop(int server_fd, proxy_server *proxy);

void handle_client(int consumer_fd, struct sockaddr_in consumer_address, proxy_server *proxy);

#endif

/* ghost.h ends here */
