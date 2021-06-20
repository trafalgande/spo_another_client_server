#include <stdbool.h>
#include <string.h>
#include <json-c/json.h>
#include "mt.h"


struct Api* api_new(enum COMMAND_TYPE command, char* params, char* nelem, char* path, char* sign, char* right) {
    struct Api* p = malloc(sizeof(struct Api));
    p->command= command;
    p->params = params;
    p->nelem = nelem;
    p->path = path;
    p->sign = sign;
    p->right = right;
    return p;
}

const char *create_json_from_args(const char *command, const char *params,
                                  const char *path, const char *condition) {
    struct json_object *jobj;
    jobj = json_object_new_object();
    json_object_object_add(jobj, "cmd", json_object_new_string(command));
    json_object_object_add(jobj, "params", json_object_new_string(params));
    json_object_object_add(jobj, "path", json_object_new_string(path));
    json_object_object_add(jobj, "cond", json_object_new_string(condition));

    return json_object_to_json_string(jobj);
}

const char *parse_cmd_to_json(char *cmd) {
    int i;
    int count = 0;
    bool flag = false;
    for (i = 0; i < strlen(cmd); i++) {
        if (cmd[i] == ' ')
            count++;
        if (cmd[i] == '[')
            flag = true;
    }


    if (count == 2) {
        char *cmd_token = strtok(cmd, " ");
        char *param_token = strtok(NULL, " ");
        char *path_token = strtok(NULL, " ");
        if (flag) {
            strtok(path_token, "[");
            char *cond_token = strtok(NULL, "[");
            cond_token[strcspn(cond_token, "]")] = 0;
            return create_json_from_args(cmd_token, param_token, path_token, cond_token);
        } else {
            return create_json_from_args(cmd_token, param_token, path_token, "");
        }
    } else {
        return NULL;
    }
}

json_object* parse_str_to_json_obj(const char* in) {
    return json_tokener_parse(in);
}

struct Api* parse_cmd_to_api_struct(const char* buf) {
    json_object *jobj = json_tokener_parse(buf);
    enum COMMAND_TYPE command;
    char *params = "";
    char *path = "";
    char *sign = "";
    char *right = "";
    char *nelem = "";
    json_object_object_foreach(jobj, key, val) {
        if (strcmp(key, "cmd") == 0) {
            if (strcmp(json_object_get_string(val), "create") == 0)
                command = CREATE;
            if (strcmp(json_object_get_string(val), "read") == 0)
                command = READ;
            if (strcmp(json_object_get_string(val), "update") == 0)
                command = UPDATE;
            if (strcmp(json_object_get_string(val), "delete") == 0)
                command = DELETE;
        }
        if (strcmp(key, "param") == 0)
            params = json_object_get_string(val);
        if (strcmp(key, "path") == 0)
            path = json_object_get_string(val);
        if (strcmp(key, "sign") == 0)
            sign = json_object_get_string(val);
        if (strcmp(key, "right") == 0)
            right = json_object_get_string(val);
        if (strcmp(key, "nelem") == 0)
            nelem = json_object_get_string(val);
    }
    struct Api* api_struct = api_new(command, params, nelem, path, sign, right);
    return api_struct;
}

