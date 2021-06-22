//
// Created by Rostislav Davydov on 09.06.2021.
//

#include "api.h"
#include "util.h"
#include <bson/bson.h>

#define BUFFER_SIZE 1024


void init_db() {

    bson_t parent;
    bson_init_from_json(&parent, "{\n"
                                 "  \"a\": {\n"
                                 "    \"b\": {\n"
                                 "      \"c\": \"r\",\n"
                                 "      \"d\": 2\n"
                                 "    },\n"
                                 "    \"a\": {\n"
                                 "      \"b\": {\n"
                                 "        \"c\": \"r\",\n"
                                 "        \"d\": 2\n"
                                 "      },\n"
                                 "      \"r\": 2\n"
                                 "    },\n"
                                 "    \"r\": 2\n"
                                 "  },\n"
                                 "  \"a1\": {\n"
                                 "    \"a6\": {\n"
                                 "      \"b\": {\n"
                                 "        \"c\": \"r\",\n"
                                 "        \"d\": 2\n"
                                 "      },\n"
                                 "      \"r\": 2\n"
                                 "    },\n"
                                 "    \"b\": {\n"
                                 "      \"c\": \"r\",\n"
                                 "      \"d\": 2\n"
                                 "    },\n"
                                 "    \"r\": 2\n"
                                 "  }\n"
                                 "}", -1, NULL);
    FILE *fptr;

    // use appropriate location if you are using MacOS or Linux
    fptr = fopen("mongo.db", "w");

    if (fptr == NULL) {
        exit(1);
    }

    fwrite(&parent, sizeof(parent), 1, fptr);
    fclose(fptr);
}

bson_t read_bson() {
    FILE *fptr;

    // use appropriate location if you are using MacOS or Linux
    fptr = fopen("mongo.db", "r");

    if (fptr == NULL) {
        exit(1);
    }
    bson_t b;

    fread(&b, sizeof(b), 1, fptr);

    fclose(fptr);

    return b;
}

void write_bson(bson_t b) {
    FILE *fptr;

    // use appropriate location if you are using MacOS or Linux
    fptr = fopen("mongo.db", "w");

    if (fptr == NULL) {
        exit(1);
    }
    fwrite(&b, sizeof(b), 1, fptr);
    fclose(fptr);
}


bson_t recurrent_read_recurse(bson_iter_t iter, int pathcond_count, int pathcond_length, path_t **pathconds) {
    bson_t inner;
    bson_init(&inner);

    while (bson_iter_next(&iter)) {
        if (BSON_ITER_HOLDS_UTF8 (&iter)) {
            //bson_append_utf8(&inner, bson_iter_key(&iter), -1, bson_iter_utf8(&iter, NULL), -1);

            //bson_append_utf8(&inner, bson_iter_key(&iter), -1, bson_iter_utf8(&iter, NULL), -1);
            if (pathcond_count == pathcond_length)
                if (pathcond_length > 0)
                    if (pathconds[pathcond_count - 1]->cond_n > 0) {
                        for (int i = 0; i < pathconds[pathcond_count - 1]->cond_n; i++)
                            if (strcmp(pathconds[pathcond_count - 1]->conditions[i]->key, bson_iter_key(&iter)) == 0)
                                bson_append_utf8(&inner, bson_iter_key(&iter), -1, bson_iter_utf8(&iter, NULL), -1);
                    } else bson_append_utf8(&inner, bson_iter_key(&iter), -1, bson_iter_utf8(&iter, NULL), -1);


        } else if (BSON_ITER_HOLDS_INT(&iter)) {
            if (pathcond_count == pathcond_length)
                if (pathcond_length > 0)
                    if (pathconds[pathcond_count - 1]->cond_n > 0) {

                        for (int i = 0; i < pathconds[pathcond_count - 1]->cond_n; i++)
                            if (strcmp(pathconds[pathcond_count - 1]->conditions[i]->key, bson_iter_key(&iter)) == 0)
                                bson_append_int32(&inner, bson_iter_key(&iter), -1, bson_iter_int32(&iter));
                    } else bson_append_int32(&inner, bson_iter_key(&iter), -1, bson_iter_int32(&iter));


        } else {
            bson_t *b_res;
            uint32_t object_len;
            const uint8_t *object_buf;

            bson_iter_document(&iter, &object_len, &object_buf);
            b_res = bson_new_from_data(object_buf, object_len);
            bson_iter_t child;
            bson_iter_init(&child, b_res);
            //if ((strcmp(full_path, path_eq) == 0)&&(strcmp(cond_t, "") == 0)) continue;
            int flag = 1;
            int index_flag=1;
            if (pathcond_count == pathcond_length) {
                index_flag=0;
            }

            if (pathcond_length > 0) {
                if ((strcmp(pathconds[index_flag ? pathcond_count : 0]->actualPath, bson_iter_key(&iter)) == 0) ||
                    (strcmp(pathconds[index_flag ? pathcond_count : 0]->actualPath, "*") == 0)) {

                    if (pathconds[index_flag ? pathcond_count : 0]->cond_n == 0) {
                        flag = 1;
                    }
                    else if (is_cond(*b_res, pathconds[index_flag ? pathcond_count : 0]->cond_n,
                                     pathconds[index_flag ? pathcond_count : 0]->conditions,
                                     pathconds[index_flag ? pathcond_count : 0]->operators)) {
                        //pathcond_count += 1;
                        flag = 1;
                    } else flag = 0;
                }else flag = 0;
            } else flag = 0;
            //pathcond_count=0;



            //if (pathcond_count == pathcond_length) pathcond_count == -1;
            bson_t inner_inner;
            if (index_flag==0)
                if (flag==0)
                    inner_inner = recurrent_read_recurse(child, 0, pathcond_length, pathconds);
                else
                    inner_inner = recurrent_read_recurse(child, pathcond_count, pathcond_length, pathconds);
            else
            if (flag==0)
                inner_inner = recurrent_read_recurse(child, 0, pathcond_length, pathconds);
            else
                inner_inner = recurrent_read_recurse(child, pathcond_count+1, pathcond_length, pathconds);

            bson_iter_t inner_inner_iter;
            bson_iter_init(&inner_inner_iter, &inner_inner);
            //bson_append_document(&inner, bson_iter_key(&iter), -1, &inner_inner);

            if (bson_iter_next(&inner_inner_iter)) {
                bson_append_document(&inner, bson_iter_key(&iter), -1, &inner_inner);
            }
//            bson_append_document(&inner, bson_iter_key(&iter), -1, &inner_inner);

        }
    }
    //bson_append_document(&result_bson, bson_iter_key(&iter), -1, &inner);



    return inner;

}


