#include <gtksourceview/gtksource.h>
#include "global.h"

GtkWidget * commit_message;

void commit(GtkWidget * self, struct Editor * editor) {
    printf("%s\n", gtk_entry_get_text(GTK_ENTRY(commit_message)));
}

void init_gitter(GtkWidget * gitter, struct Editor * editor) {
    
    commit_message = gtk_entry_new();
    GtkWidget * commit_button = gtk_button_new_with_label("Commit");
    GtkWidget * expander = gtk_expander_new("Changes\t\t");

    g_signal_connect(commit_button, "released", G_CALLBACK(commit), editor);
    
    gtk_box_pack_start(GTK_BOX(gitter), commit_message, 0, 0, 0);
    gtk_box_pack_start(GTK_BOX(gitter), commit_button, 0, 0, 0);
    gtk_box_pack_start(GTK_BOX(gitter), expander, 0, 0, 0);

}