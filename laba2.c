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
    fprintf(log_file, "%s %ld %ld %s\n", name, count, size, max_file);
    printf("%s %ld %ld %s\n", name, count, size, max_file);
}

t_count_size process(char *dir_name) {
    t_count_size curr;
    curr.dir_size = 0;
    curr.files_count = 0;
    curr.max_file[0] = 0;
    curr.max_size = 0;

    DIR *cd = opendir(dir_name);
    if (!cd) {
        perror(script_name);
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
        perror(script_name);
    }

    if (closedir(cd) == -1) {
        perror(script_name);
        return curr;
    }

    print_dir(dir_name, curr.files_count, curr.dir_size, curr.max_file);

    return curr;
}


int main(int argc, char *argv[]) {
    if (argc < ARGS_COUNT) {
        fprintf(stderr, "Not enough arguments.");
        return 1;
    }

    script_name = basename(argv[0]);

    char *dir_name = realpath(argv[1], NULL);
    if (dir_name == NULL) {
        fprintf(stderr, "%s: Error opening directory: %s", script_name, argv[1]);
        return 1;
    }

    log_file = fopen(argv[2], "w");

    process(dir_name);

    fclose(log_file);

    free(script_name);
    return 0;
}
