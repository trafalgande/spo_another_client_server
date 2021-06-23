//
// Created by Rostislav Davydov on 09.06.2021.
//
#include <string.h>
#include "util.h"

#ifndef SPO_NOW_API_H
#define SPO_NOW_API_H

void init_db();
char * api_create(char* path, int elem_length, elem_t *elems[], FILE *new_stream);
char * api_read(int pathcond_length, path_t *pathconds[], int from_root, FILE *new_stream);
char * api_read_no_cond(char* path, char * param, char * value, char * key, char * cond);
char * api_update(char* path, int elem_length, elem_t *elems[], FILE *new_stream);
char *api_delete(char *path, FILE *new_stream);

#endif //SPO_NOW_API_H
