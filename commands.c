#include <gtksourceview/gtksource.h>
#include "global.h"

void filename_to_title(struct Document ** document) {
    char title[264] = {0};
    char * p = (*document)->name;
    if (strrchr((*document)->name, '/') != NULL) {
        p = strrchr((*document)->name, '/') + 1;
    }
    strlcat(title, p, sizeof(title));
    strlcat(title, " - Triton", sizeof(title));
    gtk_window_set_title(GTK_WINDOW((*document)->window), title);
}

int open_file(char * filename, struct Document ** document) {
    
    // Get file
    FILE * f = fopen(filename, "r");
    char * contents;
    gsize len;

    fseek(f, 0L, SEEK_END);
    len = ftell(f);
    
    fseek(f, 0L, SEEK_SET);	
    contents = (char*)calloc(len, sizeof(char));	
    
    fread(contents, sizeof(char), len, f);
    fclose(f);

    (*document)->ro = FALSE;

    if (g_utf8_validate(contents, len, NULL) == FALSE) {
        (*document)->ro = TRUE;
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
        contents[wrote] = 0;
        free(new);
        gtk_text_buffer_set_text((*document)->buffer, contents, -1);
    }
    else {
        gtk_text_buffer_set_text((*document)->buffer, contents, len);
    }

    // Update edtior
    gtk_text_buffer_set_modified((*document)->buffer, FALSE);
    strcpy((*document)->name, filename);
    filename_to_title(document);
    free(contents);
    
    return 0;
}

void open_command(GtkWidget * self, struct Document ** document) {
    
    GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;
    GtkWidget *dialog = gtk_file_chooser_dialog_new ("Open File", (*document)->window, action, ("_Cancel"), GTK_RESPONSE_CANCEL, ("_Open"), GTK_RESPONSE_ACCEPT, NULL);

    gint res = gtk_dialog_run (GTK_DIALOG (dialog));
    if (res == GTK_RESPONSE_ACCEPT)
    {
        GtkFileChooser *chooser = GTK_FILE_CHOOSER (dialog);
        char * filename = gtk_file_chooser_get_filename (chooser);
        open_file(filename, document);
        g_free (filename);
    }

    gtk_widget_destroy (dialog);
}

void new_command(void) {
    GError *err = NULL;
    char* argv[] = {"/proc/self/exe", NULL};
    g_spawn_async(NULL, argv, NULL, G_SPAWN_DEFAULT, NULL, NULL, NULL, &err);
    if (err != NULL) {
        fprintf (stderr, "Unable to new window: %s\n", err->message);
        g_error_free (err);
    }
}

void read_only_popup(struct Document ** document) {
    GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;
    GtkWidget * dialog = gtk_message_dialog_new ((*document)->window, flags, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE,"File is read only", NULL);
    gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (dialog);
}

int save(struct Document ** document) {
    
    // Collect all text
    GtkTextIter start;
    GtkTextIter end;

    gtk_text_buffer_get_start_iter((*document)->buffer, &start);
    gtk_text_buffer_get_end_iter((*document)->buffer, &end);

    char * text = gtk_text_buffer_get_text((*document)->buffer, &start, &end, 0);

    FILE * f = fopen((*document)->name, "w");
    fprintf(f, text);
    fclose(f);

    gtk_text_buffer_set_modified((*document)->buffer, FALSE);

    return 0;
}

void save_as_command(GtkWidget * self, struct Document ** document) {
    if(*document == NULL) return;
    
    if ((*document)->ro == TRUE) {
        read_only_popup(document);
        return;
    }

    GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_SAVE;
    GtkWidget *dialog = gtk_file_chooser_dialog_new ("Save File", (*document)->window, action, ("_Cancel"), GTK_RESPONSE_CANCEL, ("_Save"), GTK_RESPONSE_ACCEPT, NULL);

    gint res = gtk_dialog_run (GTK_DIALOG (dialog));
    if (res == GTK_RESPONSE_ACCEPT)
    {
        GtkFileChooser *chooser = GTK_FILE_CHOOSER (dialog);
        char * filename = gtk_file_chooser_get_filename (chooser);
        strcpy((*document)->name, filename);
        save(document);
        g_free (filename);
    }

    gtk_widget_destroy (dialog);
}

