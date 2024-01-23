#define MAX_FILE 256

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
    GtkSourceSearchContext * context;
    enum Filetype type;
};

struct File {
    char * path;
    GtkWidget * label;
    char open;
};