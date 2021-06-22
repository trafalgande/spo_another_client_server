#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <json-c/json.h>
#include <libbson-1.0/bson.h>


#include <signal.h>

#define PORT 5555
#define MAX_MSG_LENGTH 1024
#define END_STRING "chau"
#define COMPLETE_STRING "fin-respuesta"
#define JAVA_CALL "java -classpath /home/losha/IdeaProjects/spo-parser/src/main/java/lib/gson-2.8.7.jar:/home/losha/IdeaProjects/spo-parser/src/main/java/classes CustomParser in.txt"
#define HOST "127.0.0.1"

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL SO_NOSIGPIPE
#endif

#define perro(x) {fprintf(stderr, "%s:%d: %s: %s\n", __FILE__, __LINE__, x, strerror(errno));exit(1);}



