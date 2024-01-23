void init_explorer(GtkWidget * explorer, struct Editor * editor);

void tab_selected(GtkNotebook * notebook, gboolean selected, struct Editor * editor);

void newpage(struct Editor * editor, char * path);

void demolish(GtkExpander * self, struct Editor * editor);

void fill_expander(GtkWidget * expander, char * directory, struct Editor * editor);