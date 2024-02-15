#include <gtksourceview/gtksource.h>
#include "global.h"

struct File * get_file_from_path(char * path, struct Editor * editor) {
    for (int i = 0; i < editor->filecount; i++) {
        if (strcmp(editor->filesystem[i]->path, path) == 0) {
            return editor->filesystem[i];
        }
    }
    return 0;
}

struct File * get_file(GtkWidget * self, struct Editor * editor) {
    for (int i = 0; i < editor->filecount; i++) {
        if (editor->filesystem[i]->label == self) {
            return editor->filesystem[i];
        }
    }
    puts("Could not find a file we know exists");
    exit(-1);
}

struct File * get_dir(int wd, struct Editor * editor) {
    for (int i = 0; i < editor->filecount; i++) {
        if (editor->filesystem[i]->wd == wd) {
            return editor->filesystem[i];
        }
    }
    puts("Inotify data corrupted");
    exit(-1);
}

struct File * new_file_struct(struct Editor * editor) {
    editor->filecount++;
    editor->filesystem = reallocarray(editor->filesystem, editor->filecount, sizeof(struct File *));
    struct File * created = malloc(sizeof(struct File));
    editor->filesystem[editor->filecount - 1] = created;

    created->open = FALSE;
    return created;
}

char * gen_path(char * dir, char * name) {
    char * path = malloc(strlen(dir) + strlen(name) + 1 + 1);
    strcpy(path, dir);
    strcat(path, "/");
    strcat(path, name);
    return path;
}