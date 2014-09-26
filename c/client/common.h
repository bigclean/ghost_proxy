/* common.h ---
 *
 * Filename: common.h
 * Description: ghost proxy 公用函数头文件
 * Created: 2014年9月25日 22:28
 * Last-Updated: 2014年9月25日 22:48
 */

#ifndef __GHOST_COMMON_H__
#define __GHOST_COMMON_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <resolv.h>

#define BUFFER_SIZE             81920
#define BACK_LOG                16

#define SERVER_SOCKET_ERROR     -1
#define SERVER_BIND_ERROR       -2
#define SERVER_SETSOCKOPT_ERROR -3
#define SERVER_LISTEN_ERROR     -4
#define CLIENT_SOCKET_ERROR     -5
#define CLIENT_RESOLVE_ERROR    -6
#define CLIENT_CONNECT_ERROR    -7

#define LOG(fmt...) do { fprintf(stderr, "%s %s ", __DATE__, __TIME__); fprintf(stderr, ##fmt); } while (0)

/**
 * @brief 流量代理中的所有不同角色
 */
enum flow_roler_t {
    CONSUMER,       /**< 消费者，其所指为各种应用程序，一般为浏览器 */
    PROXY_CLIENT,   /**< 代理客户端 */
    PROXY_SERVER,   /**< 代理服务端 */
    TERMINAL        /**< 终端，其所指为各应用程序欲访问的资源，一般为网页 */
};
typedef enum flow_roler_t flow_roler;

/**
 * @brief 流量代理中三种不同流动的方式
 */
enum flow_flag_t {
    DIRECT,   /**< 流量直接流动 */
    ENCODE,   /**< 流量加密流动 */
    DECODE    /**< 流量解密流动 */
};
typedef enum flow_flag_t flow_flag;

/**
 * @brief 本地创建监听服务器
 * @param bind_port 本地监听端口
 * @return 已建立的监听 socket
 * @note  对于代理客户端，需建立服务器监听来自于消费者的连接，而对于代理
 *        服务端，也需建立服务器监听来自于代理客户端的连接。
 */
int create_server_socket(int bind_port);

/**
 * @brief 建立与远端服务器的连接
 *
 * @param remote_host 远端服务器域名或 ip
 * @param remote_port 远端服务器监听端口
 *
 * @return 已建立的监听 socket
 *
 * @note 对于代理客户端，需建立同代理服务端的连接。而对于代理服务端而言，
 *       也需建立同终端的连接。
 */
int create_connection(char *remote_host, int remote_port);

/**
 * @brief 在不同端口之间转发数据
 *
 * @param submitter 转发数据提交者，其值为 ::flow_roler 中的四种不同角色
 * @param source_fd 转发数据来源连接
 * @param destination_fd 转发数据出口连接
 */
void forward_data(flow_roler submitter, int source_fd, int destination_fd);

/**
 * @brief 从数据来源处接收数据
 *
 * @param receive_fd 数据来源连接
 * @param buffer 存放接收到的数据
 * @param length 接收数据的长度
 * @param flag 信息流动的方式，可能取值为 ::flow_flag 中所定义的三种方式
 *
 * @todo 接收数据与发送数据应该被设计为可使用一组回调的加密解密函数从而
 *       解藕函数之间的联系
 */
ssize_t receive_data(int receive_fd, char *buffer, int length, flow_flag flag);

/**
 * @brief 发送数据到数据出口
 *
 * @param send_fd 数据出口连接
 * @param buffer 发送的数据
 * @param length 发送数据长度
 * @param flag 信息流动的方式，可能取值为 ::flow_flag 中所定义的三种方式
 */
ssize_t send_data(int send_fd, char *buffer, int length, flow_flag flag);

void strtrim(const char *dst, char *src);

#endif

/* common.h ends here */
