/* ghosted.c ---
 *
 * Filename: ghosted.c
 * Description: ghost proxy 服务端实现
 * Created: 2014年9月25日 22:28
 * Last-Updated: 2014年9月25日 22:48
 */

#include "ghosted.h"

int main(int argc, char **argv)
{
    int local_port = DEFAULT_LOCAL_PORT;
    int c;

    while ((c = getopt(argc, argv, "l:")) != -1) {
        switch (c) {
                case 'l':
                        local_port = atoi(optarg);
                        break;
                case '?':
                        break;
        }
    }

    LOG("start server on %d.\n", local_port);
    start_server(local_port);
}

void start_server(int local_port)
{
    int server_fd;

    if ((server_fd = create_server_socket(local_port)) < 0) {
        LOG("Cannot run server on %d\n", local_port);
        exit(server_fd);
    }

    server_loop(server_fd);
}

void server_loop(int server_fd)
{
    int client_fd;
    struct sockaddr_in client_address;
    socklen_t client_address_length = sizeof(client_address);

    while (true) {
        client_fd = accept(server_fd, (struct sockaddr *) &client_address, &client_address_length);

        if (fork() == 0) {
            close(server_fd);
            handle_connection(client_fd, client_address);
            exit(0);
        }

        close(client_fd);
    }
}

void handle_connection(int client_fd, struct sockaddr_in client_address)
{
#ifdef DEBUG
    LOG("======== boil new established connection ========\n");
#endif

    char *header_buffer       = NULL;
    terminal_server *terminal = NULL;
    int  terminal_fd          = 0;

    header_buffer = (char *) malloc(MAX_HEADER_BUFFER_SIZE);
    memset(header_buffer, 0, MAX_HEADER_BUFFER_SIZE);
    terminal      = (terminal_server *) malloc(sizeof(terminal_server));

    extract_consumer_http_header(client_fd, header_buffer);
    extract_terminal_server(header_buffer, terminal);

    terminal_fd = create_connection(terminal->terminal_host, terminal->terminal_port);
    if (terminal_fd < 0) {
        LOG("Cannot connect to host [%s: %d]\n", terminal->terminal_host, terminal->terminal_port);
        exit(0);
    }

    if (fork() == 0) {
        forward_consumer_http_header(terminal_fd, header_buffer);

        if (fork() == 0) {
            forward_data(TERMINAL, terminal_fd, client_fd);
            exit(0);
        }

        exit(0);
    }

    close(terminal_fd);
    close(client_fd);
}

int extract_consumer_http_header(int sock_fd, char *buffer)
{
#ifdef DEBUG
    LOG("======== extracting consumer http header ========\n");
#endif

    char line_buffer[MAX_LINE_BUFFER_SIZE];
    char *buffer_cursor   = buffer;
    int  line_buffer_size = 0;

    while (true) {
        memset(line_buffer, 0, MAX_LINE_BUFFER_SIZE);

        line_buffer_size = readline(sock_fd, line_buffer, MAX_LINE_BUFFER_SIZE);
        if (line_buffer_size <= 0) {
            return CLIENT_SOCKET_ERROR;
        }

        if (buffer_cursor + line_buffer_size - buffer <= MAX_HEADER_BUFFER_SIZE) {
            strncpy(buffer_cursor, line_buffer, line_buffer_size);
            buffer_cursor += line_buffer_size;
        } else {
            return HEADER_BUFFER_FULL;
        }

        // 读取到了空行，http header 域结束
        if (strcmp(line_buffer, "\r\n") == 0 || strcmp(line_buffer, "\n") == 0 ) {
            break;
        }
    }

    return 0;
}

/*
 * add http tunnel support
 */
