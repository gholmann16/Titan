#define MAX_FILE 256
typedef _Bool bool;

struct Editor {
    GtkWidget * sections;
    GtkNotebook * tabs;
    GtkWidget * expander;
    GtkWindow * window;
    char * cwd;
    struct File ** filesystem;
    int filecount;
    struct Document * current;
    struct Document ** pages;
    int len;
    char * theme;
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