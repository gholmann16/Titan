#include <gtksourceview/gtksource.h>
#include <dirent.h>
#include "global.h"

#define MAX_QUERY 256

void find_all(GtkWidget * entry, struct Editor * editor) {
    DIR * cwd = opendir(editor->cwd);
    struct dirent *dp;

    while ((dp = readdir (cwd)) != NULL) {
        printf("%s\n", dp->d_name);
    }
    return;
}

void replace_tool(GtkWidget * entry) {
    
}

void init_searcher(GtkWidget * searcher, struct Editor * editor) {

    // Search
    GtkWidget * box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    GtkWidget * entry = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(entry), MAX_QUERY); //Might only be unicode max length though
    g_signal_connect(entry, "activate", G_CALLBACK(find_all), editor);

    gtk_box_pack_start(GTK_BOX(searcher), entry, 0, 0, 0);

    // Replace
    GtkWidget * box2 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    GtkWidget * entry2 = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(entry2), MAX_QUERY);
    g_signal_connect(entry2, "activate", G_CALLBACK(replace_tool), editor);
    GtkWidget * replace_button = gtk_button_new_with_label("Replace");

    gtk_box_pack_start(GTK_BOX(searcher), entry2, 0, 0, 0);

    GtkWidget * bubble = gtk_check_button_new_with_label("Match case");
    GtkWidget * bubble2 = gtk_check_button_new_with_label("Replace all");

    gtk_container_add(GTK_CONTAINER(searcher), bubble);
    gtk_container_add(GTK_CONTAINER(searcher), bubble2);
    
}