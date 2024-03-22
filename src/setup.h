// Initiates i18n + icons
void init_app(GtkWindow * window);

void init_preferences(struct Editor * editor);

// Initiates the menu bar
void init_menu(GtkWidget * bar, GtkAccelGroup * accel, struct Document ** document, struct Editor * editor);
