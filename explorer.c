#include <gtksourceview/gtksource.h>
#include "global.h"
#include "commands.h"
#include <sys/stat.h>

void change_indicator(GtkTextBuffer * buf, struct Editor * editor) {    
    //Assumes it's the current tab
    if (gtk_text_buffer_get_modified(buf)) {
        const char * current = gtk_window_get_title(editor->window);
        char newtitle [MAX_FILE + 11] = "* ";
        strcat(newtitle, current);
        gtk_window_set_title(editor->window, newtitle);
        gtk_widget_show(editor->current->modified);
    }
    else {
        const char * current = gtk_window_get_title(editor->window);
        gtk_window_set_title(editor->window, current + 2);
        gtk_widget_hide(editor->current->modified);
    }
}

void tab_selected(GtkNotebook * notebook, GtkWidget * page, gint num, struct Editor * editor) {
    editor->current = editor->pages[num];
    if (!editor->current->type) {
        filename_to_title(&editor->pages[num]);
        if (gtk_text_buffer_get_modified(editor->pages[num]->buffer)) {
            const char * current = gtk_window_get_title(editor->window);
            char newtitle [MAX_FILE + 11] = "* ";
            strcat(newtitle, current);
            gtk_window_set_title(editor->window, newtitle);
        }
    }
}

struct File * get_file_from_path(char * path, struct Editor * editor) {
    for (int i = 0; i < editor->filecount; i++) {
        if (strcmp(editor->filesystem[i]->path, path) == 0) {
            return editor->filesystem[i];
        }
    }
    return 0;
}

struct File * get_file(GtkWidget * self, struct Editor * editor) {
    for (int i = 0; i < editor->filecount; i++) {
        if (editor->filesystem[i]->label == self) {
            return editor->filesystem[i];
        }
    }
    exit(-1);
}

void kill_tab_n(struct Editor * editor, int x) {
    gtk_notebook_remove_page(editor->tabs, x);

    struct File * datastruct = get_file_from_path(editor->pages[x]->path, editor);
    datastruct->open = FALSE;
    if (editor->pages[x] == editor->current) {
        editor->current = NULL;
    }

    free(editor->pages[x]);
    
    struct Document ** newpages = malloc(sizeof(struct Document *) * (editor->len - 1));

    int z = 0;
    for (int y = 0; y < editor->len; y++) {
        if (x != y) {
            newpages[z] = editor->pages[y];
            z++;
        }
    }

    free(editor->pages);

    editor->pages = newpages;
    editor->len = editor->len - 1;


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

void newpage(struct Editor * editor, char * path) {

    char * filename = strrchr(path, '/') + 1;

    struct Document * doc = malloc(sizeof(struct Document));    
    strcpy(doc->name, filename);

    editor->len++;
    editor->pages = reallocarray(editor->pages, editor->len, sizeof(struct Document *));
    editor->pages[editor->len - 1] = doc;

    gchar * content_type = g_content_type_guess(filename, NULL, 0, NULL);
    GtkWidget * main;

    doc->window = editor->window;
    doc->modified = gtk_image_new_from_icon_name("gtk-dialog-question", 2);

    struct File * datastruct = get_file_from_path(path, editor);
    doc->path = path;

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
        gtk_text_view_set_monospace(GTK_TEXT_VIEW(text), TRUE);
        gtk_source_view_set_insert_spaces_instead_of_tabs(GTK_SOURCE_VIEW(text), TRUE);
        gtk_source_view_set_tab_width(GTK_SOURCE_VIEW(text), 4);
        gtk_source_view_set_show_line_numbers(GTK_SOURCE_VIEW(text), TRUE);
        
        main = gtk_scrolled_window_new(NULL, NULL);
        gtk_container_add(GTK_CONTAINER(main), text);

        GtkTextBuffer * buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text));

        GtkSourceSearchContext * context = gtk_source_search_context_new(GTK_SOURCE_BUFFER(buffer), NULL);
        GtkSourceLanguageManager * manager = gtk_source_language_manager_get_default();

        GtkSourceLanguage * language = gtk_source_language_manager_guess_language (manager, filename, content_type);

        gtk_source_buffer_set_language(GTK_SOURCE_BUFFER(buffer), language);
        gtk_source_search_settings_set_wrap_around(gtk_source_search_context_get_settings(context), TRUE);
        gtk_source_buffer_set_implicit_trailing_newline(GTK_SOURCE_BUFFER(buffer), FALSE);

        GtkSourceStyleSchemeManager * scheme_manager = gtk_source_style_scheme_manager_get_default();
        GtkSourceStyleScheme * scheme = gtk_source_style_scheme_manager_get_scheme(scheme_manager, "solarized-light");
        gtk_source_buffer_set_style_scheme(GTK_SOURCE_BUFFER(buffer), scheme);

        doc->buffer = buffer;
        doc->view = text;
        doc->context = context;
        doc->scrolled = main;

        open_file(path, &editor->current);
    }

    g_free(content_type);
    gtk_widget_show_all(main);
    
    if (doc->type == Text) {
        g_signal_connect(doc->buffer, "modified-changed", G_CALLBACK(change_indicator), editor);
    }

    GtkWidget * box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    GtkWidget * label = gtk_label_new(filename);
    GtkWidget * close = gtk_button_new_from_icon_name("window-close", 2);

    gtk_button_set_relief(GTK_BUTTON(close), GTK_RELIEF_NONE);
    g_signal_connect(close, "clicked", G_CALLBACK(close_tab), editor);

    gtk_widget_show(label);
    gtk_widget_show(close);

    gtk_box_pack_start(GTK_BOX(box), doc->modified, 0, 0, 0);
    gtk_box_pack_start(GTK_BOX(box), label, 0, 0, 0);
    gtk_box_pack_start(GTK_BOX(box), close, 0, 0, 0);

    gtk_notebook_append_page(editor->tabs, main, box);
    gtk_notebook_set_current_page(editor->tabs, gtk_notebook_page_num(editor->tabs, main));

}

