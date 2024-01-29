#include <gtksourceview/gtksource.h>
#include "global.h"
#include "commands.h"
#include "menu.h"
#include "explorer.h"
#include <limits.h>

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
        printf(error->message);
        g_clear_error (&error);
    }
    gtk_window_set_icon(GTK_WINDOW(window), icon);

    // Main view

    GtkWidget * tabs = gtk_notebook_new();
    gtk_notebook_set_scrollable(GTK_NOTEBOOK(tabs), TRUE);
    g_signal_connect(tabs, "switch-page", G_CALLBACK(tab_selected), &editor);

    // Current working directory
    char tmp[PATH_MAX];
    getcwd(tmp, PATH_MAX);
    char * cwd = malloc(strlen(tmp) + 1);
    strcpy(cwd, tmp);

    // Editor initilization
    editor.tabs = GTK_NOTEBOOK(tabs);
    editor.cwd = cwd;
    editor.len = 0;
    editor.window = GTK_WINDOW(window);
    editor.filesystem = NULL;
    editor.filecount = 0;

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
    /*
    if (argc > 1) {
        if (getenv("OWD") != NULL) {
            strlcat(document.name, getenv("OWD"), sizeof(document.name));
            strlcat(document.name, "/", sizeof(document.name));
        }
        strlcat(document.name, argv[1], sizeof(document.name));
        if (access(document.name, F_OK) == 0) {
            open_file(document.name, &document);
            filename_to_title(&document);
        }
    }
    */

    gtk_main();
    gtk_source_finalize();

    clear_editor(&editor);
    
    return 0;
}