void save_command(GtkWidget * self, struct Document ** document) {
    if(*document == NULL) return;

    //If I want to save as file whenever someone presses control s, switch these two if statements around
    if (gtk_text_buffer_get_modified((*document)->buffer) == FALSE)
        return;

    if ((*document)->name[0] == '\0') {
        save_as_command(self, document);
        return;
    }

    if ((*document)->ro == TRUE) {
        read_only_popup(document);
        return;
    }

    save(document);

}

void draw_page (GtkPrintOperation* self, GtkPrintContext* context, gint page_nr, GtkSourcePrintCompositor *compositor) {
    gtk_source_print_compositor_draw_page (compositor, context, page_nr);
}

static gboolean paginate (GtkPrintOperation *operation, GtkPrintContext *context, GtkSourcePrintCompositor *compositor) {
    if (gtk_source_print_compositor_paginate (compositor, context))
    {
        gint n_pages;

        n_pages = gtk_source_print_compositor_get_n_pages (compositor);
        gtk_print_operation_set_n_pages (operation, n_pages);

        return TRUE;
    }

    return FALSE;
}

void print_command(GtkWidget * self, struct Document ** document) {
    if(*document == NULL) return;
    GtkPrintOperation * print = gtk_print_operation_new();
    GtkSourcePrintCompositor * compositor = gtk_source_print_compositor_new(GTK_SOURCE_BUFFER((*document)->buffer));
    
    g_signal_connect (print, "draw_page", G_CALLBACK (draw_page), compositor);
    g_signal_connect (print, "paginate", G_CALLBACK (paginate), compositor);

    GtkPrintOperationResult res = gtk_print_operation_run (print, GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG, GTK_WINDOW ((*document)->window), NULL);

    g_object_unref(print);
    g_object_unref(compositor);
}

void print_preview_command(GtkWidget * self, struct Document ** document) {
    if(*document == NULL) return;
    GtkPrintOperation * print = gtk_print_operation_new();
    GtkSourcePrintCompositor * compositor = gtk_source_print_compositor_new(GTK_SOURCE_BUFFER((*document)->buffer));
    
    g_signal_connect (print, "draw_page", G_CALLBACK (draw_page), compositor);
    g_signal_connect (print, "paginate", G_CALLBACK (paginate), compositor);

    GtkPrintOperationResult res = gtk_print_operation_run (print, GTK_PRINT_OPERATION_ACTION_PREVIEW, GTK_WINDOW ((*document)->window), NULL);

    g_object_unref(print);
    g_object_unref(compositor);
}

void exit_command(GtkWidget * self, struct Editor * editor) {

    if(editor->current == NULL) {
        gtk_main_quit();
        return;
    }

    if (gtk_text_buffer_get_modified(editor->current->buffer) == FALSE) {
        gtk_main_quit();
        return;
    }

    GtkWidget * close = gtk_dialog_new_with_buttons("Triton", editor->current->window, GTK_DIALOG_MODAL, "No", 0, "Cancel", 1, "Yes", 2, NULL);
    GtkWidget * content = gtk_dialog_get_content_area(GTK_DIALOG(close));
    GtkWidget * message = gtk_label_new("Would you like to save?");

    gtk_container_add(GTK_CONTAINER(content), message);
    gtk_widget_show_all(content);

    int res = gtk_dialog_run (GTK_DIALOG (close));
    gtk_widget_destroy (close);

    switch (res) {
        case 0:
            gtk_main_quit();
            break;
        case 1:
            return;
        case 2:
            save_command(GTK_WIDGET(self), &(editor->current));
            break;
    }
}

gboolean delete_event(GtkWidget* self, GdkEvent* event, struct Editor * editor) {
    exit_command(self, editor);
    return TRUE;
}

