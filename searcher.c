#include <gtksourceview/gtksource.h>
#include "global.h"

GtkWidget * entry;
GtkWidget * entry2;

void find_all(GtkWidget * self, struct Editor * editor) {
    /* Uses popen because if you are trying to inject code into your own system feel free
        I suppose if the app was running with sudo for some reason you could exploit that, 
        but in that case you could just use the app to write to root file anyway */
    char query [256] = "grep -Rn \"";
    strlcat(query, gtk_entry_get_text(GTK_ENTRY(entry)), sizeof(query));
    strlcat(query, "\" .", sizeof(query));
    FILE * f = popen(query, "r");
    
    char output [1024];
    if (fgets(output, sizeof(output), f)) {
        printf("output\n");
    }
    else {
        printf("No matches found\n");    
    }
    return;
}

void init_searcher(GtkWidget * sections, struct Editor * editor, struct Panels * panels) {
    GtkWidget * searcher = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    panels->searcher = searcher;

    // Search
    GtkWidget * box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget * label = gtk_label_new("Search text:");
    entry = gtk_entry_new();
    GtkWidget * search_button = gtk_button_new_with_label("Search");
    g_signal_connect(search_button, "released", G_CALLBACK(find_all), editor);

    gtk_box_pack_start(GTK_BOX(box), label, 0, 0, 0);
    gtk_box_pack_start(GTK_BOX(box), entry, 0, 0, 0);
    gtk_box_pack_start(GTK_BOX(box), search_button, 0, 0, 0);
    gtk_container_add(GTK_CONTAINER(searcher), box);

    // Replace
    GtkWidget * box2 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget * label2 = gtk_label_new("Replace text");
    entry2 = gtk_entry_new();
    GtkWidget * replace_button = gtk_button_new_with_label("Replace");

    gtk_box_pack_start(GTK_BOX(box2), label2, 0, 0, 0);
    gtk_box_pack_start(GTK_BOX(box2), entry2, 0, 0, 0);
    gtk_box_pack_start(GTK_BOX(box2), replace_button, 0, 0, 0);
    gtk_container_add(GTK_CONTAINER(searcher), box2);

    GtkWidget * bubble = gtk_check_button_new_with_label("Match case");
    GtkWidget * bubble2 = gtk_check_button_new_with_label("Replace all");

    gtk_container_add(GTK_CONTAINER(searcher), bubble);
    gtk_container_add(GTK_CONTAINER(searcher), bubble2);
    
    gtk_box_pack_start(GTK_BOX(sections), searcher, 0, 0, 0);
}