#include <gtksourceview/gtksource.h>
#include "global.h"
#include "window.h"

void save(struct Document ** document, char * path) {
    
    // Collect all text
    GtkTextIter start;
    GtkTextIter end;

    gtk_text_buffer_get_start_iter((*document)->buffer, &start);
    gtk_text_buffer_get_end_iter((*document)->buffer, &end);

    char * text = gtk_text_buffer_get_text((*document)->buffer, &start, &end, 0);

    FILE * f = fopen(path, "w");
    fprintf(f, text);
    fclose(f);

    gtk_text_buffer_set_modified((*document)->buffer, FALSE);

}

void save_as_command(GtkWidget * self, struct Document ** document) {
    if(*document == NULL) return;
    
    if ((*document)->type) {
        warning_popup((*document)->window, _("File is read only"));
        return;
    }

    GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_SAVE;
    GtkWidget *dialog = gtk_file_chooser_dialog_new (_("Save File"), (*document)->window, action, _("Cancel"), GTK_RESPONSE_CANCEL, _("Save"), GTK_RESPONSE_ACCEPT, NULL);

    gint res = gtk_dialog_run (GTK_DIALOG (dialog));
    if (res == GTK_RESPONSE_ACCEPT)
    {
        GtkFileChooser *chooser = GTK_FILE_CHOOSER (dialog);
        char * filename = gtk_file_chooser_get_filename (chooser);
        save(document, filename);
        
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
        save_as_command(NULL, document);
        return;
    }

    if ((*document)->type) {
        warning_popup((*document)->window, _("File is read only"));
        return;
    }

    save(document, (*document)->data->path);

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

void search(struct Document ** document) {

    GtkTextIter match_start;
    GtkTextIter match_end;

    if(gtk_source_search_context_forward((*document)->context, &(*document)->last, &match_start, &match_end, NULL)) {
        gtk_text_buffer_select_range((*document)->buffer, &match_start, &match_end);
        (*document)->last = match_end;

        gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW((*document)->view), &match_end, 0.1, FALSE, 0, 0);
    }
}

void search_entry(GtkWidget * entry, struct Document ** document) {
    GtkSourceSearchSettings * settings = gtk_source_search_context_get_settings((*document)->context);
    gtk_source_search_settings_set_search_text(settings, gtk_entry_get_text(GTK_ENTRY(entry)));

    search(document);
}

void match_case(GtkWidget * self, GtkSourceSearchSettings * settings) {
    gtk_source_search_settings_set_case_sensitive(settings, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(self)));
}

