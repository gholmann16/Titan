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

void filename_to_title(struct Document * document) {
    char title[MAX_FILE + 9];
    char * p = document->name;
    if (strrchr(document->name, '/') != NULL) {
        p = strrchr(document->name, '/') + 1;
    }
    strcpy(title, p);
    strcat(title, " - Triton");
    gtk_window_set_title(document->window, title);
}

void tab_selected(GtkNotebook * notebook, GtkWidget * page, gint num, struct Editor * editor) {
    editor->current = editor->pages[num];
    if (!editor->current->type) {
        filename_to_title(editor->pages[num]);
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

    struct File * datastruct = editor->pages[x]->data;
    if (datastruct)
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

void init_text_view(struct Document * doc, struct Editor * editor) {
    GtkWidget * text = gtk_source_view_new();
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text), GTK_WRAP_NONE);
    gtk_text_view_set_monospace(GTK_TEXT_VIEW(text), TRUE);
    gtk_source_view_set_insert_spaces_instead_of_tabs(GTK_SOURCE_VIEW(text), TRUE);
    gtk_source_view_set_tab_width(GTK_SOURCE_VIEW(text), 4);
    gtk_source_view_set_show_line_numbers(GTK_SOURCE_VIEW(text), TRUE);
    
    doc->scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(doc->scrolled), text);

    GtkTextBuffer * buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text));
    GtkSourceSearchContext * context = gtk_source_search_context_new(GTK_SOURCE_BUFFER(buffer), NULL);

    gtk_source_search_settings_set_wrap_around(gtk_source_search_context_get_settings(context), TRUE);
    gtk_source_buffer_set_implicit_trailing_newline(GTK_SOURCE_BUFFER(buffer), FALSE);

    GtkSourceStyleSchemeManager * scheme_manager = gtk_source_style_scheme_manager_get_default();
    GtkSourceStyleScheme * scheme = gtk_source_style_scheme_manager_get_scheme(scheme_manager, editor->theme);
    gtk_source_buffer_set_style_scheme(GTK_SOURCE_BUFFER(buffer), scheme);

    doc->buffer = buffer;
    doc->view = text;
    doc->context = context;
}

