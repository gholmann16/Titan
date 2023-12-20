#include <gtksourceview/gtksource.h>
#include "global.h"

void find_tool(GtkWidget * self, struct Panels * panels) {
    //gtk_widget_set_visible(panels->searcher);
    gtk_widget_set_visible(panels->explorer, FALSE);
}

void init_toolbar(GtkWidget * sections, struct Panels * panels) {
    
    GtkWidget * tools = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    GtkWidget * file = gtk_image_new_from_icon_name("document-open", 5);
    GtkWidget * find = gtk_image_new_from_icon_name("edit-find", 5);
    GtkWidget * git = gtk_image_new_from_icon_name("network-transmit", 5);
    GtkWidget * settings = gtk_image_new_from_icon_name("emblem-system", 5);

    GtkWidget * file_button = gtk_button_new();
    GtkWidget * find_button = gtk_button_new();
    g_signal_connect(find_button, "released", G_CALLBACK(find_tool), panels);
    GtkWidget * git_button = gtk_button_new();
    GtkWidget * settings_button = gtk_button_new();

    gtk_button_set_image(GTK_BUTTON(file_button), file);
    gtk_button_set_image(GTK_BUTTON(find_button), find);
    gtk_button_set_image(GTK_BUTTON(git_button), git);
    gtk_button_set_image(GTK_BUTTON(settings_button), settings);

    gtk_box_pack_start(GTK_BOX(tools), file_button, 0, 0, 0);
    gtk_box_pack_start(GTK_BOX(tools), find_button, 0, 0, 0);
    gtk_box_pack_start(GTK_BOX(tools), git_button, 0, 0, 0);
    gtk_box_pack_end(GTK_BOX(tools), settings_button, 0, 0, 0);

    gtk_box_pack_start(GTK_BOX(sections), tools, 0, 0, 0);
}