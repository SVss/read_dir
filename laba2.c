#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <alloca.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>

#define ARGS_COUNT 3
#define INODES_SIZE 255

char *script_name = NULL;
FILE *log_file = NULL;

typedef struct t_res {
    unsigned long files_count;
    off_t dir_size;
    off_t max_size;
    char max_file[NAME_MAX];

} t_count_size;

static const t_count_size EMPTY_T_COUNT_SIZE;


void print_dir(char *name, size_t count, off_t size, char *max_file)
{
    printf("%s %ld %ld %s\n", name, count, size, max_file);
    if (log_file) {
        fprintf(log_file, "%s %ld %ld %s\n", name, count, size, max_file);
    }
}

void print_error(const char *s_name, const char *msg, const char *f_name)
{
    fprintf(stderr, "%s: %s %s\n", s_name, msg, (f_name)? f_name : "");
    if (log_file) {
        fprintf(log_file, "%s: %s %s\n", s_name, msg, (f_name)? f_name : "");
    }
}

void process(char *dir_name) {
    t_count_size curr = EMPTY_T_COUNT_SIZE;

    DIR *cd = opendir(dir_name);
    if (!cd) {
        print_error(script_name, strerror(errno), dir_name);
        return;
    }

    char *curr_name = alloca(strlen(dir_name) + NAME_MAX + 3);
    curr_name[0] = 0;
    strcat(curr_name, dir_name);
    strcat(curr_name, "/");
    size_t curr_name_len = strlen(curr_name);

    curr.max_size = 0;
    curr.max_file[0] = 0;

    struct dirent *entry = alloca(sizeof(struct dirent) );
    struct stat st;

    size_t ilist_len = INODES_SIZE;
    ino_t *ilist = malloc(ilist_len * sizeof(ino_t) );
    int ilist_next = 0;

    errno = 0;

    while ( (entry = readdir(cd) ) ) {
        curr_name[curr_name_len] = 0;
        strcat(curr_name, entry->d_name);

        if (lstat(curr_name, &st) == -1) {
            print_error(script_name, strerror(errno), curr_name);
        }
        else {
            ino_t ino = st.st_ino;

            if (ilist_next == ilist_len) {
                ilist_len *= 2;
                ilist = (ino_t*)realloc(ilist, ilist_len*sizeof(ino_t) );
            }

            int i = 0;
            while ( (i < ilist_next) && (ino != ilist[i]) )
                ++i;

            if (i == ilist_next)
            {
                ilist[ilist_next] = ino;
                ++ilist_next;

                if ( S_ISDIR(st.st_mode) )
                {
                    if ( (strcmp(entry->d_name, ".") != 0) && (strcmp(entry->d_name, "..") != 0) )  {
                        process(curr_name);
                    }
                }
                else if ( S_ISREG(st.st_mode) )
                {
                    if (st.st_size >= curr.max_size) {
                        curr.max_size = st.st_size;
                        strcpy(curr.max_file, entry->d_name);
                    }

                    ++curr.files_count;
                    curr.dir_size += st.st_size;
                }
            }
        }
    }

    if (errno != 0) {
        print_error(script_name, strerror(errno), curr_name);
    }

    if (closedir(cd) == -1) {
        print_error(script_name, strerror(errno), dir_name);
    }
    else {
        print_dir(dir_name, curr.files_count, curr.dir_size, curr.max_file);
    }

    free(ilist);
    return;
}


int main(int argc, char *argv[]) {
    script_name = basename(argv[0]);

    if (argc < ARGS_COUNT) {
        print_error(script_name, "Not enough arguments.", 0);
        return 1;
    }

    char *dir_name = realpath(argv[1], NULL);
    if (dir_name == NULL) {
        print_error(script_name, "Error opening directory", argv[1]);
        return 1;
    }

    if ( (log_file = fopen(argv[2], "w") ) == NULL) {
        print_error(script_name, "Can't open log-file.", argv[2]);
    }

    process(dir_name);

    if (log_file) {
        if (fclose(log_file) == -1)
            print_error(script_name, "Error closing logfile", realpath(argv[2], NULL) );
    }

    return 0;
}