int is_dir(struct dirent * ent) {
    if (ent->d_type == DT_DIR) {
        return 1;
    }
    else if (ent->d_type == DT_UNKNOWN) {
        struct stat buf;
        stat(ent->d_name, &buf);
        return S_ISDIR(buf.st_mode);
    }
    return 0;
}

void fill_expander(GtkWidget * expander, char * directory, struct Editor * editor);

void demolish(GtkExpander * self, struct Editor * editor) {
    GtkWidget * listbox = gtk_bin_get_child(GTK_BIN(self));
    GList * children = gtk_container_get_children(GTK_CONTAINER(listbox));
    
    int len = 0;
    GList * cur = children;
    while(cur) {
        if (GTK_IS_EXPANDER(gtk_bin_get_child(GTK_BIN(cur->data)))) {
            if (gtk_expander_get_expanded(GTK_EXPANDER(gtk_bin_get_child(GTK_BIN(cur->data))))) {
                demolish(GTK_EXPANDER(gtk_bin_get_child(GTK_BIN(cur->data))), editor);
            }
        }
        else {
            if (get_file(gtk_bin_get_child(GTK_BIN(cur->data)), editor)->open) {
                len--;
            }
        }

        len++;
        cur = cur->next;
    }

    int total = editor->filecount - len;
    struct File ** newFilesystem = malloc(total * sizeof(struct File *));
    int count = 0, add = 0;
    for(int i = 0; i < editor->filecount; i++) {
        cur = children;
        add = 1;
        while(cur) {
            if (gtk_bin_get_child(GTK_BIN(cur->data)) == editor->filesystem[i]->label && editor->filesystem[i]->open == FALSE) {
                free(editor->filesystem[i]);
                add = 0;
                break;
            }
            cur = cur->next;
        }
        if(add) {
            newFilesystem[count] = editor->filesystem[i];
            count++;
        }
    }

    free(editor->filesystem);
    editor->filesystem = newFilesystem;
    //printf("filecount %d, total %d, count %d, len %d\n", editor->filecount, total, count, len);
    editor->filecount = total;

    gtk_widget_destroy(listbox);

    struct File * current = get_file(GTK_WIDGET(self), editor);
    current->open = FALSE;
}

