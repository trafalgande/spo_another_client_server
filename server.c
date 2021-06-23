#include "api.h"
#include "cmd_parser.h"

#define D(...) fprintf(new_stream, __VA_ARGS__)

int main() {
//    init_db();
//    api_t * api_ = parse_cmd_to_api_struct("{\"cmd\":\"create\",\"param\":\"node\",\"paths_n\":1,\"paths\":[{\"actualPath\":\"a\"}],\"nelem\":{\"elementKey\":\"b\",\"init_values_n\":1,\"element_init_values\":[{\"init_value_key\":\"p\",\"utf_value\":\"p\"}]},\"from_root\":1}");
//    char *res4 = api_create(api_->paths[0]->actualPath, api_->elem_n,api_->elems, NULL);
//    api_t * api_ = parse_cmd_to_api_struct("{\"cmd\":\"read\",\"paths_n\":1,\"paths\":[{\"actualPath\":\"a\",\"conds\":[{\"key\":\"r\",\"sign\":\"\\u003d\",\"int_value\":2}],\"operators\":[],\"cond_n\":1}],\"from_root\":1}");
//    printf("%s\n", res4);
//    api_ = parse_cmd_to_api_struct("{\"cmd\":\"read\",\"paths_n\":1,\"paths\":[{\"actualPath\":\"*\",\"conds\":[{\"key\":\"r\",\"sign\":\"\\u003d\",\"int_value\":2}],\"operators\":[],\"cond_n\":1}],\"from_root\":1}");
//    res4 = api_read(api_->path_n, api_->paths, api_->fromRoot, NULL);
//    printf("%s\n", res4);


    int sock;
    struct sockaddr_in name;
    char buf[MAX_MSG_LENGTH] = {0};
    char response[MAX_MSG_LENGTH] = {0};


    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) perro("opening socket");

    int optval = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);

    name.sin_family = AF_INET;
    name.sin_addr.s_addr = INADDR_ANY;
    name.sin_port = htons(PORT);
    if (bind(sock, (void *) &name, sizeof(name))) perro("binding tcp socket");
    if (listen(sock, 1) == -1) perro("listen");

    struct sockaddr cli_addr;
    int cli_len = sizeof(cli_addr);
    int new_socket, new_fd, pid;
    FILE *new_stream;

    new_fd = dup(STDERR_FILENO) == -1;
    if (new_fd) perro("dup");
    new_stream = fdopen(new_fd, "w");
    setbuf(new_stream, NULL); // sin buffering

    D("Initializing server...\n");
    new_socket = accept(sock, &cli_addr, &cli_len);
    while (new_socket) {
        D("Client connected.\nForking... ");
        if (pid = fork()) D("child pid = %d.\n", pid);
        else {
            pid = getpid();
            if (new_socket < 0) perro("accept");
            if (dup2(new_socket, STDOUT_FILENO) == -1) perro("dup2");
            if (dup2(new_socket, STDERR_FILENO) == -1) perro("dup2");
            while (1) {
                int readc = 0, filled = 0;
                while (1) {
                    readc = recv(new_socket, buf + filled, MAX_MSG_LENGTH - filled - 1, 0);
                    if (!readc) break;
                    filled += readc;
                    if (buf[filled - 1] == '\0') break;

                }
                if (!readc) {
                    D("\t[%d] Client disconnected.\n", pid);
                    break;
                }
                buf[strcspn(buf, "\n")] = 0; // remove newline symbol
                D("\t[%d] Command received: %s\n", pid, buf);
                D("\t[%d] Parsing command.\n", pid);
                api_t *api_struct = parse_cmd_to_api_struct(buf);
                D("\t[%d] Resolving command.\n", pid);
                switch (api_struct->command) {
                    case CREATE: {
                        strcpy(response, api_create(api_struct->paths[0]->actualPath, api_struct->elem_n,
                                                    api_struct->elems, new_stream));
                        break;
                    }
                    case READ: {
                        api_read(api_struct->path_n, api_struct->paths, api_struct->fromRoot, new_stream);
                        break;
                    }
                    case UPDATE: {
                        strcpy(response, api_update(api_struct->paths[0]->actualPath, api_struct->elem_n,
                                                    api_struct->elems, new_stream));
                        break;
                    }
                    case DELETE: {
                        strcpy(response, api_delete(api_struct->paths[0]->actualPath, new_stream));
                        break;
                    }
                    default: {
                        strcpy(response, "{\"error\": \"unexpected_command\"}");
                        break;
                    }
                }
                D("\t[%d] Finished executing command.\n", pid);
                send(new_socket, response, strlen(response), 0);
                bzero(&response, strlen(response));
                send(new_socket, "\n>\t", 3, MSG_NOSIGNAL);
            }
            close(new_socket);
            D("\t[%d] Dying.\n", pid);
            exit(0);
        }
        new_socket = accept(sock, &cli_addr, &cli_len);
    }
    fclose(new_stream);
    close(sock);
    return 0;
}