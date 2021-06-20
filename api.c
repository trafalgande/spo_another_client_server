//
// Created by Rostislav Davydov on 09.06.2021.
//

#include "api.h"
#include "util.h"
#include <bson/bson.h>

#define BUFFER_SIZE 1024

void init_db() {
    bson_t parent;
    bson_t child;
    bson_t child2;
    bson_t child3;

    bson_init(&parent);
    bson_append_utf8(&parent, "parent", -1, "123", -1);
    bson_init(&child);
    bson_append_utf8(&child, "child", -1, "123", -1);

    bson_init(&child2);
    bson_append_utf8(&child2, "childd", -1, "123", -1);
    bson_init(&child3);
    bson_append_utf8(&child3, "childdd", -1, "12311", -1);

    bson_append_document(&child, "roottt", -1, &child2);
    bson_append_document(&child, "roott", -1, &child3);
    bson_append_document(&parent, "root", -1, &child);
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
        //printf("Error writing bson");
        exit(1);
    }
    fwrite(&b, sizeof(b), 1, fptr);
    fclose(fptr);
}


bson_t recurrent_read(bson_iter_t iter, char *path, char *path_eq, char *param, char *value, char *key, char *cond) {
    bson_t inner;
    bson_init(&inner);
    while (bson_iter_next(&iter)) {
        if (BSON_ITER_HOLDS_UTF8 (&iter)) {
            char full_path[2048];
            bzero(full_path, 2048);
            if (strcmp(path, "") != 0) {
                strcpy(full_path, path);
                strcat(full_path, ".");
            }
            strcat(full_path, bson_iter_key(&iter));
            if ((strcmp(cond, "=") == 0) && (strcmp(param, bson_iter_utf8(&iter,NULL)) == 0)) {
                bson_append_utf8(&inner, bson_iter_key(&iter), -1, bson_iter_utf8(&iter,NULL), -1);
                continue;
            }
            if ((strcmp(cond, "!=") == 0) && (strcmp(param, bson_iter_utf8(&iter,NULL)) != 0)) {
                bson_append_utf8(&inner, bson_iter_key(&iter), -1, bson_iter_utf8(&iter,NULL), -1);
                continue;
            }

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
                strcat(full_path, ".");
            }
            strcat(full_path, bson_iter_key(&iter));
            bson_t inner_inner = recurrent_read(child, full_path, path_eq, param, value, key,cond);
            bson_iter_t inner_inner_iter;
            bson_iter_init(&inner_inner_iter, &inner_inner);
            if (bson_iter_next(&inner_inner_iter)){
                bson_append_document(&inner, bson_iter_key(&iter), -1, &inner_inner);
            }
        }
    }
    return inner;
}


char* api_read(char *path, char *param, char *value, char *key, char *cond) {
    if (strcmp(cond,"")==0) return api_read_no_cond(path, param, value, key, cond);

    bson_t b = read_bson();

    int full_path_len = strlen(path) + 1 + strlen(key);
    char full_path[full_path_len];
    bzero(full_path, full_path_len);
    strcat(full_path, path);

    bson_iter_t iter;
    bson_iter_init(&iter, &b);
    bson_iter_t baz;
    if (strcmp(path, "") != 0)
        if (!((bson_iter_init(&iter, &b) && bson_iter_find_descendant(&iter, full_path, &baz))))
            return "{\"error\": \"does not exists\"}";

    bson_iter_init(&iter, &b);
    bson_iter_t new_iter;
    if (strcmp(path, "") != 0){
        bson_iter_find_descendant(&iter, full_path, &baz);
        bson_t *b_res;

        uint32_t object_len;
        const uint8_t *object_buf;

        bson_iter_document(&baz, &object_len, &object_buf);
        b_res = bson_new_from_data(object_buf, object_len);
        bson_iter_init(&new_iter, b_res);
    } else {
        bson_iter_init(&new_iter, &b);
    }

    bson_t result = recurrent_read(new_iter, "", path, param, value, key, cond);
    return bson_as_canonical_extended_json(&result, NULL);
}


