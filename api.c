//
// Created by Rostislav Davydov on 09.06.2021.
//

#include "api.h"
#include "util.h"
//#include "io.h"
#include <bson.h>
#include <json.h>

#define BUFFER_SIZE 1024


void init_db() {
    bson_json_reader_t *reader;
    bson_error_t error;
    bson_t doc = BSON_INITIALIZER;

    FILE *bson;
    bson = fopen("mongo.bson", "w");
    reader = bson_json_reader_new_from_file("test.json", &error);

    if (bson_json_reader_read(reader, &doc, &error) < 0) {
        perror("parsing sucks dick");
    }

    fwrite(bson_get_data(&doc), 1, doc.len, bson);

    bson_json_reader_destroy(reader);
    bson_destroy(&doc);
    fclose(bson);
}

bson_t* read_bson_() {
    bson_reader_t *reader;
    bson_error_t error;
    const bson_t *doc;
    reader = bson_reader_new_from_file("mongo.bson", &error);
    doc = bson_reader_read(reader, NULL);
    return doc;
}

void write_bson(bson_t b) {
    bson_json_reader_t *reader;
    bson_error_t error;
    bson_t doc = b;

    FILE *bson;
    bson = fopen("mongo.bson", "w");
    reader = bson_json_reader_new_from_file("test.json", &error);

    if (bson_json_reader_read(reader, &doc, &error) < 0) {
        perror("parsing sucks dick");
    }

    fwrite(bson_get_data(&doc), 1, doc.len, bson);
}


bson_t recurrent_read_recurse(bson_iter_t iter, int pathcond_count, int pathcond_length, path_t **pathconds) {
    bson_t inner;
    bson_init(&inner);
    ////printf("sctrcmp: %s - %s\n",path,path_eq);

    while (bson_iter_next(&iter)) {
        printf("path: %s\n", bson_iter_key(&iter));
        printf("pathcount: %d\n\n", pathcond_count);
        if (BSON_ITER_HOLDS_UTF8 (&iter)) {
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
            int flag;
            int index_flag = 1;
            if (pathcond_count == pathcond_length) {
                index_flag = 0;
            }

            if (pathcond_length > 0) {
                printf("%s === %s", pathconds[index_flag ? pathcond_count : 0]->actualPath, bson_iter_key(&iter));
                if ((strcmp(pathconds[index_flag ? pathcond_count : 0]->actualPath, bson_iter_key(&iter)) == 0) ||
                    (strcmp(pathconds[index_flag ? pathcond_count : 0]->actualPath, "*") == 0)) {

                    if (pathconds[index_flag ? pathcond_count : 0]->cond_n == 0) {
                        flag = 1;
                        printf("yes=%d", pathconds[index_flag ? pathcond_count : 0]->cond_n);
                    } else if (is_cond(*b_res, pathconds[index_flag ? pathcond_count : 0]->cond_n,
                                       pathconds[index_flag ? pathcond_count : 0]->conditions,
                                       pathconds[index_flag ? pathcond_count : 0]->operators)) {
                        flag = 1;
                        printf("yes");
                    } else flag = 0;
                } else flag = 0;
            } else flag = 0;
            printf("indexflag: %d\n", index_flag);
            printf("flag: %d\n", flag);
            //pathcond_count=0;



            //if (pathcond_count == pathcond_length) pathcond_count == -1;
            bson_t inner_inner;
            if (index_flag == 0)
                if (flag == 0)
                    inner_inner = recurrent_read_recurse(child, 0, pathcond_length, pathconds);
                else
                    inner_inner = recurrent_read_recurse(child, pathcond_count, pathcond_length, pathconds);
            else if (flag == 0)
                inner_inner = recurrent_read_recurse(child, 0, pathcond_length, pathconds);
            else
                inner_inner = recurrent_read_recurse(child, pathcond_count + 1, pathcond_length, pathconds);

            bson_iter_t inner_inner_iter;
            bson_iter_init(&inner_inner_iter, &inner_inner);
            //bson_append_document(&inner, bson_iter_key(&iter), -1, &inner_inner);

            if (bson_iter_next(&inner_inner_iter)) {
                bson_append_document(&inner, bson_iter_key(&iter), -1, &inner_inner);
                //printf("inner=%s", bson_as_canonical_extended_json(&inner_inner,NULL));
            }
//            bson_append_document(&inner, bson_iter_key(&iter), -1, &inner_inner);

        }
    }
    //bson_append_document(&result_bson, bson_iter_key(&iter), -1, &inner);
    ////printf("inner before return %s\n", bson_as_canonical_extended_json(&inner,NULL));



    return inner;

}


