#include <gtksourceview/gtksource.h>
#include <sys/stat.h>
#include "global.h"
#include "explorer.h"
#include "commands.h"
#include <sys/inotify.h>

void warning_popup(GtkWindow * window, char * text) {
    GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;
    GtkWidget * dialog = gtk_message_dialog_new (window, flags, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, text, NULL);
    gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (dialog);
}

void open_command(GtkWidget * self, struct Editor * editor) {
    
    GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;
    GtkWidget *dialog = gtk_file_chooser_dialog_new (_("Open File"), editor->window, action, _("Cancel"), GTK_RESPONSE_CANCEL, _("Open"), GTK_RESPONSE_ACCEPT, NULL);

    gint res = gtk_dialog_run (GTK_DIALOG (dialog));
    if (res == GTK_RESPONSE_ACCEPT)
    {
        GtkFileChooser *chooser = GTK_FILE_CHOOSER (dialog);
        char * filename = gtk_file_chooser_get_filename (chooser);
        newpage(editor, filename);
    }

    gtk_widget_destroy (dialog);
}

void clear_editor(struct Editor * editor) {

    GtkWidget * filelist = gtk_bin_get_child(GTK_BIN(editor->expander));
    if (GTK_IS_WIDGET(filelist))
        gtk_widget_destroy(filelist);

    for (int i = 0; i < editor->filecount; i++) {
        if (editor->filesystem[i]->wd != -1) {
            inotify_rm_watch(editor->fd, editor->filesystem[i]->wd);
        }
        free(editor->filesystem[i]->path);
        free(editor->filesystem[i]);
    }
    free(editor->filesystem);
    editor->filecount = 0;
    editor->filesystem = NULL; //Necessary to avoid double free

    for (int j = 0; j < editor->len; j++) {
        free(editor->pages[j]);
    }
    
    free(editor->pages);
    editor->len = 0;
    editor->pages = NULL;
    editor->current = NULL;

}

void open_folder_command(GtkWidget * self, struct Editor * editor) {

    GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER;
    GtkWidget *dialog = gtk_file_chooser_dialog_new (_("Open Folder"), editor->window, action, _("Cancel"), GTK_RESPONSE_CANCEL, _("Open"), GTK_RESPONSE_ACCEPT, NULL);

    gint res = gtk_dialog_run (GTK_DIALOG (dialog));
    if (res == GTK_RESPONSE_ACCEPT) {
        int page;
        while((page = gtk_notebook_get_current_page(editor->tabs)) != -1) {
            gtk_notebook_remove_page(editor->tabs, page);
        }

        clear_editor(editor);

        char * dirname = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        open_explorer(editor, dirname);
    }

    gtk_widget_destroy (dialog);

}

void new_command(struct Editor * editor) {
    GError *err = NULL;
    char* argv[] = {"/proc/self/exe", NULL};
    g_spawn_async(NULL, argv, NULL, G_SPAWN_DEFAULT, NULL, NULL, NULL, &err);
    if (err != NULL) {
        warning_popup(editor->window, err->message);
        g_error_free (err);
    }
}

void new_file_command(GtkWidget * self, struct Editor * editor) {
    if (!strlen(editor->dir))
        return;
    newpage(editor, NULL);
}

