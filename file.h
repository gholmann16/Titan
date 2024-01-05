struct BetterString {
    char * contents;
    size_t length;
};

struct BetterString * file_text(char * filename);
int is_dir(struct dirent * ent);