void newpage(struct Editor * editor, char * path) {

    struct Document * doc = malloc(sizeof(struct Document));

    editor->len++;
    editor->pages = reallocarray(editor->pages, editor->len, sizeof(struct Document *));
    editor->pages[editor->len - 1] = doc;

    doc->window = editor->window;
    doc->modified = gtk_image_new_from_icon_name("gtk-dialog-question", 2);

    char * filename = NULL;
    gchar * contents = NULL;
    gsize len = 0;
    gchar * content_type = NULL;
    
    if (path) {
        filename = strrchr(path, '/') + 1;
        strcpy(doc->name, filename);

        struct File * datastruct;

        if (datastruct = get_file_from_path(path, editor)) {
            doc->data = datastruct;
            if (path != datastruct->path)
                free(path);
        }
        else {
            editor->filecount++;
            editor->filesystem = reallocarray(editor->filesystem, editor->filecount, sizeof(struct File *));
            struct File * newdir = malloc(sizeof(struct File));
            editor->filesystem[editor->filecount - 1] = newdir;

            newdir->path = path;
            newdir->label = NULL;
            newdir->open = TRUE;

            doc->data = newdir;
            datastruct = newdir;
        }

        if (!g_file_get_contents(datastruct->path, &contents, &len, NULL)) {
            puts("File system error, exiting.");
            exit(-1);
        }
        content_type = g_content_type_guess(NULL, contents, len, NULL);
        
        if(content_type != NULL && !strncmp(content_type, "image", 5)) 
            doc->type = Image;
        else if (g_utf8_validate(contents, len, NULL) == FALSE)
            doc->type = Binary;
        else 
            doc->type = Text;
    }
    else {
        doc->name[0] = 0;
        doc->data = NULL;
        doc->type = Text;
    }

    switch (doc->type) {

        case Text:
            init_text_view(doc, editor);
            if (!GTK_IS_SCROLLED_WINDOW(doc->scrolled)) {
                exit(-1);
            }

            if (contents) {
                gtk_text_buffer_set_text(doc->buffer, contents, len);
                GtkSourceLanguageManager * manager = gtk_source_language_manager_get_default();
                GtkSourceLanguage * language = gtk_source_language_manager_guess_language (manager, NULL, content_type);
                gtk_source_buffer_set_language(GTK_SOURCE_BUFFER(doc->buffer), language);
            }

            gtk_text_buffer_set_modified(doc->buffer, FALSE);
            filename_to_title(doc);
            g_signal_connect(doc->buffer, "modified-changed", G_CALLBACK(change_indicator), editor);
            break;

        case Binary:
            init_text_view(doc, editor);
            gsize read;
            gsize wrote;

            char * new = g_convert(contents, len, "UTF-8", "ISO-8859-15", &read, &wrote, NULL);
            free(contents);
            contents = malloc(wrote);
            for (gsize x = 0; x < wrote; x++) {
                if(new[x] != 0) {
                    contents[x] = new[x];
                }
                else {
                    contents[x] = ' ';
                }
            }
            free(new);
            gtk_text_buffer_set_text(doc->buffer, contents, wrote);
            gtk_text_buffer_set_modified(doc->buffer, FALSE);
            filename_to_title(doc);
            g_signal_connect(doc->buffer, "modified-changed", G_CALLBACK(change_indicator), editor);
            break;

        case Image:
            doc->scrolled = gtk_image_new_from_file (path);
            break;

    }

    free(contents);
    g_free(content_type);

    GtkWidget * box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    GtkWidget * label = gtk_label_new(doc->name);
    GtkWidget * close = gtk_button_new_from_icon_name("window-close", 2);

    gtk_button_set_relief(GTK_BUTTON(close), GTK_RELIEF_NONE);
    g_signal_connect(close, "clicked", G_CALLBACK(close_tab), editor);

    gtk_box_pack_start(GTK_BOX(box), doc->modified, 0, 0, 0);
    gtk_box_pack_start(GTK_BOX(box), label, 0, 0, 0);
    gtk_box_pack_start(GTK_BOX(box), close, 0, 0, 0);

    gtk_widget_show(close);
    gtk_widget_show(label);
    gtk_widget_show_all(doc->scrolled);

    gtk_notebook_append_page(editor->tabs, doc->scrolled, box);
    gtk_notebook_set_current_page(editor->tabs, gtk_notebook_page_num(editor->tabs, doc->scrolled));

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
        folder->open = TRUE; //Set value to the incorrect one so that selected callback works
    }
    else {
        fill_expander(GTK_WIDGET(self), folder->path, editor);
    }
}

void selected (GtkListBox* box, GtkListBoxRow* row, struct Editor * editor) {

    GtkWidget * widget = gtk_bin_get_child (GTK_BIN(row));
    struct File * file = get_file(widget, editor);
    if (GTK_IS_LABEL(widget)) {
        if (file->open) {
            for (int i = 0; i < editor->len; i++) {
                if (editor->pages[i]->data == file) {
                    gtk_notebook_set_current_page(editor->tabs, i);
                    break;
                }
            }
            return;
        }
        file->open = TRUE;
        newpage(editor, file->path);
    }
    else if GTK_IS_EXPANDER(widget) {
        if (file->open != gtk_expander_get_expanded(GTK_EXPANDER(widget))) { //expanded callback just called
            file->open = !file->open;
        }
        else { //Manual
            gtk_expander_set_expanded(GTK_EXPANDER(widget), !file->open);
            file->open = !file->open;
        }
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

void open_explorer(struct Editor * editor, char * directory) {
    gtk_expander_set_expanded(GTK_EXPANDER(editor->expander), TRUE);
    strcpy(editor->dir, directory);
    fill_expander(editor->expander, directory, editor);

}

void init_explorer(GtkWidget * sections, struct Editor * editor) {
    GtkWidget * scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_min_content_width(GTK_SCROLLED_WINDOW(scrolled), 200);
    gtk_box_pack_start(GTK_BOX(sections), scrolled, 0, 1, 0);

    GtkWidget * expander = gtk_expander_new("Code");
    editor->expander = expander;
    editor->sections = sections;

    gtk_container_add(GTK_CONTAINER(scrolled), expander);
}