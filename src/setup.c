#include <gtksourceview/gtksource.h>
#include "global.h"
#include "commands.h"
#include "window.h"

void init_app(GtkWindow * window) {
    char path[PATH_MAX] = "";
    if (getenv("APPDIR") && strlen(getenv("APPDIR")) < PATH_MAX - strlen("/usr/share/locale/")) {
        strcpy(path, getenv("APPDIR"));
        strcat(path, "/usr/share/icons");
        gtk_icon_theme_append_search_path(gtk_icon_theme_get_default(), path);
        strcpy(path, getenv("APPDIR"));
    }
    strcat(path, "/usr/share/locale/");

    setlocale(LC_ALL, "");
    bindtextdomain("titan", path);
    bind_textdomain_codeset("titan", "utf-8");
    textdomain("titan");

    gtk_window_set_icon_name(window, "titan");
}

void init_preferences(struct Editor * editor);

void init_menu(GtkWidget * bar, GtkAccelGroup * accel, struct Document ** document, struct Editor * editor) {

    // File menu

    GtkWidget * file = gtk_menu_item_new_with_label("File");
    gtk_menu_shell_append(GTK_MENU_SHELL(bar), file);
    
    GtkWidget * filemenu = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(file), filemenu);

    GtkWidget * newfile = gtk_menu_item_new_with_label(_("New File"));
    gtk_widget_add_accelerator(newfile, "activate", accel, GDK_KEY_N, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    g_signal_connect(newfile, "activate", G_CALLBACK(new_file_command), editor);
    gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), newfile);

    GtkWidget * newfolder = gtk_menu_item_new_with_label(_("New Folder"));
    g_signal_connect(newfolder, "activate", G_CALLBACK(new_folder_command), editor);
    gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), newfolder);

    GtkWidget * new = gtk_menu_item_new_with_label(_("New Window"));
    g_signal_connect(new, "activate", G_CALLBACK(new_command), editor);
    gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), new);

    GtkWidget * seperate0 = gtk_separator_menu_item_new();
    gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), seperate0);

    GtkWidget * open = gtk_menu_item_new_with_label(_("Open"));
    gtk_widget_add_accelerator(open, "activate", accel, GDK_KEY_O, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    g_signal_connect(open, "activate", G_CALLBACK(open_command), editor);
    gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), open);

    GtkWidget * folder = gtk_menu_item_new_with_label(_("Open Folder"));
    gtk_widget_add_accelerator(folder, "activate", accel, GDK_KEY_K, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    g_signal_connect(folder, "activate", G_CALLBACK(open_folder_command), editor);
    gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), folder);

    GtkWidget * save = gtk_menu_item_new_with_label(_("Save"));
    gtk_widget_add_accelerator(save, "activate", accel, GDK_KEY_S, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    g_signal_connect(save, "activate", G_CALLBACK(save_command), document);
    gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), save);

    GtkWidget * save_as = gtk_menu_item_new_with_label(_("Save as..."));
    g_signal_connect(save_as, "activate", G_CALLBACK(save_as_command), document);
    gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), save_as);

    GtkWidget * seperate = gtk_separator_menu_item_new();
    gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), seperate);
    
    GtkWidget * print = gtk_menu_item_new_with_label(_("Print..."));
    gtk_widget_add_accelerator(print, "activate", accel, GDK_KEY_P, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    g_signal_connect(print, "activate", G_CALLBACK(print_command), document);
    gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), print);

    GtkWidget * preview = gtk_menu_item_new_with_label(_("Print Preview..."));
    g_signal_connect(preview, "activate", G_CALLBACK(print_preview_command), document);
    gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), preview);

    GtkWidget * seperate1 = gtk_separator_menu_item_new();
    gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), seperate1);

    GtkWidget * close = gtk_menu_item_new_with_label(_("Close tab"));
    gtk_widget_add_accelerator(close, "activate", accel, GDK_KEY_W, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    g_signal_connect(close, "activate", G_CALLBACK(close_tab_command), editor);
    gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), close);

    GtkWidget * exit = gtk_menu_item_new_with_label(_("Exit"));
    gtk_widget_add_accelerator(exit, "activate", accel, GDK_KEY_Q, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    g_signal_connect(exit, "activate", G_CALLBACK(exit_command), editor);
    gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), exit);

    // Edit menu

    GtkWidget * edit = gtk_menu_item_new_with_label(_("Edit"));
    gtk_menu_shell_append(GTK_MENU_SHELL(bar), edit);

    GtkWidget * editmenu = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(edit), editmenu);

    GtkWidget * undo = gtk_menu_item_new_with_label(_("Undo"));
    gtk_widget_add_accelerator(undo, "activate", accel, GDK_KEY_Z, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    g_signal_connect(undo, "activate", G_CALLBACK(undo_command), document);
    gtk_menu_shell_append(GTK_MENU_SHELL(editmenu), undo);

    GtkWidget * redo = gtk_menu_item_new_with_label(_("Redo"));
    gtk_widget_add_accelerator(redo, "activate", accel, GDK_KEY_Y, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    g_signal_connect(redo, "activate", G_CALLBACK(redo_command), document);
    gtk_menu_shell_append(GTK_MENU_SHELL(editmenu), redo);

    GtkWidget * seperate2 = gtk_separator_menu_item_new();
    gtk_menu_shell_append(GTK_MENU_SHELL(editmenu), seperate2);

    GtkWidget * cut = gtk_menu_item_new_with_label(_("Cut"));
    gtk_widget_add_accelerator(cut, "activate", accel, GDK_KEY_X, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    g_signal_connect(cut, "activate", G_CALLBACK(cut_command), document);
    gtk_menu_shell_append(GTK_MENU_SHELL(editmenu), cut);

    GtkWidget * copy = gtk_menu_item_new_with_label(_("Copy"));
    gtk_widget_add_accelerator(copy, "activate", accel, GDK_KEY_C, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    g_signal_connect(copy, "activate", G_CALLBACK(copy_command), document);
    gtk_menu_shell_append(GTK_MENU_SHELL(editmenu), copy);

    GtkWidget * paste = gtk_menu_item_new_with_label(_("Paste"));
    gtk_widget_add_accelerator(paste, "activate", accel, GDK_KEY_V, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    g_signal_connect(paste, "activate", G_CALLBACK(paste_command), document);
    gtk_menu_shell_append(GTK_MENU_SHELL(editmenu), paste);

    GtkWidget * delete = gtk_menu_item_new_with_label(_("Delete"));
    g_signal_connect(delete, "activate", G_CALLBACK(delete_command), document);
    gtk_menu_shell_append(GTK_MENU_SHELL(editmenu), delete);

    GtkWidget * seperate3 = gtk_separator_menu_item_new();
    gtk_menu_shell_append(GTK_MENU_SHELL(editmenu), seperate3);

    GtkWidget * select_all = gtk_menu_item_new_with_label(_("Select all"));
    gtk_widget_add_accelerator(select_all, "activate", accel, GDK_KEY_A, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    g_signal_connect(select_all, "activate", G_CALLBACK(select_all_command), document);
    gtk_menu_shell_append(GTK_MENU_SHELL(editmenu), select_all);

    GtkWidget * search = gtk_menu_item_new_with_label(_("Search"));
    gtk_menu_shell_append(GTK_MENU_SHELL(bar), search);

    GtkWidget * searchmenu = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(search), searchmenu);

    GtkWidget * search_button = gtk_menu_item_new_with_label(_("Find"));
    gtk_widget_add_accelerator(search_button, "activate", accel, GDK_KEY_F, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    g_signal_connect(search_button, "activate", G_CALLBACK(search_command), document);
    gtk_menu_shell_append(GTK_MENU_SHELL(searchmenu), search_button);
    
    GtkWidget * search_next = gtk_menu_item_new_with_label(_("Find next"));
    gtk_widget_add_accelerator(search_next, "activate", accel, GDK_KEY_F3, 0, GTK_ACCEL_VISIBLE);
    g_signal_connect(search_next, "activate", G_CALLBACK(search_next_command), document);
    gtk_menu_shell_append(GTK_MENU_SHELL(searchmenu), search_next);

    GtkWidget * replace = gtk_menu_item_new_with_label(_("Replace"));
    gtk_widget_add_accelerator(replace, "activate", accel, GDK_KEY_H, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    g_signal_connect(replace, "activate", G_CALLBACK(replace_command), document);
    gtk_menu_shell_append(GTK_MENU_SHELL(searchmenu), replace);

    GtkWidget * seperate4 = gtk_separator_menu_item_new();
    gtk_menu_shell_append(GTK_MENU_SHELL(searchmenu), seperate4);

    GtkWidget * go_to = gtk_menu_item_new_with_label(_("Go to"));
    gtk_widget_add_accelerator(go_to, "activate", accel, GDK_KEY_G, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    g_signal_connect(go_to, "activate", G_CALLBACK(go_to_command), document);
    gtk_menu_shell_append(GTK_MENU_SHELL(searchmenu), go_to);

    GtkWidget * view = gtk_menu_item_new_with_label(_("View"));
    gtk_menu_shell_append(GTK_MENU_SHELL(bar), view);

    GtkWidget * viewmenu = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(view), viewmenu);

    GtkWidget * theme = gtk_menu_item_new_with_label(_("Theme"));
    gtk_menu_shell_append(GTK_MENU_SHELL(viewmenu), theme);

    GtkWidget * theme_submenu = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(theme), theme_submenu);

    GtkSourceStyleSchemeManager * manager = gtk_source_style_scheme_manager_get_default();
    const gchar* const* themes = gtk_source_style_scheme_manager_get_scheme_ids(manager);

    int count = 0;
    while(themes[count] != NULL) {
        GtkWidget * current_theme = gtk_menu_item_new_with_label(themes[count]);
        gtk_menu_shell_append(GTK_MENU_SHELL(theme_submenu), current_theme);
        g_signal_connect(current_theme, "activate", G_CALLBACK(theme_command), editor);
        count++;
    }

    GtkWidget * terminal = gtk_menu_item_new_with_label(_("Terminal"));
    gtk_widget_add_accelerator(terminal, "activate", accel, GDK_KEY_J, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    g_signal_connect(terminal, "activate", G_CALLBACK(terminal_command), editor);
    gtk_menu_shell_append(GTK_MENU_SHELL(viewmenu), terminal);

    GtkWidget * options = gtk_menu_item_new_with_label(_("Options"));
    gtk_menu_shell_append(GTK_MENU_SHELL(bar), options);

    GtkWidget * optionsmenu = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(options), optionsmenu);

    GtkWidget * fonts = gtk_menu_item_new_with_label(_("Font..."));
    g_signal_connect(fonts, "activate", G_CALLBACK(font_command), document);
    gtk_menu_shell_append(GTK_MENU_SHELL(optionsmenu), fonts);

    GtkWidget * wrap = gtk_check_menu_item_new_with_label(_("Wrap line"));
    g_signal_connect(wrap, "activate", G_CALLBACK(wrap_command), document);
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(wrap), FALSE);
    gtk_menu_shell_append(GTK_MENU_SHELL(optionsmenu), wrap);

    GtkWidget * spaces = gtk_check_menu_item_new_with_label(_("Convert tabs to spaces"));
    g_signal_connect(spaces, "activate", G_CALLBACK(spaces_command), document);
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(spaces), TRUE);
    gtk_menu_shell_append(GTK_MENU_SHELL(optionsmenu), spaces);

    GtkWidget * help = gtk_menu_item_new_with_label(_("Help"));
    gtk_menu_shell_append(GTK_MENU_SHELL(bar), help);

    GtkWidget * helpmenu = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(help), helpmenu);

    GtkWidget * about = gtk_menu_item_new_with_label(_("About"));
    g_signal_connect(about, "activate", G_CALLBACK(about_command), editor);
    gtk_menu_shell_append(GTK_MENU_SHELL(helpmenu), about);

}
