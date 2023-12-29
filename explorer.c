#include <gtksourceview/gtksource.h>
#include "global.h"
#include "commands.h"

void selected (GtkListBox* box, GtkListBoxRow* row, struct Editor * editor);

void change_indicator(GtkTextBuffer * buf, struct Editor * editor) {    
    //Assumes it's the current tab
    if (gtk_text_buffer_get_modified(buf)) {
        gtk_widget_show(editor->current->modified);
    }
    else {
        gtk_widget_hide(editor->current->modified);
    }
}

void tab_selected(GtkNotebook * notebook, GtkWidget * page, gint num, struct Editor * editor) {
    for(int x = 0; x < editor->len; x++) {
        if((editor->pages[x])->scrolled == page) {
            editor->current = editor->pages[x];
            return;
        }
    }
}

void close_tab(GtkButton * close, struct Editor * editor) {
    GtkWidget * head = gtk_widget_get_parent(GTK_WIDGET(close));
    int x;
    for(x = 0; x < editor->len; x++) {
        if(gtk_notebook_get_tab_label(editor->tabs, (editor->pages[x])->scrolled) == head)
            break;
    }

    kill_tab_n(editor, x);
}

void fill_expander(GtkWidget * expander, char * directory, struct Editor * editor) {
    
    GtkWidget * files = gtk_list_box_new();
    gtk_container_add(GTK_CONTAINER(expander), files);

    DIR *dir;
    struct dirent *ent;
    char path[512];

    if ((dir = opendir (directory)) != NULL) {
        /* print all the files and directories within directory */
        while ((ent = readdir (dir)) != NULL) {
            if (ent->d_type == DT_DIR) {
                if (strcmp(ent->d_name, ".") && strcmp(ent->d_name, "..") && strcmp(ent->d_name, ".git")) {
                    GtkWidget * folder = gtk_expander_new(ent->d_name);
                    gtk_list_box_insert(GTK_LIST_BOX(files), folder, -1);
                    gtk_widget_set_visible(folder, TRUE);

                    strlcpy(path, directory, sizeof(path));
                    strlcat(path, "/", sizeof(path));
                    strlcat(path, ent->d_name, sizeof(path));

                    fill_expander(folder, path, editor);
                }
            }
            else {
                GtkWidget * name = gtk_label_new(ent->d_name);
                strlcpy(path, directory, sizeof(path));
                strlcat(path, "/", sizeof(path));
                strlcat(path, ent->d_name, sizeof(path));
                gtk_widget_set_name(name, path);
                gtk_label_set_xalign(GTK_LABEL(name), 0.0);
                gtk_list_box_insert(GTK_LIST_BOX(files), name, -1);
                gtk_widget_set_visible(name, TRUE);
            }
        }
        closedir (dir);

        g_signal_connect(files, "row-selected", G_CALLBACK(selected), editor);
        gtk_widget_set_visible(files, TRUE);
    }
    
    else {
        perror ("");
    }
}

void newpage(char * filename, struct Editor * editor, char * path) {
    
    struct Document * doc = malloc(sizeof(struct Document));
    
    doc->name[0] = '\0';
    strcpy(doc->name, filename);

    struct Document ** newpages = malloc(sizeof(struct Document *) * (editor->len + 1));

    for (int x = 0; x < editor->len; x++) {
        newpages[x] = editor->pages[x];
    }

    newpages[editor->len] = doc;

    free(editor->pages);

    editor->pages = newpages;
    editor->len = editor->len + 1;

    gchar * content_type = g_content_type_guess(filename, NULL, 0, NULL);
    GtkWidget * main;

    doc->window = editor->window;
    doc->modified = gtk_image_new_from_icon_name("gtk-dialog-question", 2);;

    // Update main struct
    editor->current = doc;

    if(strncmp(content_type, "image", 5) == 0) {
        main = gtk_image_new_from_file (path);
        doc->type = Image;
        doc->scrolled = main;
    }
    else {
        GtkWidget * text = gtk_source_view_new();
        gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text), GTK_WRAP_NONE);
        gtk_source_view_set_insert_spaces_instead_of_tabs(GTK_SOURCE_VIEW(text), TRUE);
        gtk_source_view_set_tab_width(GTK_SOURCE_VIEW(text), 4);
        gtk_source_view_set_show_line_numbers(GTK_SOURCE_VIEW(text), TRUE);
        
        main = gtk_scrolled_window_new(NULL, NULL);
        gtk_container_add(GTK_CONTAINER(main), text);

        GtkTextBuffer * buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text));
        g_signal_connect(buffer, "modified-changed", G_CALLBACK(change_indicator), editor);

        GtkSourceSearchContext * context = gtk_source_search_context_new(GTK_SOURCE_BUFFER(buffer), NULL);
        GtkSourceLanguageManager * manager = gtk_source_language_manager_get_default();

        GtkSourceLanguage * language = gtk_source_language_manager_guess_language (manager, filename, content_type);

        gtk_source_search_context_set_highlight(context, TRUE);
        gtk_source_buffer_set_language(GTK_SOURCE_BUFFER(buffer), language);
        gtk_source_search_settings_set_wrap_around(gtk_source_search_context_get_settings(context), TRUE);
        
        doc->buffer = buffer;
        doc->view = text;
        doc->context = context;
        doc->scrolled = main;

        open_file(path, &editor->current);
    }

    g_free(content_type);
    gtk_widget_show_all(main);

    GtkWidget * box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    GtkWidget * label = gtk_label_new(filename);
    GtkWidget * close = gtk_button_new_from_icon_name("window-close", 2);

    gtk_button_set_relief(GTK_BUTTON(close), GTK_RELIEF_NONE);
    g_signal_connect(close, "clicked", G_CALLBACK(close_tab), editor);

    gtk_widget_show(label);
    gtk_widget_show(close);

    gtk_box_pack_start(GTK_BOX(box), label, 0, 0, 0);
    gtk_box_pack_start(GTK_BOX(box), close, 0, 0, 0);
    gtk_box_pack_start(GTK_BOX(box), doc->modified, 0, 0, 0);

    gtk_notebook_append_page(editor->tabs, main, box);
    gtk_notebook_set_current_page(editor->tabs, gtk_notebook_page_num(editor->tabs, main));

}

void selected (GtkListBox* box, GtkListBoxRow* row, struct Editor * editor) {

    GtkWidget * widget = gtk_bin_get_child (GTK_BIN(row));
    if (GTK_IS_LABEL(widget)) {
        char * file = (char *)gtk_label_get_text(GTK_LABEL(widget));
        char * path = (char *)gtk_widget_get_name(widget);
        newpage(file, editor, path);
    }

}

int init_explorer(GtkWidget * sections, struct Editor * editor, struct Panels * panels) {

    GtkWidget * explorer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    panels->explorer = explorer;

    GtkWidget * expander = gtk_expander_new("Code\t\t");
    gtk_box_pack_start(GTK_BOX(explorer), expander, 1, 1, 0);
    gtk_expander_set_expanded(GTK_EXPANDER(expander), TRUE);
    fill_expander(expander, editor->cwd, editor);

    gtk_box_pack_start(GTK_BOX(sections), explorer, 0, 0, 0);
}