#include <stdbool.h>
#include <string.h>
#include <json-c/json.h>
#include "mt.h"
#include "util.h"




static int arr_get_obj(struct array_list *arr, int idx, struct json_object **val) {
    if (arr == NULL || idx >= arr->length)
        return -1;

    *val = array_list_get_idx(arr, idx);
    return 0;
}

static inline int obj_get_obj(struct json_object *obj, char *key, struct json_object **val) {
    if (!json_object_object_get_ex(obj, key, val))
        return -1;
    return 0;
}


api_t* parse_cmd_to_api_struct(const char *buf) {
    json_object* jobj = json_tokener_parse(buf);

    cmd_t command;
    int path_n;
    int from_root;

    path_t** paths;
    int cond_n;
    int* operators = {0};
    char* actualPath = NULL;
    cond_t** conditions;
    char* cond_key = NULL;
    char* cond_sign = NULL;
    char* cond_param_type = NULL;
    char* param_utf = NULL;
    int param_int;

    char* nelem_path_part = NULL;
    int nelem_values_n;
    elem_t** nelems;
    char* key = NULL;
    char* value_utf = NULL;
    int value_int;
    char* type = NULL; // dir or node json->params

    json_object_object_foreach(jobj, jkey, jval) {
        if (strcmp(jkey, "cmd") == 0) {
            if (strcmp(json_object_get_string(jval), "create") == 0)
                command = CREATE;
            if (strcmp(json_object_get_string(jval), "read") == 0)
                command = READ;
            if (strcmp(json_object_get_string(jval), "update") == 0)
                command = UPDATE;
            if (strcmp(json_object_get_string(jval), "delete") == 0)
                command = DELETE;
        }

        if (strcmp(jkey, "paths") == 0) {
            array_list *arr = json_object_get_array(jval);
            size_t arr_len = array_list_length(arr);
            paths = malloc(sizeof(path_t*) * arr_len);
            for (int i = 0; i < arr_len; ++i) {
                json_object* tmp;
                arr_get_obj(arr, i, &tmp);
                json_object_object_foreach(tmp, jkey_, jval_) {
                    if (strcmp(jkey_, "actualPath") == 0)
                        actualPath = json_object_get_string(jval_);
                    if (strcmp(jkey_, "cond_n") == 0)
                        cond_n = json_object_get_int(jval_);
                    if (strcmp(jkey_, "operators") == 0) {
                        array_list* arr_ = json_object_get_array(jval_);
                        size_t arr_len_ = array_list_length(arr_);
                        operators = malloc(sizeof(int*) * arr_len_);
                        for (int j = 0; j < arr_len_; ++j) {
                            json_object* tmp_;
                            arr_get_obj(arr_, j, &tmp_);
                            json_object_object_foreach(tmp_, jjkey, jjval) {
                                operators[j] = json_object_get_int(jjval);
                            }
                        }
                    }
                    if (strcmp(jkey_, "conds") == 0) {
                        array_list* arr_ = json_object_get_array(jval_);
                        size_t arr_len_ = array_list_length(arr_);
                        conditions = malloc(sizeof(cond_t*) * arr_len_);
                        for (int j = 0; j < arr_len_; ++j) {
                            json_object* tmp_;
                            arr_get_obj(arr_, j, &tmp_);
                            json_object_object_foreach(tmp_, jjkey, jjval) {
                                if (strcmp(jjkey, "key") == 0)
                                    cond_key = json_object_get_string(jjval);
                                if (strcmp(jjkey, "sign") == 0)
                                    cond_sign = json_object_get_string(jjval);
                                if (strcmp(jjkey, "utf_value") == 0) {
                                    param_utf = json_object_get_string(jjval);
                                    param_int = 0;
                                    cond_param_type = "utf8";
                                }
                                if (strcmp(jjkey, "int_value") == 0) {
                                    param_int = json_object_get_int(jjval);
                                    param_utf = NULL;
                                    cond_param_type = "int";
                                }
                            }
                            conditions[i] = new_cond(cond_key, cond_sign, cond_param_type, param_utf, param_int);
                        }
                    }
                }
                paths[i] = new_path(conditions, cond_n, operators, actualPath);
            }
        }
        if (strcmp(jkey, "nelem") == 0) {
            json_object* tmp = json_object_get(jval);
            json_object_object_foreach(tmp, jjkey, jjval) {
                if (strcmp(jjkey, "elementKey") == 0)
                    nelem_path_part = json_object_get_string(jjval);
                if (strcmp(jjkey, "init_values_n") == 0)
                    nelem_values_n = json_object_get_int(jjval);
                if (strcmp(jjkey, "element_init_values") == 0) {
                    array_list *arr = json_object_get_array(jjval);
                    size_t len = array_list_length(arr);
                    nelems = malloc(sizeof(elem_t) * len);
                    for (int i = 0; i < len; ++i) {
                        json_object* tmp_;
                        arr_get_obj(arr, i, &tmp_);
                        json_object_object_foreach(tmp_, jjkey_, jjval_) {
                            if (strcmp(jjkey_, "init_value_key") == 0)
                                key = json_object_get_string(jjval_);
                            if (strcmp(jjkey_, "utf_value") == 0) {
                                value_utf = json_object_get_string(jjval_);
                                value_int = 0;
                                type = "utf8";
                            }
                            if (strcmp(jjkey_, "int_value") == 0) {
                                value_utf = NULL;
                                value_int = json_object_get_int(jjval_);
                                type = "int";
                            }
                        }
                        nelems[i] = new_elem(key, value_utf, value_int, type);
                    }
                }
            }
        }
        if (strcmp(jkey, "from_root") == 0)
            from_root = json_object_get_int(jval);

        if (strcmp(jkey, "paths_n") == 0)
            path_n = json_object_get_int(jval);
    }

    api_t* api = new_api(command, nelems, nelem_values_n, nelem_path_part, paths, path_n, from_root);

    return api;
}