bson_t recurrent_read_root(bson_iter_t iter, int pathcond_count, int pathcond_length, path_t **pathconds) {
    bson_t inner;
    bson_init(&inner);
    int end = 0;
    ////printf("sctrcmp: %s - %s\n",path,path_eq);

    //printf("path_eq: %s\n", path_eq);
    //printf("strcmp: %d\n", (strcmp(path, path_eq) == 0));
    while (bson_iter_next(&iter)) {
        printf("path: %s\n", bson_iter_key(&iter));
        printf("pathcount: %d\n\n", pathcond_count);
        //printf("iterated");
        if (BSON_ITER_HOLDS_UTF8 (&iter)) {
            //bson_append_utf8(&inner, bson_iter_key(&iter), -1, bson_iter_utf8(&iter, NULL), -1);

            //bson_append_utf8(&inner, bson_iter_key(&iter), -1, bson_iter_utf8(&iter, NULL), -1);
            // printf("s=%s, s=%s\n",pathconds[pathcond_count-1]->conds[0]->key, bson_iter_key(&iter));
            //printf("s=%s, s=%s\n",pathconds[pathcond_count-1]->pathkey, bson_iter_key(&iter));
            if (pathcond_count == pathcond_length)
                if (pathcond_length > 0)
                    if (pathconds[pathcond_count - 1]->cond_n > 0) {
                        for (int i = 0; i < pathconds[pathcond_count - 1]->cond_n; i++)
                            if (strcmp(pathconds[pathcond_count - 1]->conditions[i]->key, bson_iter_key(&iter)) == 0)
                                bson_append_utf8(&inner, bson_iter_key(&iter), -1, bson_iter_utf8(&iter, NULL), -1);
                    } else bson_append_utf8(&inner, bson_iter_key(&iter), -1, bson_iter_utf8(&iter, NULL), -1);

            ////printf("%s\n",bson_as_canonical_extended_json(&inner,NULL));

        } else if (BSON_ITER_HOLDS_INT(&iter)) {
            if (pathcond_count == pathcond_length)
                if (pathcond_length > 0)
                    if (pathconds[pathcond_count - 1]->cond_n > 0) {

                        for (int i = 0; i < pathconds[pathcond_count - 1]->cond_n; i++)
                            if (strcmp(pathconds[pathcond_count - 1]->conditions[i]->key, bson_iter_key(&iter)) == 0)
                                bson_append_int32(&inner, bson_iter_key(&iter), -1, bson_iter_int32(&iter));
                    } else bson_append_int32(&inner, bson_iter_key(&iter), -1, bson_iter_int32(&iter));


        } else {
            ////printf("%s\n", bson_iter_key(&iter));
            bson_t *b_res;
            uint32_t object_len;
            const uint8_t *object_buf;

            bson_iter_document(&iter, &object_len, &object_buf);
            b_res = bson_new_from_data(object_buf, object_len);
            ////printf("---%s\n", bson_as_canonical_extended_json(b_res,NULL));
            bson_iter_t child;
            bson_iter_init(&child, b_res);
            //if ((strcmp(full_path, path_eq) == 0)&&(strcmp(cond_t, "") == 0)) continue;
            int flag = 1;
            int index_flag = 1;
            if (pathcond_count == pathcond_length) {
                index_flag = 0;
                end = 1;
            }

            if (pathcond_length > 0) {
                printf("%s === %s", pathconds[index_flag ? pathcond_count : 0]->actualPath, bson_iter_key(&iter));
                if ((strcmp(pathconds[index_flag ? pathcond_count : 0]->actualPath, bson_iter_key(&iter)) == 0) ||
                    (strcmp(pathconds[index_flag ? pathcond_count : 0]->actualPath, "*") == 0)) {

                    if (pathconds[index_flag ? pathcond_count : 0]->cond_n == 0) {
                        flag = 1;
                        printf("yes=%d", pathconds[index_flag ? pathcond_count : 0]->cond_n);
                    } else if (is_cond(*b_res, pathconds[index_flag ? pathcond_count : 0]->cond_n,
                                       pathconds[index_flag ? pathcond_count : 0]->conditions,
                                       pathconds[index_flag ? pathcond_count : 0]->operators)) {
                        //pathcond_count += 1;
                        flag = 1;
                        printf("yes");
                    } else flag = 0;
                } else flag = 0;
            } else flag = 0;
            printf("indexflag: %d\n", index_flag);
            printf("flag: %d\n", flag);
            printf("end: %d\n", end);
            //pathcond_count=0;



            //if (pathcond_count == pathcond_length) pathcond_count == -1;
            bson_t inner_inner;
            if (index_flag == 0)
                if (flag == 0)
                    inner_inner = recurrent_read_recurse(child, 0, pathcond_length, pathconds);
                else
                    inner_inner = recurrent_read_recurse(child, pathcond_count, pathcond_length, pathconds);
            else if (flag == 0)
                inner_inner = recurrent_read_recurse(child, 0, pathcond_length, pathconds);
            else
                inner_inner = recurrent_read_recurse(child, pathcond_count + 1, pathcond_length, pathconds);

            bson_iter_t inner_inner_iter;
            bson_iter_init(&inner_inner_iter, &inner_inner);
            //bson_append_document(&inner, bson_iter_key(&iter), -1, &inner_inner);

            if (bson_iter_next(&inner_inner_iter) && end == 0) {
                bson_append_document(&inner, bson_iter_key(&iter), -1, &inner_inner);
                //printf("inner=%s", bson_as_canonical_extended_json(&inner_inner,NULL));
            }
//            bson_append_document(&inner, bson_iter_key(&iter), -1, &inner_inner);

        }
    }
    //bson_append_document(&result_bson, bson_iter_key(&iter), -1, &inner);
    ////printf("inner before return %s\n", bson_as_canonical_extended_json(&inner,NULL));



    return inner;

}

