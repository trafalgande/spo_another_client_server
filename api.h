//
// Created by Rostislav Davydov on 09.06.2021.
//
#include <string.h>
#include "util.h"

#ifndef SPO_NOW_API_H
#define SPO_NOW_API_H

void init_db();

char * api_create(char* path, int elem_length, elem_t *elems[]);
char * api_read(int pathcond_length, path_t *pathconds[], int from_root);
char * api_read_no_cond(char* path, char * param, char * value, char * key, char * cond);
char * api_update(char* path, int elem_length, elem_t *elems[]);
char * api_delete(char* path);
//char * recurrent(char* path, char * param, char * value, char * cond_t);

#endif //SPO_NOW_API_H
