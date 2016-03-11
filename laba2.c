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

char *script_name = NULL;
FILE *log_file = NULL;

typedef struct t_res {
    unsigned long files_count;
    off_t dir_size;
    off_t max_size;
    char max_file[PATH_MAX];

} t_count_size;

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

t_count_size process(char *dir_name) {
    t_count_size curr;
    curr.dir_size = 0;
    curr.files_count = 0;
    curr.max_file[0] = 0;
    curr.max_size = 0;

    DIR *cd = opendir(dir_name);
    if (!cd) {
        print_error(script_name, strerror(errno), dir_name);
        return curr;
    }

    struct stat st;

    char *curr_name = alloca(strlen(dir_name) + NAME_MAX + 3);
    curr_name[0] = 0;
    strcat(curr_name, dir_name);
    strcat(curr_name, "/");
    size_t curr_len = strlen(curr_name);

    curr.max_size = 0;
    curr.max_file[0] = 0;

    struct dirent *entry = alloca(sizeof(struct dirent) );

    errno = 0;
    entry = readdir(cd);

    while (entry != NULL) {
        curr_name[curr_len] = 0;
        strcat(curr_name, entry->d_name);

        if (lstat(curr_name, &st) == -1) {
            perror(script_name);
        }

        if ( S_ISDIR(st.st_mode) )
        {
            if ( (strcmp(entry->d_name, ".") != 0) && (strcmp(entry->d_name, "..") != 0) )  {
                t_count_size forward = process(curr_name);

                curr.dir_size += forward.dir_size;
                curr.files_count += forward.files_count;

                if (forward.max_size >= curr.max_size)
                {
                    curr.max_size = st.st_size;
                    strcpy(curr.max_file, forward.max_file);
                }

                ++curr.files_count;
                curr.dir_size += st.st_size;
            }
        }
        else if (S_ISREG(st.st_mode) ) {
            curr.dir_size += st.st_size;
            ++curr.files_count;

            if (st.st_size >= curr.max_size)
            {
                curr.max_size = st.st_size;
                strcpy(curr.max_file, curr_name);
            }
        }

        entry = readdir(cd);
    }

    if (errno != 0) {
        print_error(script_name, strerror(errno), curr_name);
    }

    if (closedir(cd) == -1) {
        print_error(script_name, strerror(errno), dir_name);
        return curr;
    }

    print_dir(dir_name, curr.files_count, curr.dir_size, curr.max_file);

    return curr;
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
        fclose(log_file);
    }

    return 0;
}