char *api_read(int pathcond_length, path_t *pathconds[], int from_root, FILE *new_stream) {

    bson_t *b = read_bson_();
    char *buff = bson_as_canonical_extended_json(b, NULL);
    printf("true: %s\n", buff);
    bson_free(buff);
    bson_iter_t iter;
    bson_iter_init(&iter, b);

    bson_t result;
    if (from_root != 0) result = recurrent_read_root(iter, 0, pathcond_length, pathconds);
    else result = recurrent_read_recurse(iter, 0, pathcond_length, pathconds);

    char* res = bson_as_relaxed_extended_json(&result, NULL);
    fprintf(stdout, "%s", res);

    return res;
}


char *api_read_no_cond(char *path, char *param, char *value, char *key, char *cond) {
    bson_t* b = read_bson_();
    int full_path_len = strlen(path) + 1 + strlen(key);

    char full_path[full_path_len];
    strcpy(full_path, path);
    if (strcmp(key, "") != 0) {
        strcat(full_path, ".");
        strcat(full_path, key);
    }
    ////printf("fullpath=%s",full_path);



    ////printf("%s\n", bson_as_canonical_extended_json(&b,NULL));
    bson_iter_t iter;
    bson_iter_t baz;
    if ((strcmp(full_path, "") == 0) || (strcmp(full_path, ".") == 0)) return bson_as_canonical_extended_json(b, NULL);

    if (bson_iter_init(&iter, b) && bson_iter_find_descendant(&iter, full_path, &baz)) {
        if (BSON_ITER_HOLDS_UTF8 (&baz)) {
            //bson_iter_value (&baz)

            ////printf("baz = %s", bson_iter_utf8(&baz,NULL));

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
    ////printf("sctrcmp: %s - %s\n",path,path_eq);
    ////printf("path: %s\n",path);
    while (bson_iter_next(&iter)) {
        if (BSON_ITER_HOLDS_UTF8 (&iter)) {
            ////printf("===%s\n", bson_iter_key(&iter));
            bson_append_utf8(&inner, bson_iter_key(&iter), -1, bson_iter_utf8(&iter, NULL), -1);
            ////printf("%s\n",bson_as_canonical_extended_json(&inner,NULL));

        } else if (BSON_ITER_HOLDS_INT (&iter)) {
            ////printf("===%s\n", bson_iter_key(&iter));
            bson_append_int32(&inner, bson_iter_key(&iter), -1, bson_iter_int32(&iter));
            ////printf("%s\n",bson_as_canonical_extended_json(&inner,NULL));

        } else {
            ////printf("%s\n", bson_iter_key(&iter));
            bson_t *b_res;
            uint32_t object_len;
            const uint8_t *object_buf;

            bson_iter_document(&iter, &object_len, &object_buf);
            b_res = bson_new_from_data(object_buf, object_len);
            ////printf("---%s\n", bson_as_canonical_extended_json(b_res,NULL));
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
            ////printf("inner++ %s\n", bson_as_canonical_extended_json(&inner_inner,NULL));
            bson_append_document(&inner, bson_iter_key(&iter), -1, &inner_inner);

        }

        //bson_append_document(&result_bson, bson_iter_key(&iter), -1, &inner);
        ////printf("inner before return %s\n", bson_as_canonical_extended_json(&inner,NULL));
        if (strcmp(path, path_eq) == 0) {
//        bson_iter_t new_iter;
//        bson_iter_init(&new_iter,);

            for (int i = 0; i < elem_length; i++) {
                //printf("iterkey=%s, elemskwy=%s", bson_iter_key(&iter), elems[i]->key);
                const char *hui = bson_iter_key(&iter);
                if (strcmp(bson_iter_key(&iter), elems[i]->key) != 0) {
                    printf("creating->%s\n", elems[i]->key);
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

char *api_create(char *path, int elem_length, elem_t *elems[], FILE *new_stream) {

    bson_t* b = read_bson_();
    ////printf("%s\n", bson_as_canonical_extended_json(&b, NULL));

    ////printf("new_path=%s\n", path);
    ////printf("key=%s\n", key);
    ////printf("full_path=%s\n", full_path);

    //conatraints
    bson_iter_t iter;
    bson_iter_init(&iter, b);
    bson_iter_t baz;
    if (strcmp(path, "") != 0)
        if (!(bson_iter_init(&iter, b) && bson_iter_find_descendant(&iter, path, &baz)))
            return "{\"error\": \"path_is_invalid\"}";
    if (bson_iter_init(&iter, b) && bson_iter_find_descendant(&iter, path, &baz) && (BSON_ITER_HOLDS_UTF8 (&baz)))
        return "{\"error\": \"creating node inside node\"}";
//    if (bson_iter_init(&iter, &b) && bson_iter_find_descendant(&iter, full_path, &baz))
//        return "{\"error\": \"already_exists\"}";
    //conatraints

    bson_iter_init(&iter, b);
    bson_t result = recurrent_create(iter, "", path, elem_length, elems);
    write_bson(result);
    //printf("%s\n", bson_as_canonical_extended_json(&result, NULL));
    return "{\"status\": \"ok\"}";//bson_as_canonical_extended_json(&result, NULL);

}

bson_t recurrent_delete(bson_iter_t iter, char *path, char *path_eq) {
    bson_t inner;
    bson_init(&inner);
    ////printf("sctrcmp: %s - %s\n",path,path_eq);

    //printf("path: %s\n", path);
    //printf("path_eq: %s\n", path_eq);
    //printf("strcmp: %d\n", (strcmp(path, path_eq) == 0));
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


            ////printf("===%s\n", bson_iter_key(&iter));
            bson_append_utf8(&inner, bson_iter_key(&iter), -1, bson_iter_utf8(&iter, NULL), -1);
            ////printf("%s\n",bson_as_canonical_extended_json(&inner,NULL));

        } else if (BSON_ITER_HOLDS_INT (&iter)) {

            bson_append_int32(&inner, bson_iter_key(&iter), -1, bson_iter_int32(&iter));
            ////printf("%s\n",bson_as_canonical_extended_json(&inner,NULL));

        } else {
            ////printf("%s\n", bson_iter_key(&iter));
            bson_t *b_res;
            uint32_t object_len;
            const uint8_t *object_buf;

            bson_iter_document(&iter, &object_len, &object_buf);
            b_res = bson_new_from_data(object_buf, object_len);
            ////printf("---%s\n", bson_as_canonical_extended_json(b_res,NULL));
            bson_iter_t child;
            bson_iter_init(&child, b_res);

            bson_t inner_inner = recurrent_delete(child, full_path, path_eq);
            ////printf("inner++ %s\n", bson_as_canonical_extended_json(&inner_inner,NULL));
            bson_append_document(&inner, bson_iter_key(&iter), -1, &inner_inner);

        }
    }
    //bson_append_document(&result_bson, bson_iter_key(&iter), -1, &inner);
    ////printf("inner before return %s\n", bson_as_canonical_extended_json(&inner,NULL));

    return inner;

}

char *api_delete(char *path, FILE *new_stream) {

    bson_t* b = read_bson_();
    ////printf("%s\n", bson_as_canonical_extended_json(&b, NULL));


    ////printf("new_path=%s\n", path);
    ////printf("key=%s\n", key);
    ////printf("full_path=%s\n", full_path);

    //conatraints
    bson_iter_t iter;
    bson_iter_init(&iter, b);
    bson_iter_t baz;
    if (strcmp(path, "") != 0)
        if (!((bson_iter_init(&iter, b) && bson_iter_find_descendant(&iter, path, &baz))))
            return "{\"error\": \"does not exists\"}";
    //conatraints

    bson_iter_init(&iter, b);
    bson_t result = recurrent_delete(iter, "", path);
    write_bson(result);
    ////printf("%s\n",bson_as_canonical_extended_json(&result,NULL));
    return "{\"status\": \"ok\"}";//bson_as_canonical_extended_json(&result, NULL);

}


bson_t recurrent_update(bson_iter_t iter, char *path, char *path_eq, int elem_length, elem_t *elems[]) {
    bson_t inner;
    bson_init(&inner);
    ////printf("sctrcmp: %s - %s\n",path,path_eq);
    ////printf("path: %s\n",path);
    while (bson_iter_next(&iter)) {
        if (BSON_ITER_HOLDS_UTF8 (&iter)) {
            ////printf("===%s\n", bson_iter_key(&iter));
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


            ////printf("%s\n",bson_as_canonical_extended_json(&inner,NULL));

        } else if (BSON_ITER_HOLDS_INT (&iter)) {
            ////printf("===%s\n", bson_iter_key(&iter));
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


            ////printf("%s\n",bson_as_canonical_extended_json(&inner,NULL));

        } else {
            ////printf("%s\n", bson_iter_key(&iter));
            bson_t *b_res;
            uint32_t object_len;
            const uint8_t *object_buf;

            bson_iter_document(&iter, &object_len, &object_buf);
            b_res = bson_new_from_data(object_buf, object_len);
            ////printf("---%s\n", bson_as_canonical_extended_json(b_res,NULL));
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
            ////printf("inner++ %s\n", bson_as_canonical_extended_json(&inner_inner,NULL));
            bson_append_document(&inner, bson_iter_key(&iter), -1, &inner_inner);

        }

        //bson_append_document(&result_bson, bson_iter_key(&iter), -1, &inner);
        ////printf("inner before return %s\n", bson_as_canonical_extended_json(&inner,NULL));




//        if (strcmp(path, path_eq) == 0) {
////        bson_iter_t new_iter;
////        bson_iter_init(&new_iter,);
//
//            for (int i = 0; i < elem_length; i++) {
//                //printf("iterkey=%s, elemskwy=%s", bson_iter_key(&iter), elems[i]->key);
//                const char *hui = bson_iter_key(&iter);
//                if (strcmp(bson_iter_key(&iter), elems[i]->key) == 0) {
//                    printf("creating->%s\n", elems[i]->key);
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

char *api_update(char *path, int elem_length, elem_t *elems[], FILE *new_stream) {

    bson_t* b = read_bson_();
    ////printf("%s\n", bson_as_canonical_extended_json(&b, NULL));

    printf("new_path=%s\n", path);
    //conatraints
    bson_iter_t iter;
    bson_iter_init(&iter, b);
    bson_iter_t baz;
    if (!(bson_iter_init(&iter, b) && bson_iter_find_descendant(&iter, path, &baz)))
        return "{\"error\": \"path_is_invalid\"}";


//conatraints

    bson_iter_init(&iter, b);
    bson_t result = recurrent_update(iter, "", path, elem_length, elems);
    write_bson(result);
////printf("%s\n",bson_as_canonical_extended_json(&result,NULL));
    return "{\"status\": \"ok\"}";//bson_as_canonical_extended_json(&result, NULL);

}