void search_command(GtkWidget * self, struct Document ** document) {
    if(*document == NULL) return;

    GtkTextIter last;
    gtk_text_buffer_get_start_iter((*document)->buffer, &last);
    (*document)->last = last;

    GtkSourceSearchSettings * settings = gtk_source_search_context_get_settings((*document)->context);

    GtkWidget * dialog = gtk_dialog_new_with_buttons(_("Find"), (*document)->window, GTK_DIALOG_DESTROY_WITH_PARENT, _("Cancel"), 0, _("Find"), 1, NULL);
    GtkWidget * content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

    GtkWidget * box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    
    GtkWidget * entry = gtk_entry_new();
    g_signal_connect(entry, "activate", G_CALLBACK(search_entry), document);
    const gchar * text = gtk_source_search_settings_get_search_text(settings);
    if (text)
        gtk_entry_set_text(GTK_ENTRY(entry), text);

    GtkWidget * label = gtk_label_new(_("Find text:"));
    GtkWidget * bubble = gtk_check_button_new_with_label(_("Match case"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(bubble), gtk_source_search_settings_get_case_sensitive(settings));
    g_signal_connect(bubble, "toggled", G_CALLBACK(match_case), settings);

    gtk_box_pack_start(GTK_BOX(box), label, 0, 0, 0);
    gtk_box_pack_end(GTK_BOX(box), entry, 0, 0, 0);

    gtk_container_add(GTK_CONTAINER(content), box);
    gtk_container_add(GTK_CONTAINER(content), bubble);
    gtk_widget_show_all(content);

    gtk_source_search_context_set_highlight((*document)->context, TRUE);
    int res = gtk_dialog_run (GTK_DIALOG (dialog));

    if (res == 1) 
        search_entry(entry, document);

    gtk_widget_destroy (dialog);
    gtk_source_search_context_set_highlight((*document)->context, FALSE);
}

void search_next_command(GtkWidget * self, struct Document ** document) {
    if(*document == NULL) return;

    search(document);
}

struct Replace {
    GtkSourceSearchSettings * settings;
    GtkWidget * search_entry;
    GtkWidget * replace_entry;
    bool all;
    struct Document ** document;
};

void search_to_replace(GtkWidget * self, struct Replace * replace) {
    search_entry(replace->search_entry, replace->document);
}

void replace(GtkWidget * self, struct Replace * replace) {

    const char * text = gtk_entry_get_text(GTK_ENTRY(replace->replace_entry));
    gtk_source_search_settings_set_search_text(replace->settings, gtk_entry_get_text(GTK_ENTRY(replace->search_entry)));
    
    if (replace->all) {
        gtk_source_search_context_replace_all((*replace->document)->context, text, strlen(text), NULL);
        return;
    }

    GtkTextIter start;
    GtkTextIter end;

    if(!gtk_text_buffer_get_selection_bounds((*replace->document)->buffer, &start, &end)) {
        search_entry(replace->search_entry, replace->document);
        gtk_text_buffer_get_selection_bounds((*replace->document)->buffer, &start, &end);
    }
    gtk_source_search_context_replace((*replace->document)->context, &start, &end, text, strlen(text), NULL);
    (*replace->document)->last = end;

}

void replace_all (GtkWidget * self, struct Replace * rep) {
    rep->all = !rep->all;
}

void replace_command(GtkWidget * self, struct Document ** document) {
    if(*document == NULL) return;
    
    GtkTextIter last;
    gtk_text_buffer_get_start_iter((*document)->buffer, &last);
    (*document)->last = last;
    GtkSourceSearchSettings * settings = gtk_source_search_context_get_settings((*document)->context);

    GtkWidget * dialog = gtk_dialog_new_with_buttons(_("Replace"), (*document)->window, GTK_DIALOG_DESTROY_WITH_PARENT, _("Cancel"), 0, NULL);
    GtkWidget * content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

    // Search
    GtkWidget * box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget * label = gtk_label_new(_("Find text:"));
    GtkWidget * entry = gtk_entry_new();
    g_signal_connect(entry, "activate", G_CALLBACK(search_entry), document);
    const gchar * text = gtk_source_search_settings_get_search_text(settings);
    if (text)
        gtk_entry_set_text(GTK_ENTRY(entry), text);
    GtkWidget * search_button = gtk_button_new_with_label(_("Find"));

    gtk_box_pack_start(GTK_BOX(box), label, 0, 0, 0);
    gtk_box_pack_start(GTK_BOX(box), entry, 0, 0, 0);
    gtk_box_pack_start(GTK_BOX(box), search_button, 0, 0, 0);
    gtk_container_add(GTK_CONTAINER(content), box);

    // Replace
    GtkWidget * box2 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget * label2 = gtk_label_new(_("Replace text"));
    GtkWidget * entry2 = gtk_entry_new();
    GtkWidget * replace_button = gtk_button_new_with_label(_("Replace"));

    struct Replace rep = {
        settings,
        entry,
        entry2,
        0,
        document
    };

    gtk_box_pack_start(GTK_BOX(box2), label2, 0, 0, 0);
    gtk_box_pack_start(GTK_BOX(box2), entry2, 0, 0, 0);
    g_signal_connect(entry2, "activate", G_CALLBACK(replace), &rep);
    gtk_box_pack_start(GTK_BOX(box2), replace_button, 0, 0, 0);
    gtk_container_add(GTK_CONTAINER(content), box2);

    GtkWidget * bubble = gtk_check_button_new_with_label(_("Match case"));
    GtkWidget * bubble2 = gtk_check_button_new_with_label(_("Replace all"));

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(bubble), gtk_source_search_settings_get_case_sensitive(settings));
    g_signal_connect(bubble, "toggled", G_CALLBACK(match_case), settings);
    g_signal_connect(bubble2, "toggled", G_CALLBACK(replace_all), &rep);

    gtk_container_add(GTK_CONTAINER(content), bubble);
    gtk_container_add(GTK_CONTAINER(content), bubble2);

    g_signal_connect(search_button, "clicked", G_CALLBACK(search_to_replace), &rep);
    g_signal_connect(replace_button, "clicked", G_CALLBACK(replace), &rep);

    gtk_widget_show_all(content);

    gtk_source_search_context_set_highlight((*document)->context, TRUE);
    gtk_dialog_run (GTK_DIALOG (dialog));

    gtk_widget_destroy (dialog);
    gtk_source_search_context_set_highlight((*document)->context, FALSE);
}

int lines_in_buffer(GtkTextBuffer * buffer) {
    int count = 0;

    GtkTextIter end;
    gtk_text_buffer_get_end_iter(buffer, &end);

    count = gtk_text_iter_get_line(&end) + 1;
    
    return count;
}

void go_to_command(GtkWidget * self, struct Document ** document) {
    if(*document == NULL) return;
    GtkWidget * dialog = gtk_dialog_new_with_buttons(_("Go To"), (*document)->window, GTK_DIALOG_DESTROY_WITH_PARENT, _("Go To"), 0, _("Cancel"), 1, NULL);
    GtkWidget * content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

    GtkWidget * line = gtk_label_new(_("Line number:"));
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
        gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW((*document)->view), &jump, 0.0, TRUE, 0.0, 0.15);
    }

    gtk_widget_destroy(dialog);

}

void font_callback(GtkFontChooser * self, gchar * selected, struct Document ** document) {
    PangoFontDescription * description = pango_font_description_from_string(selected);
    GtkCssProvider * cssProvider = gtk_css_provider_new();
    char * css = g_strdup_printf ("textview { font: %dpt %s; }", pango_font_description_get_size (description) / PANGO_SCALE, pango_font_description_get_family (description));
    gtk_css_provider_load_from_data (cssProvider, css, -1, NULL);
    GtkStyleContext * context = gtk_widget_get_style_context((*document)->view);
    gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(cssProvider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    pango_font_description_free(description);
    free(css);
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
