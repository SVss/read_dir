#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>

char *script_name = NULL;
FILE *log = NULL;   // ToDo: use log to write output

int process(char *dir_name) {   // ToDo: add files_count and overall_size to parametres (to evaluate recursively)
    DIR *cd = opendir(dir_name);

    if (!cd) {
        perror(script_name);
        return 1;
    }

    ulong overall_size = 0;
    struct stat st;

    char *curr_name = alloca(strlen(dir_name) + NAME_MAX + 3);
    curr_name[0] = 0;
    strcat(curr_name, dir_name);
    strcat(curr_name, "/");
    size_t curr_len = strlen(curr_name);

    size_t max_size = 0;
    char *max_file = alloca(NAME_MAX);
    max_file[0] = 0;

    int files_count = 0;    // ToDo: move to parametres (to evaluate recursively)

    struct dirent *entry = alloca(sizeof(struct dirent) );

    errno = 0;
    entry = readdir(cd);

    while (entry != NULL) {
        curr_name[curr_len] = 0;
        strcat(curr_name, entry->d_name);

        if (stat(curr_name, &st) == -1) {
            perror(script_name);
        }

        if ( S_ISDIR(st.st_mode) )
        {
            if ( (strcmp(entry->d_name, ".") != 0) && (strcmp(entry->d_name, "..") != 0) )  {
//                printf("%s\n", entry->d_name);
                process(curr_name);
            }
        }
        else if (S_ISREG(st.st_mode) ) {
//            printf("%s\n", curr_name);
            overall_size += st.st_size;
            ++files_count;

            if (st.st_size > max_size)
            {
                max_size = st.st_size;
                max_file[0] = 0;
                strcpy(max_file, entry->d_name);
            }
        }

        entry = readdir(cd);
    }

    if (errno != 0) {
        perror(script_name);
    }

    if (closedir(cd) == -1) {
        perror(script_name);
        return 1;
    }

    printf("%s %d %d %s\n", dir_name, files_count, overall_size, max_file);

    return 0;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Not enough arguments.");
        return 1;
    }

    script_name = argv[0];  // ToDo: change to basename (without prefix and path)

    char *dir_name = argv[1];
    log = fopen(argv[2], "w");

    process(dir_name);

    fclose(log);
    return 0;
}
