#include "mt.h"

int index_of(char * str, char* substr){
    char *result = strstr(str, substr);
    if (result==NULL) return -1;
    int position = result - str;
    return position;
}

void send_cmd(int sock, int pid) {
	char str[MAX_MSG_LENGTH] = {0};
    char full[MAX_MSG_LENGTH] = {0};
    char buf[MAX_MSG_LENGTH] = {0};

	while (fgets(str, MAX_MSG_LENGTH, stdin) == str) {
        if(strncmp(str, END_STRING, strlen(END_STRING)) == 0) break;

        strcat(full, "echo \"");
        strcat(full, str);
        strcat(full, "\">in.txt | ");
        strcat(full, JAVA_CALL);

        FILE *fp;
        if ((fp = popen(full, "r")) == NULL) {
            printf("Error opening pipe!\n");
        }
        while (fgets(buf, MAX_MSG_LENGTH, fp) != NULL) {
            if (index_of(buf, "{") != -1) {
                if(send(sock, buf, strlen(buf)+1, 0) < 0) perro("send");

                bzero(str, sizeof str);
                bzero(full, sizeof full);
                bzero(buf, sizeof buf);
            } else {
                printf("%s", buf);
                bzero(buf, sizeof buf);
                bzero(str, sizeof str);
                bzero(full, sizeof full);
            }
        }

    }
	kill(pid, SIGKILL);
	printf("Goodbye.\n");
}

void receive(int sock) {
	char buf[MAX_MSG_LENGTH] = {0};
	int filled = recv(sock, buf, MAX_MSG_LENGTH - 1, 0);
    while(filled) {
		buf[filled] = '\0';
		printf("%s", buf);
        bzero(buf, sizeof buf);
		fflush(stdout);
        filled = recv(sock, buf, MAX_MSG_LENGTH - 1, 0);
    }
	printf("Server disconnected.\n");
}

int main() {
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock == -1) perro("socket");

	struct in_addr server_addr;
	if(!inet_aton(HOST, &server_addr)) perro("inet_aton");

	struct sockaddr_in connection;
	connection.sin_family = AF_INET;
	memcpy(&connection.sin_addr, &server_addr, sizeof(server_addr));
	connection.sin_port = htons(PORT);
	if (connect(sock, (const struct sockaddr*) &connection, sizeof(connection)) != 0) perro("connect");
	
	int pid;
	if(pid = fork()) {
        printf(">\t");
        send_cmd(sock, pid);
	}
	else receive(sock);
	return 0;
}