bson_t recurrent_read_root(bson_iter_t iter, int pathcond_count, int pathcond_length, path_t **pathconds) {
    bson_t inner;
    bson_init(&inner);
    int end=0;

    while (bson_iter_next(&iter)) {
        if (BSON_ITER_HOLDS_UTF8 (&iter)) {
            //bson_append_utf8(&inner, bson_iter_key(&iter), -1, bson_iter_utf8(&iter, NULL), -1);

            //bson_append_utf8(&inner, bson_iter_key(&iter), -1, bson_iter_utf8(&iter, NULL), -1);
            if (pathcond_count == pathcond_length)
                if (pathcond_length > 0)
                    if (pathconds[pathcond_count - 1]->cond_n > 0) {
                        for (int i = 0; i < pathconds[pathcond_count - 1]->cond_n; i++)
                            if (strcmp(pathconds[pathcond_count - 1]->conditions[i]->key, bson_iter_key(&iter)) == 0)
                                bson_append_utf8(&inner, bson_iter_key(&iter), -1, bson_iter_utf8(&iter, NULL), -1);
                    } else bson_append_utf8(&inner, bson_iter_key(&iter), -1, bson_iter_utf8(&iter, NULL), -1);


        } else if (BSON_ITER_HOLDS_INT(&iter)) {
            if (pathcond_count == pathcond_length)
                if (pathcond_length > 0)
                    if (pathconds[pathcond_count - 1]->cond_n > 0) {

                        for (int i = 0; i < pathconds[pathcond_count - 1]->cond_n; i++)
                            if (strcmp(pathconds[pathcond_count - 1]->conditions[i]->key, bson_iter_key(&iter)) == 0)
                                bson_append_int32(&inner, bson_iter_key(&iter), -1, bson_iter_int32(&iter));
                    } else bson_append_int32(&inner, bson_iter_key(&iter), -1, bson_iter_int32(&iter));


        } else {
            bson_t *b_res;
            uint32_t object_len;
            const uint8_t *object_buf;

            bson_iter_document(&iter, &object_len, &object_buf);
            b_res = bson_new_from_data(object_buf, object_len);
            bson_iter_t child;
            bson_iter_init(&child, b_res);
            //if ((strcmp(full_path, path_eq) == 0)&&(strcmp(cond_t, "") == 0)) continue;
            int flag = 1;
            int index_flag=1;
            if (pathcond_count == pathcond_length) {
                index_flag=0;
                end=1;
            }

            if (pathcond_length > 0) {
                if ((strcmp(pathconds[index_flag ? pathcond_count : 0]->actualPath, bson_iter_key(&iter)) == 0) ||
                    (strcmp(pathconds[index_flag ? pathcond_count : 0]->actualPath, "*") == 0)) {

                    if (pathconds[index_flag ? pathcond_count : 0]->cond_n == 0) {
                        flag = 1;
                    }
                    else if (is_cond(*b_res, pathconds[index_flag ? pathcond_count : 0]->cond_n,
                                     pathconds[index_flag ? pathcond_count : 0]->conditions,
                                     pathconds[index_flag ? pathcond_count : 0]->operators)) {
                        //pathcond_count += 1;
                        flag = 1;
                    } else flag = 0;
                }else flag = 0;
            } else flag = 0;
            //pathcond_count=0;



            //if (pathcond_count == pathcond_length) pathcond_count == -1;
            bson_t inner_inner;
            if (index_flag==0)
                if (flag==0)
                    inner_inner = recurrent_read_recurse(child, 0, pathcond_length, pathconds);
                else
                    inner_inner = recurrent_read_recurse(child, pathcond_count, pathcond_length, pathconds);
            else
            if (flag==0)
                inner_inner = recurrent_read_recurse(child, 0, pathcond_length, pathconds);
            else
                inner_inner = recurrent_read_recurse(child, pathcond_count+1, pathcond_length, pathconds);

            bson_iter_t inner_inner_iter;
            bson_iter_init(&inner_inner_iter, &inner_inner);
            //bson_append_document(&inner, bson_iter_key(&iter), -1, &inner_inner);

            if (bson_iter_next(&inner_inner_iter) && end==0) {
                bson_append_document(&inner, bson_iter_key(&iter), -1, &inner_inner);
            }
//            bson_append_document(&inner, bson_iter_key(&iter), -1, &inner_inner);

        }
    }
    //bson_append_document(&result_bson, bson_iter_key(&iter), -1, &inner);



    return inner;

}

