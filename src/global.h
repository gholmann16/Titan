#define MAX_FILE 256
#define THREAD_BUFFER 1024

#include <limits.h>
#include <libintl.h>
#include <vte/vte.h>
#define _(STRING) gettext(STRING)
typedef _Bool bool;

struct Editor {
    GtkNotebook * tabs;
    GtkWidget * expander;
    GtkWindow * window;
    VteTerminal * term;
    char * dir;
    struct File ** filesystem;
    int filecount;
    struct Document * current;
    struct Document ** pages;
    int len;
    char * theme;
    int fd;
    struct inotify_event * event;
    pthread_t tid;
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
    int wd;
    char * path;
    GtkWidget * label;
    bool open;
};