char *api_read_no_cond(char *path, char *param, char *value, char *key, char *cond) {
    bson_t b = read_bson();
    int full_path_len = strlen(path) + 1 + strlen(key);

    char full_path[full_path_len];
    strcpy(full_path, path);
    if (strcmp(key, "") != 0) {
        strcat(full_path, ".");
        strcat(full_path, key);
    }

    bson_iter_t iter;
    bson_iter_t baz;
    if ((strcmp(full_path, "") == 0) || (strcmp(full_path, ".") == 0)) return bson_as_canonical_extended_json(&b, NULL);

    if (bson_iter_init(&iter, &b) && bson_iter_find_descendant(&iter, full_path, &baz)) {
        if (BSON_ITER_HOLDS_UTF8 (&baz)) {
            char* buffer;
            const char* hui = bson_iter_utf8(&baz, NULL);
            buffer = bson_utf8_escape_for_json(hui, strlen(hui));
            return buffer;
        } else if (BSON_ITER_HOLDS_DOCUMENT(&baz)) {
            bson_t *b_res;
            uint32_t object_len;
            const uint8_t *object_buf;
            bson_iter_document(&baz, &object_len, &object_buf);
            b_res = bson_new_from_data(object_buf, object_len);
            char *res = bson_as_canonical_extended_json(b_res, NULL);
            return res;
        }
    }
    else return "{\"error\": \"not_found\"}";
}



bson_t recurrent_create(bson_iter_t iter, char *path, char *path_eq, char *param, char *value, char *key) {
    bson_t inner;
    bson_init(&inner);
    if (strcmp(path, path_eq) == 0) {

        if (strcmp(param, "node") == 0) bson_append_utf8(&inner, key, -1, value, -1);
        else if (strcmp(param, "dir") == 0) {
            bson_t b_inserted;
            bson_init(&b_inserted);
            bson_append_document(&inner, key, -1, &b_inserted);

        }
    }
    while (bson_iter_next(&iter)) {
        if (BSON_ITER_HOLDS_UTF8 (&iter)) {
            bson_append_utf8(&inner, bson_iter_key(&iter), -1, bson_iter_utf8(&iter, NULL), -1);
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
                strcat(full_path, ".");
            }

            strcat(full_path, bson_iter_key(&iter));
            bson_t inner_inner = recurrent_create(child, full_path, path_eq, param, value, key);
            bson_append_document(&inner, bson_iter_key(&iter), -1, &inner_inner);

        }
    }
    return inner;

}

char *api_create(char *path, char *param, char *value, char *key, char *cond) {

    bson_t b = read_bson();
    int full_path_len = strlen(path) + 1 + strlen(key);
    char full_path[full_path_len];
    bzero(full_path, full_path_len);
    if (strcmp(path, "")) {
        strcpy(full_path, path);
        strcat(full_path, ".");
    }
    strcat(full_path, key);

    bson_iter_t iter;
    bson_iter_init(&iter, &b);
    bson_iter_t baz;
    if (!(strcmp(path, "") == 0))
        if (!(bson_iter_init(&iter, &b) && bson_iter_find_descendant(&iter, path, &baz)))
            return "{\"error\": \"path_is_invalid\"}";
    if (bson_iter_init(&iter, &b) && bson_iter_find_descendant(&iter, path, &baz) && (BSON_ITER_HOLDS_UTF8 (&baz)))
        return "{\"error\": \"creating node inside node\"}";
    if (bson_iter_init(&iter, &b) && bson_iter_find_descendant(&iter, full_path, &baz))
        return "{\"error\": \"already_exists\"}";

    bson_iter_init(&iter, &b);
    bson_t result = recurrent_create(iter, "", path, param, value, key);
    write_bson(result);
    return bson_as_canonical_extended_json(&result, NULL);

}

