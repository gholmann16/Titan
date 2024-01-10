#include <gtksourceview/gtksource.h>
#include <dirent.h>
#include "global.h"
#include "file.h"
#include "explorer.h"

void clickfind(GtkListBox* box, GtkListBoxRow* row, struct Editor * editor) {
    GtkWidget * expander = gtk_widget_get_parent(GTK_WIDGET(box));
    char * filename = (char *)gtk_expander_get_label(GTK_EXPANDER(expander));
    char path[MAX_PATH];
    realpath(filename, path);
    newpage(editor, path);
}

void clear_search(GtkContainer * searcher) {
    GList * children = gtk_container_get_children(searcher);
    while(1) {
        if (GTK_IS_SCROLLED_WINDOW(children->data)) {
            gtk_container_remove(searcher, children->data);
        }
        if(children->next == 0)
            break;
        children = children->next;
    }
}

void find_all(GtkWidget * entry, struct Editor * editor) {

    GtkContainer * searcher = GTK_CONTAINER(gtk_widget_get_parent (entry));
    clear_search(searcher);

    const char * search = gtk_entry_get_text(GTK_ENTRY(entry));

    GtkWidget * results = gtk_scrolled_window_new(NULL, NULL);
    gtk_box_pack_start(GTK_BOX(searcher), results, 1, 1, 0);
    GtkContainer * box = GTK_CONTAINER(gtk_box_new(GTK_ORIENTATION_VERTICAL, 0));
    gtk_container_add(GTK_CONTAINER(results), GTK_WIDGET(box));

    size_t len = strlen(search);

    DIR * cwd = opendir(editor->cwd);
    struct dirent *dp;
    char current[32];

    while ((dp = readdir (cwd)) != NULL) {
        if(is_dir(dp))
            continue;

        struct BetterString * string = file_text(dp->d_name);
        char * place = strstr(string->contents, search);

        if (!place)
            continue;

        GtkWidget * file_expander = gtk_expander_new(dp->d_name);
        
        GtkWidget * instances = gtk_list_box_new();
        gtk_container_add(GTK_CONTAINER(file_expander), instances);

        while (place) {
            char * temp = place;
            while(temp != string->contents) {
                if (*temp == '\n') {
                    temp++;
                    break;
                }
                else if (place - temp == 16) {
                    break;
                }
                temp--;
            }
            char * end = strchr(place, '\n');
            size_t line = end - temp;
            if (line > 31) {
                strncpy(current, temp, 32);
                current[28] = '.';
                current[29] = '.';
                current[30] = '.';
                current[31] = 0;
            }
            else {
                strncpy(current, temp, line);
                current[line] = 0;
            }

            GtkWidget * label = gtk_label_new(current);
            gtk_label_set_xalign(GTK_LABEL(label), 0.0);
            gtk_list_box_insert(GTK_LIST_BOX(instances), label, -1);

            place += len;
            place = strstr(place, search);
        }

        free(string->contents);
        free(string);

        g_signal_connect(instances, "row-selected", G_CALLBACK(clickfind), editor);

        gtk_container_add(box, file_expander);
        gtk_expander_set_expanded(GTK_EXPANDER(file_expander), TRUE);
    }

    gtk_widget_show_all(results);

    return;
}

void replace_tool(GtkWidget * entry) {
    
}

void init_searcher(GtkWidget * searcher, struct Editor * editor) {

    // Search
    GtkWidget * box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    GtkWidget * entry = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(entry), MAX_FILE); //Might only be unicode max length though
    g_signal_connect(entry, "activate", G_CALLBACK(find_all), editor);

    gtk_box_pack_start(GTK_BOX(searcher), entry, 0, 0, 0);

    // Replace
    GtkWidget * box2 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    GtkWidget * entry2 = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(entry2), MAX_FILE);
    g_signal_connect(entry2, "activate", G_CALLBACK(replace_tool), editor);
    GtkWidget * replace_button = gtk_button_new_with_label("Replace");

    gtk_box_pack_start(GTK_BOX(searcher), entry2, 0, 0, 0);

    GtkWidget * bubble = gtk_check_button_new_with_label("Match case");
    GtkWidget * bubble2 = gtk_check_button_new_with_label("Replace all");

    gtk_container_add(GTK_CONTAINER(searcher), bubble);
    gtk_container_add(GTK_CONTAINER(searcher), bubble2);
    
}