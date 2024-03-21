void open_command(GtkWidget * self, struct Document ** document);

void open_folder_command(GtkWidget * self, struct Editor * editor);

void clear_editor(struct Editor * editor);

void new_command(void);

void close_tab_command(GtkWidget * self, struct Editor * editor);

bool prompt_save(struct Editor * editor);

void exit_command(GtkWidget * self, struct Editor * editor);

gboolean delete_event(GtkWidget* self, GdkEvent* event, struct Editor * editor);

void about_command(GtkWidget * self, struct Editor * editor);