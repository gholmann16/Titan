struct Editor {
    GtkWidget * tabs;
    GtkWindow * window;
    char * cwd;
    struct Document * current;
    struct Document ** pages;
    int len;
};

struct Document {
    char name[256];
    GtkTextBuffer * buffer;
    GtkWindow * window;
    GtkWidget * scrolled;
    GtkWidget * view;
    GtkSourceSearchContext * context;
    int ro;
};