bson_t recurrent_delete(bson_iter_t iter, char *path, char *path_eq, char *param, char *value, char *key, char *cond) {
    bson_t inner;
    bson_init(&inner);
    ////printf("sctrcmp: %s - %s\n",path,path_eq);

    //printf("path: %s\n", path);
    //printf("path_eq: %s\n", path_eq);
    //printf("strcmp: %d\n", (strcmp(path, path_eq) == 0));
    while (bson_iter_next(&iter)) {
        if ((strcmp(path, path_eq) == 0)&&(strcmp(cond, "") == 0)) {
            ////printf("skipped");
            continue;

//            if (strcmp(param, "node") == 0) bson_append_utf8(&inner,key, -1, value, -1);
//            else if (strcmp(param, "dir") == 0) {
//                bson_t b_inserted;
//                bson_init(&b_inserted);
//                bson_append_document(&inner,key, -1, &b_inserted);
//
//            }
        }
        if (BSON_ITER_HOLDS_UTF8 (&iter)) {

            //if (strcmp(path, path_eq) == 0) continue;
            char full_path[2048];
            bzero(full_path, 2048);
            if (strcmp(path, "") != 0) {
                strcpy(full_path, path);
                //full_path1 = strcat(new_path,".");
                strcat(full_path, ".");
            }
            strcat(full_path, bson_iter_key(&iter));
            if (strcmp(cond, "") == 0) {
                if (strcmp(full_path, path_eq) == 0) continue;
            } else{
                if ((strcmp(cond, "=") == 0) && (strcmp(param, bson_iter_utf8(&iter,NULL)) == 0) && (index_of(full_path,path_eq)==0)) {
                    continue;
                }
                if ((strcmp(cond, "!=") == 0) && (strcmp(param, bson_iter_utf8(&iter,NULL)) != 0) && (index_of(full_path,path_eq)==0)) {
                    continue;
                }
            }

            ////printf("===%s\n", bson_iter_key(&iter));
            bson_append_utf8(&inner, bson_iter_key(&iter), -1, bson_iter_utf8(&iter, NULL), -1);
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
            if ((strcmp(full_path, path_eq) == 0)&&(strcmp(cond, "") == 0)) continue;
            bson_t inner_inner = recurrent_delete(child, full_path, path_eq, param, value, key,cond);
            ////printf("inner++ %s\n", bson_as_canonical_extended_json(&inner_inner,NULL));
            bson_append_document(&inner, bson_iter_key(&iter), -1, &inner_inner);

        }
    }
    //bson_append_document(&result_bson, bson_iter_key(&iter), -1, &inner);
    ////printf("inner before return %s\n", bson_as_canonical_extended_json(&inner,NULL));

    return inner;

}

char *api_delete(char *path, char *param, char *value, char *key, char *cond) {

    bson_t b = read_bson();
    ////printf("%s\n", bson_as_canonical_extended_json(&b, NULL));

    int full_path_len = strlen(path) + 1 + strlen(key);
    char full_path[full_path_len];
    bzero(full_path, full_path_len);
    strcat(full_path, path);

    ////printf("new_path=%s\n", path);
    ////printf("key=%s\n", key);
    ////printf("full_path=%s\n", full_path);

    //conatraints
    bson_iter_t iter;
    bson_iter_init(&iter, &b);
    bson_iter_t baz;
    if (!(strcmp(path, "") == 0))
        if (!((bson_iter_init(&iter, &b) && bson_iter_find_descendant(&iter, full_path, &baz))))
            return "{\"error\": \"does not exists\"}";
    //conatraints

    bson_iter_init(&iter, &b);
    bson_t result = recurrent_delete(iter, "", path, param, value, key, cond);
    write_bson(result);
    ////printf("%s\n",bson_as_canonical_extended_json(&result,NULL));
    return bson_as_canonical_extended_json(&result, NULL);

}


