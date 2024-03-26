void warning_popup(GtkWindow * window, char * text);

void open_command(GtkWidget * self, struct Document ** document);

void open_folder_command(GtkWidget * self, struct Editor * editor);

void clear_editor(struct Editor * editor);

void open_folder_command(GtkWidget * self, struct Editor * editor);

void new_command(struct Editor * editor);

void new_file_command(GtkWidget * self, struct Editor * editor);

void new_folder_command(GtkWidget * self, struct Editor * editor);

void close_tab_command(GtkWidget * self, struct Editor * editor);

bool prompt_save(struct Editor * editor);

void exit_command(GtkWidget * self, struct Editor * editor);

void terminal_command(GtkWidget * self, struct Editor * editor);

gboolean delete_event(GtkWidget* self, GdkEvent* event, struct Editor * editor);

void theme_command(GtkWidget * self, struct Editor * editor);

void about_command(GtkWidget * self, struct Editor * editor);