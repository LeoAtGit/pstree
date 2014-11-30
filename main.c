#include "functions.h"

#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int main(){
	char pid_dir_name[255+6];

	struct dirent *dir_entry;
	DIR *directory;

	char *endptr = NULL;

	int fd = 0;

	pid_dir_name[0] = '\0';

	directory = opendir("/proc/");
	dir_entry = readdir(directory);

	// Now we will initialize the linked list with the values from the status files
	// We will not initialize the .child_proc linked list of each node
	while (dir_entry != NULL){
		if (strcmp(dir_entry->d_name, ".") == 0 || strcmp(dir_entry->d_name, ",,") == 0){
			dir_entry = readdir(directory);
			continue;
		}

		strtol(dir_entry->d_name, &endptr, 10);
		if (*endptr != '\0'){
			dir_entry = readdir(directory);
			continue;
		}

		change_pid_dir_name(pid_dir_name, dir_entry->d_name);

		// fd == the status file 
		fd = open(pid_dir_name, O_RDONLY);
		if (fd == -1){
			printf("Couldnt open the status file, maybe the process terminated.\n");
			if (fd) close (fd);
		}

		init(fd);

		if (close(fd) == -1){
			printf("The file couldnt be closed, It doesnt exist anymore.\n");
		}

		dir_entry = readdir(directory);
	}

	// Now we can start to initialize the Linked list a little more. Since we have all of the entries we want, we look who is whose child process and add that to the list
	init_child_proc();

	// As the last step, we will print out the stuff
	
	draw();	

	return 0;
}