bson_t recurrent_update(bson_iter_t iter, char *path, char *path_eq, char *param, char *value, char *key, char *cond) {
    bson_t inner;
    bson_init(&inner);
    ////printf("sctrcmp: %s - %s\n",path,path_eq);

    //printf("path: %s\n", path);
    //printf("path_eq: %s\n", path_eq);
    //printf("strcmp: %d\n", (strcmp(path, path_eq) == 0));
    while (bson_iter_next(&iter)) {
        if (BSON_ITER_HOLDS_UTF8 (&iter)) {

            //if (strcmp(path, path_eq) == 0) continue;
//            char full_path[2048];
//            bzero(full_path, 2048);
//            if (strcmp(path,"")!=0){
//                strcpy(full_path, path);
//                //full_path1 = strcat(new_path,".");
//                strcat(full_path, ".");
//            }
//            strcat(full_path, bson_iter_key(&iter));
            if (strcmp(cond, "") == 0) {
                if ((strcmp(path, path_eq) == 0) && (strcmp(key, bson_iter_key(&iter)) == 0)) {
                    //printf("update!");
                    bson_append_utf8(&inner, key, -1, value, -1);

                    continue;
                }
            } else {
                if ((strcmp(cond, "=") == 0) && (strcmp(param, bson_iter_utf8(&iter,NULL)) == 0) && (index_of(path,path_eq)==0)) {
                    //printf("update=");
                    bson_append_utf8(&inner, bson_iter_key(&iter), -1, value, -1);
                    continue;
                }
                if ((strcmp(cond, "!=") == 0) && (strcmp(param, bson_iter_utf8(&iter,NULL) && (index_of(path,path_eq)==0)) != 0)) {
                    //printf("update!=");
                    bson_append_utf8(&inner, bson_iter_key(&iter), -1, value, -1);
                    continue;
                }
            }

            ////printf("===%s\n", bson_iter_key(&iter));
            bson_append_utf8(&inner, bson_iter_key(&iter), -1, bson_iter_utf8(&iter, NULL), -1);
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
            // if (strcmp(full_path, path_eq) == 0) continue;
            bson_t inner_inner = recurrent_update(child, full_path, path_eq, param, value, key, cond);
            ////printf("inner++ %s\n", bson_as_canonical_extended_json(&inner_inner,NULL));
            bson_append_document(&inner, bson_iter_key(&iter), -1, &inner_inner);

        }
    }
    //bson_append_document(&result_bson, bson_iter_key(&iter), -1, &inner);
    ////printf("inner before return %s\n", bson_as_canonical_extended_json(&inner,NULL));

    return inner;

}


char *api_update(char *path, char *param, char *value, char *key, char *cond) {

    bson_t b = read_bson();
    ////printf("%s\n", bson_as_canonical_extended_json(&b, NULL));

    int full_path_len = strlen(path) + 1 + strlen(key);
    char full_path[full_path_len];
    bzero(full_path, full_path_len);
    if (strcmp(path, "")) {
        strcpy(full_path, path);
        //full_path1 = strcat(new_path,".");
        strcat(full_path, ".");
    }
    strcat(full_path, key);

    ////printf("new_path=%s\n", path);
    ////printf("key=%s\n", key);
    ////printf("full_path=%s\n", full_path);

    //conatraints
    bson_iter_t iter;
    bson_iter_init(&iter, &b);
    bson_iter_t baz;
    if (strcmp(cond, "") == 0) {
        if (!(strcmp(path, "") == 0))
            if (!(bson_iter_init(&iter, &b) && bson_iter_find_descendant(&iter, path, &baz)))
                return "{\"error\": \"path_is_invalid\"}";
        if (bson_iter_init(&iter, &b) && bson_iter_find_descendant(&iter, full_path, &baz) &&
            (BSON_ITER_HOLDS_DOCUMENT(&baz)))
            return "{\"error\": \"object is document, can not update\"}";
        if (!((bson_iter_init(&iter, &b) && bson_iter_find_descendant(&iter, full_path, &baz))))
            return "{\"error\": \"does not exist\"}";
    }
    else {
        if (!(strcmp(path, "") == 0))
            if (!(bson_iter_init(&iter, &b) && bson_iter_find_descendant(&iter, path, &baz)))
                return "{\"error\": \"path_is_invalid\"}";

    }
    //conatraints

    bson_iter_init(&iter, &b);
    bson_t result = recurrent_update(iter, "", path, param, value, key, cond);
    write_bson(result);
    ////printf("%s\n",bson_as_canonical_extended_json(&result,NULL));
    return bson_as_canonical_extended_json(&result, NULL);

}