void kill_tab_n(struct Editor * editor, int x) {
    gtk_notebook_remove_page(GTK_NOTEBOOK(editor->tabs), x);

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

void close_tab_command(GtkWidget * self, struct Editor * editor) {
    if (editor->current == NULL) return;
    int x = gtk_notebook_page_num (GTK_NOTEBOOK(editor->tabs), editor->current->scrolled);
    kill_tab_n(editor, x);
}

void undo_command(GtkWidget * self, struct Document ** document) {
    if(*document == NULL) return;
    if (gtk_source_buffer_can_undo(GTK_SOURCE_BUFFER((*document)->buffer)))
        gtk_source_buffer_undo(GTK_SOURCE_BUFFER((*document)->buffer));
}

void redo_command(GtkWidget * self, struct Document ** document) {
    if(*document == NULL) return;
    if (gtk_source_buffer_can_redo(GTK_SOURCE_BUFFER((*document)->buffer)))
    gtk_source_buffer_redo(GTK_SOURCE_BUFFER((*document)->buffer));
}

void cut_command(GtkWidget * self, struct Document ** document) {
    if(*document == NULL) return;
    gtk_text_buffer_cut_clipboard((*document)->buffer, gtk_clipboard_get_default(gdk_display_get_default()), TRUE);
}

void copy_command(GtkWidget * self, struct Document ** document) {
    if(*document == NULL) return;
    gtk_text_buffer_copy_clipboard((*document)->buffer, gtk_clipboard_get_default(gdk_display_get_default()));
}

void paste_command(GtkWidget * self, struct Document ** document) {
    if(*document == NULL) return;
    gtk_text_buffer_paste_clipboard((*document)->buffer, gtk_clipboard_get_default(gdk_display_get_default()), NULL, TRUE);
}

void delete_command(GtkWidget * self, struct Document ** document) {
    if(*document == NULL) return;
    gtk_text_buffer_delete_selection((*document)->buffer, TRUE, TRUE);
}

void select_all_command(GtkWidget * self, struct Document ** document) {
    if(*document == NULL) return;

    GtkTextIter start;
    GtkTextIter end;

    gtk_text_buffer_get_start_iter((*document)->buffer, &start);
    gtk_text_buffer_get_end_iter((*document)->buffer, &end);

    gtk_text_buffer_select_range((*document)->buffer, &start, &end);
}

void search(struct Document ** document, GtkTextIter start) {
    GtkTextIter match_start;
    GtkTextIter match_end;

    if(gtk_source_search_context_forward((*document)->context, &start, &match_start, &match_end, NULL))
        gtk_text_buffer_select_range((*document)->buffer, &match_start, &match_end);
}

void search_command(GtkWidget * self, struct Document ** document) {
    if(*document == NULL) return;

    GtkSourceSearchSettings * settings = gtk_source_search_context_get_settings((*document)->context);

    GtkWidget * dialog = gtk_dialog_new_with_buttons("Find", (*document)->window, GTK_DIALOG_DESTROY_WITH_PARENT, "Cancel", 0, "Find", 1, NULL);
    GtkWidget * content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

    GtkWidget * box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    
    GtkWidget * entry = gtk_entry_new();
    const gchar * text = gtk_source_search_settings_get_search_text(settings);
    if (text)
        gtk_entry_set_text(GTK_ENTRY(entry), text);

    GtkWidget * label = gtk_label_new("Find text:");
    GtkWidget * bubble = gtk_check_button_new_with_label("Match case");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(bubble), gtk_source_search_settings_get_case_sensitive(settings));


    gtk_box_pack_start(GTK_BOX(box), label, 0, 0, 0);
    gtk_box_pack_end(GTK_BOX(box), entry, 0, 0, 0);

    gtk_container_add(GTK_CONTAINER(content), box);
    gtk_container_add(GTK_CONTAINER(content), bubble);
    gtk_widget_show_all(content);

    GtkTextIter start_of_selection;
    GtkTextIter start;
    gtk_text_buffer_get_selection_bounds((*document)->buffer, &start_of_selection, &start);
    
    int res = gtk_dialog_run (GTK_DIALOG (dialog));

    if (res == 1) {        

        gtk_source_search_settings_set_search_text(settings, gtk_entry_get_text(GTK_ENTRY(entry)));
        gtk_source_search_settings_set_case_sensitive(settings, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(bubble)));

        gtk_widget_destroy (dialog);
        search(document, start);
    }
    else {
        gtk_widget_destroy (dialog);
    }

}

