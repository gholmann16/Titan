struct File * get_file_from_path(char * path, struct Editor * editor);

struct File * get_file(GtkWidget * self, struct Editor * editor);

struct File * new_file_struct(struct Editor * editor);

char * gen_path(char * dir, char * name);

struct File * get_dir(int wd, struct Editor * editor);