char *api_read(int pathcond_length, path_t *pathconds[], int from_root) {

    bson_t b = read_bson();
    bson_iter_t iter;
    bson_iter_init(&iter, &b);
    bson_t result;
    if (from_root != 0) result = recurrent_read_root(iter, 0, pathcond_length, pathconds);
    else result = recurrent_read_recurse(iter, 0, pathcond_length, pathconds);

    return bson_as_canonical_extended_json(&result, NULL);

}


char *api_read_no_cond(char *path, char *param, char *value, char *key, char *cond) {
    bson_t b = read_bson();
    int full_path_len = strlen(path) + 1 + strlen(key);

    char full_path[full_path_len];
    strcpy(full_path, path);
    if (!(strcmp(key, "") == 0)) {
        strcat(full_path, ".");
        strcat(full_path, key);
    }



    bson_iter_t iter;
    bson_iter_t baz;
    if ((strcmp(full_path, "") == 0) || (strcmp(full_path, ".") == 0)) return bson_as_canonical_extended_json(&b, NULL);

    if (bson_iter_init(&iter, &b) && bson_iter_find_descendant(&iter, full_path, &baz)) {
        if (BSON_ITER_HOLDS_UTF8 (&baz)) {
            //bson_iter_value (&baz)


            // char buffer[BUFFER_SIZE];
//            bzero(buffer, BUFFER_SIZE);
//            strcpy(buffer, bson_iter_utf8(&baz, NULL));
            char *utf8 = bson_iter_utf8(&baz, NULL);
            char *buffer = bson_utf8_escape_for_json(utf8, strlen(utf8));
            return buffer;

        } else if (BSON_ITER_HOLDS_DOCUMENT(&baz)) {
            //bson_iter_value (&baz)

            bson_t *b_res;

            uint32_t object_len;
            const uint8_t *object_buf;

            bson_iter_document(&baz, &object_len, &object_buf);
            b_res = bson_new_from_data(object_buf, object_len);
            char *res = bson_as_canonical_extended_json(b_res, NULL);
            return res;
            //return res;
        }
    } else return "{\"error\": \"not_found\"}";


}


