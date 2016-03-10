#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

char *script_name = NULL;
FILE *log = NULL;

int process(char *dir_name, int padding) {
	DIR *cd = opendir(dir_name);
	
	if (cd == -1) {
		perror(script_name);
		return 1;
	}
	
	ulong overall = 0;
	struct stat st;
	
	struct dirent *entry = alloca(sizeof(struct dirent) );
	entry = readdir(cd);
	
	char *curr_name = alloca(strlen(dir_name) + 	NAME_MAX + 3);
	strcat(curr_name, dir_name);
	strcat(curr_name, "/");
	size_t curr_len = strlen(curr_name);
	
//	int i = 0;
	
	while (entry != NULL) {
		curr_name[curr_len] = 0;
		strcat(curr_name, entry->d_name);
		printf("%s\n", curr_name);
		
		if (entry->d_type == DT_DIR) {
//			process(curr_name, padding+1);
		}
		else if (entry->d_type == DT_REG) {
//			printf("%s\n", entry->d_name);
			/*
			if (stat(entry->d_name, &st) == -1)
			{
				overall += st.st_size;
			}
			* */
		}
		
		/*
		for (i = 0; i < padding; ++i) {
			putchar('\t');
		}
		printf("%s\n", entry->d_name);
		*/
		
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
	
	process(dir_name, 0);
	
	fclose(log);
	return 0;
}