int extract_terminal_server(char *header_buffer, terminal_server *terminal)
{
#ifdef DEBUG
    LOG("======== extracting terminal server from extracted http header ========\n");
#endif

    char *terminal_field      = NULL;
    char *terminal_field_from = NULL;
    char *terminal_field_to   = NULL;
    int  terminal_length      = 0;
    int  terminal_index       = 0;

    const char *terminal_separator = ":";
    char *terminal_token = NULL;

    terminal_field_from = strstr(header_buffer, "Host:");
    if (!terminal_field_from) {
        return BAD_HTTP_PROTOCOL;
    }

    terminal_field_to = strchr(terminal_field_from, '\n');
    if (!terminal_field_to) {
        return BAD_HTTP_PROTOCOL;
    }

    terminal_length = terminal_field_to - terminal_field_from;
    terminal_field  = (char *) malloc(sizeof(char) * terminal_length);
    strncpy(terminal_field, terminal_field_from, terminal_length - 1);
    *(terminal_field + terminal_length) = '\0';

    strtrim(strdup(terminal_field), terminal_field);
    terminal_token = strtok(terminal_field, terminal_separator);

    terminal->terminal_host = (char *) malloc(MAX_HOST_LENGTH);
    terminal->terminal_port = DEFAULT_HTTP_PORT;

    while (terminal_token != NULL) {
        if (terminal_index == 1) {
            strncpy(terminal->terminal_host, terminal_token, strlen(terminal_token));
            *(terminal->terminal_host + strlen(terminal_token) + 1) = '\0';
        } else if(terminal_index == 2) {
            terminal->terminal_port = atoi(terminal_token);
        }

        terminal_index++;
        terminal_token = strtok(NULL, terminal_separator);
    }

#ifdef DEBUG
    LOG("extracted terminal: [%s: %d]\n", terminal->terminal_host, terminal->terminal_port);
#endif

    return 0;
}

void forward_consumer_http_header(int destination_fd, char *header_buffer)
{
#ifdef DEBUG
    LOG("======== forwarding http header from conusmer to terminal ========\n");
#endif

#ifdef DEBUG
    LOG("http header before rewriting:\n");
    LOG("%s\n", header_buffer);
#endif

    rewrite_header(header_buffer);

#ifdef DEBUG
    LOG("http header after rewriting:\n");
    LOG("%s\n", header_buffer);
#endif

    send_data(destination_fd, header_buffer, strlen(header_buffer), DIRECT);
}

void rewrite_header(char *header_buffer)
{
#ifdef DEBUG
    LOG("======== rewriting consumer http header ========\n");
#endif

     const char *url_schema = "http://";
     const char *protocol   = "HTTP/";
     const char url_protocol_separator_literal = ' ';
     const char root_url_rewrite_path          = '/';

     char *url_from               = NULL;
     char *buffer_ending          = NULL;
     char *protocol_from          = NULL;

     char *path_from              = NULL;
     char *url_protocol_separator = NULL;

     url_from      = strstr(header_buffer, url_schema);
     buffer_ending = header_buffer+ strlen(header_buffer);
     protocol_from = strstr(header_buffer, protocol);

     if (url_from) {
         path_from = strchr(url_from + strlen(url_schema), '/');
         if (path_from && (protocol_from > url_from)) {
             /*
              * 例：GET http://www.zhihu.com/explore HTTP/1.1
              * 此种情况下，完整路径需要重写为方法路径
              *
              * XXX: 此处为何如使用 strncpy 替代 memcpy 做拷贝操作将会使
              *      http header Host 域混乱
              */
             memcpy(url_from, path_from, (int) (buffer_ending - path_from));
             *(header_buffer + strlen(header_buffer) - (path_from - url_from)) = '\0';
         } else {
             /*
              * 例：GET http://www.zhihu.com HTTP/1.1
              * 此种情形，将路径重写为 '/' 即可
              */
             url_protocol_separator = strchr(url_from, url_protocol_separator_literal);
             *url_from = root_url_rewrite_path;
             memcpy(url_from + 1, url_protocol_separator, (int) (buffer_ending - url_protocol_separator));
             *(buffer_ending + 1 - (url_protocol_separator - url_from)) = '\0';
         }
     }
}

ssize_t readline(int sock_fd, char *buffer, size_t length)
{
    ssize_t slice_read;
    size_t  total_read;
    char    *buffer_cursor;
    char    character;

    if (length <= 0 || buffer == NULL) {
        errno = EINVAL;
        return -1;
    }

    buffer_cursor = buffer;
    total_read    = 0;

    while (true) {
        slice_read = receive_data(sock_fd, &character, 1, DECODE);

        if (slice_read == -1) {
            if (errno == EINTR) {
                continue;
            } else {
                return -1;
            }
        } else if (slice_read == 0) {
            /* 说明已经遍历到字符结束符 EOF */
            if (total_read == 0) {
                /* 无任何数据可供读取 */
                return 0;
            } else {
                /* 已读取全部数据，需要添加字符串结束符 */
                break;
            }
        } else {
            /* 正常字符串遍历读取过程 */
            if (total_read < length - 1) {
                /*
                 * 至多读取长度为 (length - 1) 的字符串，因此会抛弃
                 * sock_fd 中大于 (length - 1) 的有效字符串
                 */
                *buffer_cursor++ = character;
                total_read++;
            }

            if (character == '\n') {
                break;
            }
        }
    }

    *buffer_cursor = '\0';

    return total_read;
}

/* ghosted.c ends here */