bson_t recurrent_create(bson_iter_t iter, char *path, char *path_eq, int elem_length, elem_t *elems[]) {
    bson_t inner;
    bson_init(&inner);
    while (bson_iter_next(&iter)) {
        if (BSON_ITER_HOLDS_UTF8 (&iter)) {
            bson_append_utf8(&inner, bson_iter_key(&iter), -1, bson_iter_utf8(&iter, NULL), -1);

        } else if (BSON_ITER_HOLDS_INT (&iter)) {
            bson_append_int32(&inner, bson_iter_key(&iter), -1, bson_iter_int32(&iter));

        } else {
            bson_t *b_res;
            uint32_t object_len;
            const uint8_t *object_buf;

            bson_iter_document(&iter, &object_len, &object_buf);
            b_res = bson_new_from_data(object_buf, object_len);
            bson_iter_t child;
            bson_iter_init(&child, b_res);


            char full_path[2048];
            bzero(full_path, 2048);
            if (strcmp(path, "") != 0) {
                strcpy(full_path, path);
                //full_path1 = strcat(new_path,".");
                strcat(full_path, ".");
            }

            strcat(full_path, bson_iter_key(&iter));
            bson_t inner_inner = recurrent_create(child, full_path, path_eq, elem_length, elems);
            bson_append_document(&inner, bson_iter_key(&iter), -1, &inner_inner);

        }

        //bson_append_document(&result_bson, bson_iter_key(&iter), -1, &inner);
        if (strcmp(path, path_eq) == 0) {
//        bson_iter_t new_iter;
//        bson_iter_init(&new_iter,);

            for (int i = 0; i < elem_length; i++) {
                const char *hui = bson_iter_key(&iter);
                if (strcmp(bson_iter_key(&iter), elems[i]->key) != 0) {
                    if (strcmp(elems[i]->type, "utf8") == 0)
                        bson_append_utf8(&inner, elems[i]->key, -1, elems[i]->value_utf8, -1);
                    if (strcmp(elems[i]->type, "int") == 0)
                        bson_append_int32(&inner, elems[i]->key, -1, elems[i]->value_int);
                    if (strcmp(elems[i]->type, "doc") == 0) {
                        bson_t b_inserted;
                        bson_init(&b_inserted);
                        bson_append_document(&inner, elems[i]->key, -1, &b_inserted);
                    }
                }
            }

        }
    }
    return inner;

}

char *api_create(char *path, int elem_length, elem_t *elems[]) {

    bson_t b = read_bson();


    //conatraints
    bson_iter_t iter;
    bson_iter_init(&iter, &b);
    bson_iter_t baz;
    if (!(strcmp(path, "") == 0))
        if (!(bson_iter_init(&iter, &b) && bson_iter_find_descendant(&iter, path, &baz)))
            return "{\"error\": \"path_is_invalid\"}";
    if (bson_iter_init(&iter, &b) && bson_iter_find_descendant(&iter, path, &baz) && (BSON_ITER_HOLDS_UTF8 (&baz)))
        return "{\"error\": \"creating node inside node\"}";
//    if (bson_iter_init(&iter, &b) && bson_iter_find_descendant(&iter, full_path, &baz))
//        return "{\"error\": \"already_exists\"}";
    //conatraints

    bson_iter_init(&iter, &b);
    bson_t result = recurrent_create(iter, "", path, elem_length, elems);
    write_bson(result);
    return "{\"status\": \"ok\"}";//bson_as_canonical_extended_json(&result, NULL);

}

bson_t recurrent_delete(bson_iter_t iter, char *path, char *path_eq) {
    bson_t inner;
    bson_init(&inner);

    while (bson_iter_next(&iter)) {

        char full_path[2048];
        bzero(full_path, 2048);
        if (strcmp(path, "") != 0) {
            strcpy(full_path, path);
            //full_path1 = strcat(new_path,".");
            strcat(full_path, ".");
        }
        strcat(full_path, bson_iter_key(&iter));

        if (strcmp(path, path_eq) == 0) continue;
        if (strcmp(full_path, path_eq) == 0) continue;

        if (BSON_ITER_HOLDS_UTF8 (&iter)) {


            bson_append_utf8(&inner, bson_iter_key(&iter), -1, bson_iter_utf8(&iter, NULL), -1);

        } else if (BSON_ITER_HOLDS_INT (&iter)) {

            bson_append_int32(&inner, bson_iter_key(&iter), -1, bson_iter_int32(&iter));

        } else {
            bson_t *b_res;
            uint32_t object_len;
            const uint8_t *object_buf;

            bson_iter_document(&iter, &object_len, &object_buf);
            b_res = bson_new_from_data(object_buf, object_len);
            bson_iter_t child;
            bson_iter_init(&child, b_res);

            bson_t inner_inner = recurrent_delete(child, full_path, path_eq);
            bson_append_document(&inner, bson_iter_key(&iter), -1, &inner_inner);

        }
    }
    //bson_append_document(&result_bson, bson_iter_key(&iter), -1, &inner);

    return inner;

}

