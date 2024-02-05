#include <gtksourceview/gtksource.h>
#include "global.h"
#include "commands.h"
#include "menu.h"
#include "explorer.h"
#include <limits.h>
#include <sys/stat.h>

int main(int argc, char * argv[]) {

    struct Editor editor;
    editor.current = NULL;
    editor.pages = NULL;

    gtk_init(NULL, NULL);
    gtk_source_init();

    GtkWidget * window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Triton");
    gtk_window_set_default_size(GTK_WINDOW(window), 1080, 840);
    g_signal_connect(window, "delete-event", G_CALLBACK(delete_event), &editor);

    GError *error = NULL;
    GdkPixbuf * icon;
    if (getenv("APPDIR")) {
        char path[PATH_MAX];
        strcpy(path, getenv("APPDIR"));
        strcat(path, "/triton.png");
        icon = gdk_pixbuf_new_from_file(path, &error);
    }
    else 
        icon = gdk_pixbuf_new_from_file("/usr/share/pixmaps/triton.png", &error);

    if (error != NULL) {
        puts(error->message);
        g_clear_error (&error);
    }
    gtk_window_set_icon(GTK_WINDOW(window), icon);

    // Main view

    GtkWidget * tabs = gtk_notebook_new();
    gtk_notebook_set_scrollable(GTK_NOTEBOOK(tabs), TRUE);
    g_signal_connect(tabs, "switch-page", G_CALLBACK(tab_selected), &editor);

    // Theme
    char * default_theme = "solarized-light";
    char * theme = malloc(strlen(default_theme) + 1);
    strcpy(theme, default_theme);

    // Editor initilization
    editor.tabs = GTK_NOTEBOOK(tabs);
    editor.len = 0;
    editor.window = GTK_WINDOW(window);
    editor.filesystem = NULL;
    editor.filecount = 0;
    editor.theme = theme;

    // Menu setup
    GtkAccelGroup * accel = gtk_accel_group_new();
    gtk_window_add_accel_group(GTK_WINDOW(window), accel);

    GtkWidget * bar = gtk_menu_bar_new();
    init_menu(bar, accel, &editor.current, &editor);

    // Boxes
    GtkWidget * box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    GtkWidget * sections = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);

    // Explorer
    init_explorer(sections, &editor);

    // Add boxes together
    gtk_box_pack_start(GTK_BOX(box), bar, 0, 0, 0);
    gtk_box_pack_start(GTK_BOX(box), sections, 1, 1, 0);
    gtk_box_pack_end(GTK_BOX(sections), tabs, 1, 1, 0);

    // Pack up app and run
    gtk_container_add(GTK_CONTAINER(window), box);
    gtk_widget_show_all (window);

    // Command line
    if (argc > 1) {
        char full[4096];
        realpath(argv[1], full);
        struct stat buf;
        if (stat(full, &buf) == -1)
            printf("File %s does not exist\n", argv[1]);
        else if (S_ISREG(buf.st_mode)) {
            char * newpath = malloc(strlen(full) + 1);
            strcpy(newpath, full);
            newpage(&editor, newpath);
        }
        else if (S_ISDIR(buf.st_mode)) {
            open_explorer(&editor, full);
        }
    }

    gtk_main();
    gtk_source_finalize();

    clear_editor(&editor);
    free(editor.theme);
    
    return 0;
}