void expanded(GtkExpander * self, struct Editor * editor) {
    struct File * folder = get_file(GTK_WIDGET(self), editor);
    if (gtk_expander_get_expanded(self)) {
        demolish(self, editor);
    }
    else {
        folder->open = TRUE;
        fill_expander(GTK_WIDGET(self), folder->path, editor);
    }
}

void selected (GtkListBox* box, GtkListBoxRow* row, struct Editor * editor) {

    GtkWidget * widget = gtk_bin_get_child (GTK_BIN(row));
    if (GTK_IS_LABEL(widget)) {
        struct File * file = get_file(widget, editor);
        if (file->open)
            return;
        file->open = TRUE;
        newpage(editor, file->path);
    }

}

void fill_expander(GtkWidget * expander, char * directory, struct Editor * editor) {
    
    GtkWidget * files = gtk_list_box_new();
    gtk_container_add(GTK_CONTAINER(expander), files);

    DIR *dir;
    struct dirent *ent;

    if ((dir = opendir (directory)) == NULL) {
        perror ("");
        return;
    }

    /* add all the files and directories within directory */
    while ((ent = readdir (dir)) != NULL) {
        if (is_dir(ent)) {
            if (strcmp(ent->d_name, ".") && strcmp(ent->d_name, "..") && strcmp(ent->d_name, ".git")) {

                editor->filecount++;
                editor->filesystem = reallocarray(editor->filesystem, editor->filecount, sizeof(struct File *));
                struct File * newdir = malloc(sizeof(struct File));
                editor->filesystem[editor->filecount - 1] = newdir;

                char * path = malloc(strlen(ent->d_name) + strlen(directory) + 1 + 1);
                strcpy(path, directory);
                strcat(path, "/");
                strcat(path, ent->d_name);

                GtkWidget * folder = gtk_expander_new(ent->d_name);
                gtk_list_box_insert(GTK_LIST_BOX(files), folder, -1);
                gtk_widget_set_visible(folder, TRUE);
                g_signal_connect(folder, "activate", G_CALLBACK(expanded), editor);

                newdir->path = path;
                newdir->label = folder;
                newdir->open = FALSE;
            }
        }
        else {
            GtkWidget * name = gtk_label_new(ent->d_name);
            gtk_label_set_xalign(GTK_LABEL(name), 0.0);
            gtk_list_box_insert(GTK_LIST_BOX(files), name, -1);
            gtk_widget_set_visible(name, TRUE);

            char * path = malloc(strlen(ent->d_name) + strlen(directory) + 1 + 1);
            strcpy(path, directory);
            strcat(path, "/");
            strcat(path, ent->d_name);

            struct File * loc = get_file_from_path(path, editor);
            if (loc) {
                loc->label = name;
                free(path);
                continue;
            }

            editor->filecount++;
            editor->filesystem = reallocarray(editor->filesystem, editor->filecount, sizeof(struct File *));
            struct File * newfile = malloc(sizeof(struct File));
            editor->filesystem[editor->filecount - 1] = newfile;

            newfile->path = path;
            newfile->label = name;
            newfile->open = FALSE;
        }
    }
    closedir (dir);

    g_signal_connect(files, "row-activated", G_CALLBACK(selected), editor);
    gtk_widget_set_visible(files, TRUE);
}

void init_explorer(GtkWidget * sections, struct Editor * editor) {
    GtkWidget * scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_min_content_width(GTK_SCROLLED_WINDOW(scrolled), 200);
    gtk_box_pack_start(GTK_BOX(sections), scrolled, 0, 1, 0);

    GtkWidget * expander = gtk_expander_new("Code");
    editor->expander = expander;
    editor->sections = sections;

    gtk_expander_set_expanded(GTK_EXPANDER(expander), TRUE);
    fill_expander(expander, editor->cwd, editor);
    gtk_container_add(GTK_CONTAINER(scrolled), expander);
}