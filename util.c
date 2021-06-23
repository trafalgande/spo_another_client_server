//
// Created by Rostislav Davydov on 09.06.2021.
//
#include <string.h>
#include <stdlib.h>
#include "util.h"
#include <bson.h>


char *substring(char *destination, const char *source, int beg, int n) {
    // extracts `n` characters from the source string starting from `beg` index
    // and copy them into the destination string
    while (n > 0) {
        *destination = *(source + beg);

        destination++;
        source++;
        n--;
    }

    // null terminate destination string
    *destination = '\0';

    // return the destination string
    return destination;
}

const int index_of(char *str, char *substr) {
    //char *str = "sdfadabcGGGGGGGGG";
    char *result = strstr(str, substr);
    if (result == NULL) return 0;
    int position = result - str;
    //int substringLength = strlen(str) - position;
    return position;
}

elem_t *new_elem(char *key, char *value_utf8, int value_int, char *type) {
    elem_t *e = malloc(sizeof(elem_t));
    e->key = key;
    e->value_utf8 = value_utf8;
    e->value_int = value_int;
    e->type = type;
    return e;
};

cond_t *new_cond(char *key, char *operator, char *type, char *param_utf8, int param_int) {
    cond_t *c = malloc(sizeof(cond_t));
    c->key = key;
    c->operator = operator;
    c->type = type;
    c->param_utf8 = param_utf8;
    c->param_int = param_int;
    return c;
};

path_t* new_path(cond_t **conditions, int cond_n, int * operators, char * actualPath){
    path_t *pc = malloc(sizeof(cond_t));
    pc->conditions=conditions;
    pc->cond_n=cond_n;
    pc->operators=operators;
    pc->actualPath=actualPath;
    return pc;
};

api_t *new_api(cmd_t command, elem_t** elems, int elem_n, char* nelem_part_path, path_t** paths, int path_n, int fromRoot) {
    api_t* api = malloc(sizeof(api_t));
    api->command = command;
    api->elems = elems;
    api->elem_n = elem_n;
    api->nelem_part_path = nelem_part_path;
    api->paths = paths;
    api->path_n = path_n;
    api->fromRoot = fromRoot;
    return api;
}


int is_cond(bson_t b, int cond_length, cond_t **conds, const int *operators) {
    int sum = 0;

    if (cond_length == 0) return 1;
    int current = 1;
    int current_operator = 1;
    int current_value = 1;

    for (int i = 0; i < cond_length; i++) {
        bson_iter_t iter;
        bson_iter_init(&iter, &b);

        while (bson_iter_next(&iter)) {
            current = 1;
            if (strcmp(bson_iter_key(&iter), conds[i]->key) == 0) {
                if (strcmp(conds[i]->type, "utf8") == 0) {
                    if (!BSON_ITER_HOLDS_UTF8(&iter)) {
                        current = 0;
                        continue;
                    } else if ((strcmp(conds[i]->operator, "=") == 0) &&
                               (strcmp(conds[i]->param_utf8, bson_iter_utf8(&iter, NULL)) == 0)) {
                        current = current;
                        break;
                    } else if ((strcmp(conds[i]->operator, "!=") == 0) &&
                               (strcmp(conds[i]->param_utf8, bson_iter_utf8(&iter, NULL)) != 0)) {
                        current = current;
                        break;
                    } else current = 0;
                    break;
                }

                if (strcmp(conds[i]->type, "int") == 0) {
                    if (!BSON_ITER_HOLDS_INT(&iter)) {
                        current = 0;
                        continue;
                    } else if ((strcmp(conds[i]->operator, "=") == 0) &&
                               (conds[i]->param_int == bson_iter_int32(&iter))) {
                        current = current;
                        break;
                    } else if ((strcmp(conds[i]->operator, "!=") == 0) &&
                               (conds[i]->param_int == bson_iter_int32(&iter))) {
                        current = current;
                        break;
                    } else if ((strcmp(conds[i]->operator, ">") == 0) &&
                               (bson_iter_int32(&iter) > conds[i]->param_int)) {
                        current = current;
                        break;
                    } else if ((strcmp(conds[i]->operator, "<") == 0) &&
                               (bson_iter_int32(&iter) < conds[i]->param_int)) {
                        current = current;
                        break;
                    } else current = 0;
                    break;

                }
                if (strcmp(conds[i]->type, "doc") == 0) {
                    if (!BSON_ITER_HOLDS_DOCUMENT(&iter)) {
                        current = 0;
                        continue;
                    }
                }

            } else {
                current = 0;
            }
        }

        if (i == 0) {
            current_value = current;
            if (i == cond_length - 1) sum += current_value;
        } else {
            if (operators[i - 1] == 0) {
                sum += current_value;
                current_value = current;
            } else {
                current_value = current_value && current;
                if (i == cond_length - 1) sum += current_value;
            }
        }
    }
    return sum;
}