void search_next_command(GtkWidget * self, struct Document ** document) {
    if(*document == NULL) return;
    
    GtkTextIter start_of_selection;
    GtkTextIter start;
    gtk_text_buffer_get_selection_bounds((*document)->buffer, &start_of_selection, &start);

    search(document, start);
}

int lines_in_buffer(GtkTextBuffer * buffer) {
    int count = 0;

    GtkTextIter end;
    gtk_text_buffer_get_end_iter(buffer, &end);

    count = gtk_text_iter_get_line(&end) + 1;
    
    return count;
}

struct Replace {
    GtkSourceSearchSettings * settings;
    GtkEntry * search_entry;
    GtkEntry * replace_entry;
    GtkToggleButton * caps;
    GtkToggleButton * all;
    struct Document ** document;
};

void search_to_replace(GtkWidget * self, struct Replace * replace) {
    gtk_source_search_settings_set_search_text(replace->settings, gtk_entry_get_text(replace->search_entry));
    gtk_source_search_settings_set_case_sensitive(replace->settings, gtk_toggle_button_get_active(replace->caps));

    GtkTextIter start_of_selection;
    GtkTextIter start;
    gtk_text_buffer_get_selection_bounds((*replace->document)->buffer, &start_of_selection, &start);

    search(replace->document, start);
}

void replace(GtkWidget * self, struct Replace * replace) {

    const char * text = gtk_entry_get_text(replace->replace_entry);

    gtk_source_search_settings_set_search_text(replace->settings, gtk_entry_get_text(replace->search_entry));
    gtk_source_search_settings_set_case_sensitive(replace->settings, gtk_toggle_button_get_active(replace->caps));
    
    if (gtk_toggle_button_get_active(replace->all)) {
        gtk_source_search_context_replace_all((*replace->document)->context, text, strlen(text), NULL);
        return;
    }

    GtkTextIter start;
    GtkTextIter end;

    if(!gtk_text_buffer_get_selection_bounds((*replace->document)->buffer, &start, &end)) {
        search(replace->document, start);
        gtk_text_buffer_get_selection_bounds((*replace->document)->buffer, &start, &end);
    }
    gtk_source_search_context_replace((*replace->document)->context, &start, &end, text, strlen(text), NULL);

}

void replace_command(GtkWidget * self, struct Document ** document) {
    if(*document == NULL) return;
    
    GtkSourceSearchSettings * settings = gtk_source_search_context_get_settings((*document)->context);

    GtkWidget * dialog = gtk_dialog_new_with_buttons("Replace", (*document)->window, GTK_DIALOG_DESTROY_WITH_PARENT, "Cancel", 0, NULL);
    GtkWidget * content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

    // Search
    GtkWidget * box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget * label = gtk_label_new("Search text:");
    GtkWidget * entry = gtk_entry_new();
    const gchar * text = gtk_source_search_settings_get_search_text(settings);
    if (text)
        gtk_entry_set_text(GTK_ENTRY(entry), text);
    GtkWidget * search_button = gtk_button_new_with_label("Search");

    gtk_box_pack_start(GTK_BOX(box), label, 0, 0, 0);
    gtk_box_pack_start(GTK_BOX(box), entry, 0, 0, 0);
    gtk_box_pack_start(GTK_BOX(box), search_button, 0, 0, 0);
    gtk_container_add(GTK_CONTAINER(content), box);

    // Replace
    GtkWidget * box2 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget * label2 = gtk_label_new("Replace text");
    GtkWidget * entry2 = gtk_entry_new();
    GtkWidget * replace_button = gtk_button_new_with_label("Replace");

    gtk_box_pack_start(GTK_BOX(box2), label2, 0, 0, 0);
    gtk_box_pack_start(GTK_BOX(box2), entry2, 0, 0, 0);
    gtk_box_pack_start(GTK_BOX(box2), replace_button, 0, 0, 0);
    gtk_container_add(GTK_CONTAINER(content), box2);

    GtkWidget * bubble = gtk_check_button_new_with_label("Match case");
    GtkWidget * bubble2 = gtk_check_button_new_with_label("Replace all");

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(bubble), gtk_source_search_settings_get_case_sensitive(settings));

    gtk_container_add(GTK_CONTAINER(content), bubble);
    gtk_container_add(GTK_CONTAINER(content), bubble2);

    gtk_widget_show_all(content);

    struct Replace rep = {
        settings,
        GTK_ENTRY(entry),
        GTK_ENTRY(entry2),
        GTK_TOGGLE_BUTTON(bubble),
        GTK_TOGGLE_BUTTON(bubble2),
        document
    };

    g_signal_connect(search_button, "clicked", G_CALLBACK(search_to_replace), &rep);
    g_signal_connect(replace_button, "clicked", G_CALLBACK(replace), &rep);
    
    gtk_dialog_run (GTK_DIALOG (dialog));

    gtk_widget_destroy (dialog);
}

