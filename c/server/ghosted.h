/* ghosted.h ---
 *
 * Filename: ghosted.h
 * Description: ghost proxy 服务端声明
 * Created: 2014年9月25日 22:28
 * Last-Updated: 2014年9月25日 22:48
 */

#ifndef __GHOST_SERVER_H__
#define __GHOST_SERVER_H__

#include "common.h"

#define DEFAULT_LOCAL_PORT     4020
#define DEFAULT_HTTP_PORT      80

#define MAX_HOST_LENGTH        1024
#define MAX_HEADER_BUFFER_SIZE 8192
#define MAX_LINE_BUFFER_SIZE   2048

#define HEADER_BUFFER_FULL     -8
#define BAD_HTTP_PROTOCOL      -9

struct terminal_server_t {
    char *terminal_host;
    int  terminal_port;
};
typedef struct terminal_server_t terminal_server;

/**
 * @brief 启动 ghost proxy server 进程
 * @param local_port ghost proxy server 本地监听端口
 */
void start_server(int local_port);

/**
 * @brief ghost proxy server 循环
 * @param server_fd ghost proxy server 服务端监听连接
 */
void server_loop(int server_fd);

/**
 * @brief 处理来自于代理客户端的连接
 *
 * @param client_fd 与代理客户端建立的连接
 * @param client_address 代理客户端网络地址
 */
void handle_connection(int client_fd, struct sockaddr_in client_address);

/**
 * @brief 从 http 被动连接中解析出 http 头部域
 *
 * @param sock_fd 与客户端被动建立的连接
 * @param buffer  存放已解析的 http 头部域的字符串缓冲区
 */
int extract_consumer_http_header(int sock_fd, char *buffer);

/**
 * @brief 从 http 头部域中解析出终端主机域名与端口
 *
 * @param header_buffer http 头部域缓冲区
 * @param terminal      存放从 http 头部域解析出的终端主机
 */
int extract_terminal_server(char *header_buffer, terminal_server *terminal);

/**
 * @brief 转发代理 http 头部域至远端目的主机
 *
 * @param destination_fd 与目的主机之间所建立的连接
 * @param header_buffer  需被转发的代理 http 头部域
 */
void forward_consumer_http_header(int destination_fd, char *header_buffer);

/**
 * @brief 重写代理 http 头部域
 * @param header_buffer  需被转发的代理 http 头部域
 */
void rewrite_header(char *header_buffer);

/**
 * @brief 从建立的 socket 连接中以行来读取数据
 *
 * @param sock_fd 所读取数据来源的连接
 * @param buffer  存放单次读取的字符串缓冲区
 * @param lenth    单次读取所容纳的最大长度
 *
 * @return ssize_t 本地读取的字符串的长度，小于等于 length - 1
 *
 * @note 当所读取的字符串的长度超出 (length - 1) 时，其所超出的部分将会被舍弃
 *
 * @see http://man7.org/tlpi/code/online/dist/sockets/read_line.c.html
 */
ssize_t readline(int sock_fd, char *buffer, size_t length);

#endif

/* ghosted.h ends here */