void new_folder_command(GtkWidget * self, struct Editor * editor) {
    if (!strlen(editor->dir))
        return;
    GtkWidget *dialog = gtk_file_chooser_dialog_new (_("Open Folder"), editor->window, GTK_FILE_CHOOSER_ACTION_CREATE_FOLDER, _("Cancel"), GTK_RESPONSE_CANCEL, _("Open"), GTK_RESPONSE_ACCEPT, NULL);
    gint res = gtk_dialog_run (GTK_DIALOG (dialog));

    if (res == GTK_RESPONSE_ACCEPT)
    {
        char * dirname = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        if (strlen(editor->dir) + strlen(dirname) + 1 >= PATH_MAX || strlen(dirname) + 1 >= MAX_FILE) {
            warning_popup(editor->window, _("The name of your directory is too long."));
            free(dirname);
            gtk_widget_destroy(dialog);
            return;
        }
        char fullpath[PATH_MAX];
        strcpy(fullpath, editor->dir);
        strcat(fullpath, "/");
        strcat(fullpath, dirname);

        mkdir(fullpath, 0755);
        
        free(dirname);
    }
    gtk_widget_destroy (dialog);
}
bool prompt_save(struct Editor * editor) {
    GtkWidget * close = gtk_dialog_new_with_buttons("Titan", editor->current->window, GTK_DIALOG_MODAL, _("No"), 0, _("Cancel"), 1, _("Yes"), 2, NULL);
    GtkWidget * content = gtk_dialog_get_content_area(GTK_DIALOG(close));
    GtkWidget * message = gtk_label_new(_("Would you like to save?"));

    gtk_container_add(GTK_CONTAINER(content), message);
    gtk_widget_show_all(content);

    int res = gtk_dialog_run (GTK_DIALOG (close));
    gtk_widget_destroy (close);

    switch (res) {
        case 0:
            return TRUE;
        case 1:
            return FALSE;
        case 2:
            save_command(NULL, &(editor->current));
            return TRUE;
        default:
            return FALSE;
    }
}

void exit_command(GtkWidget * self, struct Editor * editor) {

    if (editor->len == 0) {
        gtk_main_quit();
        return;
    }

    for(int i = 0; i < editor->len; i++) {
        if (editor->pages[i]->type) {
            continue;
        }
        else if (gtk_text_buffer_get_modified(editor->pages[i]->buffer) == TRUE) {
            gtk_notebook_set_current_page(editor->tabs, i);

            if(prompt_save(editor)) 
                continue;
            else 
                return;
        }
    }

    gtk_main_quit();
    return;

}

void terminal_command(GtkWidget * self, struct Editor * editor) {
    gtk_widget_set_visible(GTK_WIDGET(editor->term), !gtk_widget_get_visible(GTK_WIDGET(editor->term)));
}

gboolean delete_event(GtkWidget* self, GdkEvent* event, struct Editor * editor) {
    exit_command(self, editor);
    return TRUE;
}

void close_tab_command(GtkWidget * self, struct Editor * editor) {
    if (editor->current == NULL) return;
    int x = gtk_notebook_page_num (editor->tabs, editor->current->scrolled);
    kill_tab_n(editor, x);
}

void theme_command(GtkWidget * self, struct Editor * editor) {
    const char * theme = gtk_menu_item_get_label(GTK_MENU_ITEM(self));
    GtkSourceStyleSchemeManager * manager = gtk_source_style_scheme_manager_get_default();
    GtkSourceStyleScheme * scheme = gtk_source_style_scheme_manager_get_scheme(manager, theme);

    for (int i = 0; i < editor->len; i++) {
        gtk_source_buffer_set_style_scheme(GTK_SOURCE_BUFFER(editor->pages[i]->buffer), scheme);
    }

    char * new = malloc(strlen(theme) + 1);
    strcpy(new, theme);
    editor->theme = new;
}

void about_command(GtkWidget * self, struct Editor * editor) {
    GtkWidget * about_dialog = gtk_about_dialog_new();
    GtkAboutDialog * about = GTK_ABOUT_DIALOG(about_dialog);

    gtk_about_dialog_set_program_name(about, "Titan");
    gtk_about_dialog_set_logo_icon_name(about, "titan");

    const char * authors[] = {"Gabriel Holmann <gholmann16@satyrn.net>", NULL};
    gtk_about_dialog_set_authors(about, authors);

    gtk_about_dialog_set_license_type(about, GTK_LICENSE_GPL_3_0);

    const char * comments = _("Titan is a simple gtk3 code editor intended to be small and efficient. It aims to have a similar functionality to vs code and a smaller memory footprint.");
    gtk_about_dialog_set_comments(about, comments);

    const char * website = "https://github.com/gholmann16/Titan";
    gtk_about_dialog_set_website(about, website);

    gtk_dialog_run(GTK_DIALOG(about_dialog));
    gtk_widget_destroy(about_dialog);
}
