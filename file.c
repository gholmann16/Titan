#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "file.h"

// Simple helper functions

struct BetterString * file_text(char * filename) {
    // Get file
    FILE * f = fopen(filename, "r");
    char * contents;
    size_t len;

    fseek(f, 0L, SEEK_END);
    len = ftell(f);
    
    fseek(f, 0L, SEEK_SET);
    contents = (char*)calloc(len, sizeof(char));	
    
    fread(contents, sizeof(char), len, f);
    fclose(f);

    struct BetterString * ret = malloc(sizeof(struct BetterString));
    ret->contents = contents;
    ret->length = len;

    return ret;
}

int is_dir(struct dirent * ent) {
    if (ent->d_type == DT_DIR) {
        return 1;
    }
    else if (ent->d_type == DT_UNKNOWN) {
        struct stat buf;
        stat(ent->d_name, &buf);
        return S_ISDIR(buf.st_mode);
    }
    return 0;
}