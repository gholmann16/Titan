#include <gtksourceview/gtksource.h>
#include "global.h"
#include "window.h"
#include "setup.h"
#include "explorer.h"
#include <sys/stat.h>
#include <sys/inotify.h>

int main(int argc, char * argv[]) {

    gtk_init(NULL, NULL);
    gtk_source_init();

    GtkWidget * window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Titan");
    gtk_window_set_default_size(GTK_WINDOW(window), 1080, 840);
    init_app(GTK_WINDOW(window));

    // Main view
    GtkWidget * tabs = gtk_notebook_new();
    gtk_notebook_set_show_border(GTK_NOTEBOOK(tabs), FALSE);
    gtk_notebook_set_scrollable(GTK_NOTEBOOK(tabs), TRUE);

    // Terminal
    GtkWidget * term = vte_terminal_new();
    char * cwd = g_get_current_dir();
    char * args[] = {"/bin/bash", NULL};
    vte_terminal_spawn_async(VTE_TERMINAL(term), VTE_PTY_DEFAULT, cwd, args, NULL, G_SPAWN_DEFAULT, NULL, NULL, NULL, -1, NULL, NULL, NULL);
    free(cwd);

    // Theme
    char * theme = g_strdup("solarized-light");

    // Current working directory
    char tmp[PATH_MAX] = "\0";

    // C automatically allocates non filled entries as NULL/0
    struct Editor editor = {
        .tabs = GTK_NOTEBOOK(tabs),
        .dir = tmp,
        .window = GTK_WINDOW(window),
        .theme = theme,
        .term = VTE_TERMINAL(term),
    };

    // Connect signals
    g_signal_connect(window, "delete-event", G_CALLBACK(delete_event), &editor);
    g_signal_connect(tabs, "switch-page", G_CALLBACK(tab_selected), &editor);

    // Updater thread
    if ((editor.fd = inotify_init()) == -1) {
        puts(_("Filelist updates could not be set up, exiting program"));
        return -1;
    }
    editor.event = malloc(THREAD_BUFFER);
    pthread_create(&editor.tid, NULL, *thread, &editor);

    // Menu setup
    GtkAccelGroup * accel = gtk_accel_group_new();
    gtk_window_add_accel_group(GTK_WINDOW(window), accel);

    GtkWidget * bar = gtk_menu_bar_new();
    init_menu(bar, accel, &editor.current, &editor);

    // Boxes
    GtkWidget * box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    GtkWidget * topbar = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
    GtkWidget * sections = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_paned_set_position(GTK_PANED(sections), 192);
    GtkWidget * pane = gtk_paned_new(GTK_ORIENTATION_VERTICAL);
    gtk_paned_set_position(GTK_PANED(pane), 576);

    // Explorer
    init_explorer(sections, &editor);

    // Add boxes together
    gtk_box_pack_start(GTK_BOX(box), bar, 0, 0, 0);
    gtk_box_pack_start(GTK_BOX(box), topbar, 0, 0, 0);
    gtk_box_pack_start(GTK_BOX(box), sections, 1, 1, 0);
    gtk_paned_add2(GTK_PANED(sections), pane);
    gtk_paned_add1(GTK_PANED(pane), tabs);
    gtk_paned_add2(GTK_PANED(pane), term);

    // Pack up app and run
    gtk_container_add(GTK_CONTAINER(window), box);
    gtk_widget_show_all (window);
    gtk_widget_hide(term);

    // Command line
    if (argc > 1) {
        char * full = realpath(argv[1], NULL);
        struct stat buf;
        if (stat(full, &buf) == -1) {
            warning_popup(GTK_WINDOW(window), _("File does not exist\n"));
            free(full);
        }
        else if (S_ISREG(buf.st_mode)) {
            newpage(&editor, full);
        }
        else if (S_ISDIR(buf.st_mode)) {
            open_explorer(&editor, full);
        }
    }

    gtk_main();
    clear_editor(&editor);
    gtk_source_finalize();

    free(editor.theme);
    pthread_cancel(editor.tid);
    close(editor.fd);
    free(editor.event);

    return 0;
}
