#include <gtksourceview/gtksource.h>
#include "global.h"
#include "explorer.h"
#include "searcher.h"
#include "gitter.h"

void file_tool(GtkWidget * self, GtkStack * stack) {
    gtk_stack_set_visible_child_name(stack, "explorer");
}

void find_tool(GtkWidget * self, GtkStack * stack) {
    gtk_stack_set_visible_child_name(stack, "searcher");
}

void git_tool(GtkWidget * self, GtkStack * stack) {
    gtk_stack_set_visible_child_name(stack, "gitter");
}

void init_toolbar(GtkBox * sections, struct Editor * editor) {
    
    GtkWidget * tools = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    GtkWidget * file = gtk_image_new_from_icon_name("document-open", 5);
    GtkWidget * find = gtk_image_new_from_icon_name("edit-find", 5);
    GtkWidget * git = gtk_image_new_from_icon_name("network-transmit", 5);
    GtkWidget * settings = gtk_image_new_from_icon_name("emblem-system", 5);

    GtkWidget * file_button = gtk_button_new();
    GtkWidget * find_button = gtk_button_new();
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

    gtk_box_pack_start(sections, tools, 0, 0, 0);

    GtkWidget * explorer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    GtkWidget * searcher = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    GtkWidget * gitter = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);

    GtkStack * stack = GTK_STACK(gtk_stack_new());
    gtk_stack_set_hhomogeneous(stack, TRUE);

    gtk_stack_add_named(stack, searcher, "searcher");
    gtk_stack_add_named(stack, explorer, "explorer");
    gtk_stack_add_named(stack, gitter, "gitter");

    gtk_widget_show(explorer);
    gtk_stack_set_visible_child(stack, explorer);

    g_signal_connect(file_button, "released", G_CALLBACK(file_tool), stack);
    g_signal_connect(find_button, "released", G_CALLBACK(find_tool), stack);
    g_signal_connect(git_button, "released", G_CALLBACK(git_tool), stack);

    gtk_box_pack_start(sections, GTK_WIDGET(stack), 0, 0, 0);

    init_explorer(explorer, editor);
    init_searcher(searcher, editor);
    init_gitter(gitter, editor);

}