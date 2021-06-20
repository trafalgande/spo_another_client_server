//
// Created by Rostislav Davydov on 09.06.2021.
//
#include <string.h>
#ifndef SPO_NOW_API_H
#define SPO_NOW_API_H

void init_db();

char * api_create(char* path, char * param, char * value, char * key, char *cond);
char * api_read(char* path, char * param, char * value, char * key, char * cond);
char * api_read_no_cond(char* path, char * param, char * value, char * key, char * cond);
char * api_update(char* path, char * param, char * value, char * key, char * cond);
char * api_delete(char* path, char * param, char * value, char * key, char * cond);
char * recurrent(char* path, char * param, char * value, char * cond);

#endif //SPO_NOW_API_H
