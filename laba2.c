#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>

char *script_name = NULL;
FILE *log = NULL;

int process(char *dir_name) {
	DIR *cd = opendir(dir_name);
	
	if (cd == -1) {
		perror(script_name);
		return 1;
	}
	
	ulong overall = 0;
	struct stat st;
	
	char *curr_name = alloca(strlen(dir_name) + NAME_MAX + 3);
	curr_name[0] = 0;
	strcat(curr_name, dir_name);
	strcat(curr_name, "/");
	size_t curr_len = strlen(curr_name);
	
	int files_count = 0;
	
	struct dirent *entry = alloca(sizeof(struct dirent) );
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
//				printf("%s\n", entry->d_name);
				process(curr_name);
			}
		}
		else if (S_ISREG(st.st_mode) ) {
			printf("%s\n", curr_name);
			overall += st.st_size;
			++files_count;
		}
				
		entry = readdir(cd);
	}
	
	if (errno != 0) {
		perror(script_name);
	}
	
//	fprintf(
	
	if (closedir(cd) == -1) {
		perror(script_name);
		return 1;
	}
	
	return 0;
}

int main(int argc, char *argv[]) {	
	if (argc < 3) {
		fprintf(stderr, "Not enough arguments.");
		return 1;
	}
	
	script_name = argv[0];
	char *dir_name = argv[1];
	log = fopen(argv[2], "w");
	
	process(dir_name);
	
	fclose(log);
	return 0;
}