char *api_delete(char *path) {

    bson_t b = read_bson();



    //conatraints
    bson_iter_t iter;
    bson_iter_init(&iter, &b);
    bson_iter_t baz;
    if (!(strcmp(path, "") == 0))
        if (!((bson_iter_init(&iter, &b) && bson_iter_find_descendant(&iter, path, &baz))))
            return "{\"error\": \"does not exists\"}";
    //conatraints

    bson_iter_init(&iter, &b);
    bson_t result = recurrent_delete(iter, "", path);
    write_bson(result);
    return "{\"status\": \"ok\"}";//bson_as_canonical_extended_json(&result, NULL);

}


bson_t recurrent_update(bson_iter_t iter, char *path, char *path_eq, int elem_length, elem_t *elems[]) {
    bson_t inner;
    bson_init(&inner);
    while (bson_iter_next(&iter)) {
        if (BSON_ITER_HOLDS_UTF8 (&iter)) {
            //bson_append_utf8(&inner, bson_iter_key(&iter), -1, bson_iter_utf8(&iter, NULL), -1);

            int flag = 0;
            char *new_value = "";
            for (int i = 0; i < elem_length; i++)
                if (strcmp(bson_iter_key(&iter), elems[i]->key) == 0) {
                    flag = 1;
                    new_value = elems[i]->value_utf8;
                }
            if ((flag == 1) && (strcmp(path, path_eq) == 0))
                bson_append_utf8(&inner, bson_iter_key(&iter), -1, new_value, -1);
            else
                bson_append_utf8(&inner, bson_iter_key(&iter), -1, bson_iter_utf8(&iter, NULL), -1);



        } else if (BSON_ITER_HOLDS_INT (&iter)) {
            int flag = 0;
            int new_value = 0;
            for (int i = 0; i < elem_length; i++)
                if (strcmp(bson_iter_key(&iter), elems[i]->key) == 0) {
                    flag = 1;
                    new_value = elems[i]->value_int;
                }

            if ((flag == 1) && (strcmp(path, path_eq) == 0))
                bson_append_int32(&inner, bson_iter_key(&iter), -1, new_value);
            else
                bson_append_int32(&inner, bson_iter_key(&iter), -1, bson_iter_int32(&iter));



        } else {
            bson_t *b_res;
            uint32_t object_len;
            const uint8_t *object_buf;

            bson_iter_document(&iter, &object_len, &object_buf);
            b_res = bson_new_from_data(object_buf, object_len);
            bson_iter_t child;
            bson_iter_init(&child, b_res);


            char full_path[2048];
            bzero(full_path, 2048);
            if (strcmp(path, "") != 0) {
                strcpy(full_path, path);
                //full_path1 = strcat(new_path,".");
                strcat(full_path, ".");
            }

            strcat(full_path, bson_iter_key(&iter));
            bson_t inner_inner = recurrent_update(child, full_path, path_eq, elem_length, elems);
            bson_append_document(&inner, bson_iter_key(&iter), -1, &inner_inner);

        }

        //bson_append_document(&result_bson, bson_iter_key(&iter), -1, &inner);




//        if (strcmp(path, path_eq) == 0) {
////        bson_iter_t new_iter;
////        bson_iter_init(&new_iter,);
//
//            for (int i = 0; i < elem_length; i++) {
//                const char *hui = bson_iter_key(&iter);
//                if (strcmp(bson_iter_key(&iter), elems[i]->key) == 0) {
//                    if (strcmp(elems[i]->type, "utf8") == 0)
//                        bson_append_utf8(&inner, elems[i]->key, -1, elems[i]->value_utf8, -1);
//                    if (strcmp(elems[i]->type, "int") == 0)
//                        bson_append_int32(&inner, elems[i]->key, -1, elems[i]->value_int);
//
//                }
//            }
//
//        }

    }
    return inner;

}

char *api_update(char *path, int elem_length, elem_t *elems[]) {

    bson_t b = read_bson();

    //conatraints
    bson_iter_t iter;
    bson_iter_init(&iter, &b);
    bson_iter_t baz;
    if (!(bson_iter_init(&iter, &b) && bson_iter_find_descendant(&iter, path, &baz)))
        return "{\"error\": \"path_is_invalid\"}";


//conatraints

    bson_iter_init(&iter, &b);
    bson_t result = recurrent_update(iter, "", path, elem_length, elems);
    write_bson(result);
    return "{\"status\": \"ok\"}";//bson_as_canonical_extended_json(&result, NULL);

}