void go_to_command(GtkWidget * self, struct Document ** document) {
    if(*document == NULL) return;
    GtkWidget * dialog = gtk_dialog_new_with_buttons("Go To", (*document)->window, GTK_DIALOG_DESTROY_WITH_PARENT, "Go To", 0, "Cancel", 1, NULL);
    GtkWidget * content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

    GtkWidget * line = gtk_label_new("Line number:");
    int l = lines_in_buffer((*document)->buffer);

    GtkWidget * spin = gtk_spin_button_new_with_range(1, l, 1);
    
    gtk_container_add(GTK_CONTAINER(content), line);
    gtk_container_add(GTK_CONTAINER(content), spin);
    gtk_widget_show_all(content);

    int res = gtk_dialog_run(GTK_DIALOG(dialog));
    
    if (res == 0) {
        GtkTextIter jump;
        int value = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin)) - 1;
        gtk_text_buffer_get_iter_at_line((*document)->buffer, &jump, value);
        gtk_text_buffer_place_cursor((*document)->buffer, &jump);
    }

    gtk_widget_destroy(dialog);

}

void font_callback(GtkFontChooser * self, gchar * selected, struct Document ** document) {
    PangoFontDescription * description = pango_font_description_from_string(selected);
    // I don't want to use a deprecated feature, but for some reason gtk decided to deprecate literally every command that works with fonts
    // There is still a way to override them using css, but its messy, and I don't know how consistently it would work with this output
    gtk_widget_override_font((*document)->view, description);
}

void font_command(GtkWidget * self, struct Document ** document) {
    if(*document == NULL) return;

    GtkWidget * dialog = gtk_font_chooser_dialog_new("Fonts", (*document)->window);
    g_signal_connect(dialog, "font-activated", G_CALLBACK(font_callback), document);

    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

void wrap_command(GtkWidget * self, struct Document ** document) {
    if(*document == NULL) return;
    if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(self))) {
        gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW((*document)->view), GTK_WRAP_WORD);
    }
    else {
        gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW((*document)->view), GTK_WRAP_NONE);
    }
}

void spaces_command(GtkWidget * self, struct Document ** document) {
    if(*document == NULL) return;
    if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(self))) {
        gtk_source_view_set_insert_spaces_instead_of_tabs(GTK_SOURCE_VIEW((*document)->view), TRUE);
    }
    else {
        gtk_source_view_set_insert_spaces_instead_of_tabs(GTK_SOURCE_VIEW((*document)->view), FALSE);
    }
}

void about_command(GtkWidget * self, struct Editor * editor) {
    GtkWidget * about_dialog = gtk_about_dialog_new();
    GtkAboutDialog * about = GTK_ABOUT_DIALOG(about_dialog);

    GdkPixbuf * icon = gtk_window_get_icon(editor->window);
    gtk_about_dialog_set_logo(about, icon);

    const char * authors[] = {"Gabriel Holmann <gholmann16@gmail.com>", NULL};
    gtk_about_dialog_set_authors(about, authors);

    gtk_about_dialog_set_license_type(about, GTK_LICENSE_GPL_3_0);

    const char * comments = "Triton is a simple gtk3 code editor intended to be small and efficient. It aims to have a similar functionality to vs code and a smaller memory footprint.";
    gtk_about_dialog_set_comments(about, comments);

    const char * website = "https://github.com/gholmann16/Triton";
    gtk_about_dialog_set_website(about, website);

    gtk_dialog_run(GTK_DIALOG(about_dialog));
    gtk_widget_destroy(about_dialog);
}
