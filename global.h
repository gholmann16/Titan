struct Editor {
    GtkWidget * tabs;
    GtkWindow * window;
    char * cwd;
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
    char name[256];
    GtkTextBuffer * buffer;
    GtkWindow * window;
    GtkWidget * scrolled;
    GtkWidget * view;
    GtkSourceSearchContext * context;
    enum Filetype type;
};

struct Panels {
    GtkWidget * explorer;
    GtkWidget * searcher;
    GtkWidget * gitter;
};
