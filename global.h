#define MAX_FILE 256
#include <limits.h>
typedef _Bool bool;

struct Editor {
    GtkWidget * sections;
    GtkNotebook * tabs;
    GtkWidget * expander;
    GtkWindow * window;
    char * dir;
    struct File ** filesystem;
    int filecount;
    struct Document * current;
    struct Document ** pages;
    int len;
    char * theme;
    struct Threader * process;
};

enum Filetype {
    Text,
    Binary,
    Image
};

struct Document {
    char name[MAX_FILE];
    GtkTextBuffer * buffer;
    GtkWindow * window;
    GtkWidget * scrolled;
    GtkWidget * view;
    GtkWidget * modified;
    struct File * data;
    GtkSourceSearchContext * context;
    GtkTextIter last;
    enum Filetype type;
};

struct File {
    char * path;
    GtkWidget * label;
    bool open;
};

#define THREAD_BUFFER 1024
struct Threader {
    struct inotify_event * event;
    pthread_t tid;
};