#ifndef FUNCTIONS_C
#define FUNCTIONS_C

#include "functions.h"

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>
#include <stdio.h>

proc first_process = {
	.Name = NULL,
	.Pid = -1, // so that it wont get confused with systemd etc
	.PPid = -1, // same as above
	.next_proc = NULL,
	.child_proc = NULL,
};

void change_pid_dir_name (char *dest, char *appendix){
	int i = 0;

	dest[0] = '/';
	dest[1] = 'p';
        dest[2] = 'r';
        dest[3] = 'o';
        dest[4] = 'c';
        dest[5] = '/';

        for (i = 0; i < strlen(appendix); i++){
                dest[6 + i] = appendix[i];
        }

        dest[6 + i + 0] = '/';
        dest[6 + i + 1] = 's';
        dest[6 + i + 2] = 't';
        dest[6 + i + 3] = 'a';
        dest[6 + i + 4] = 't';
        dest[6 + i + 5] = 'u';
        dest[6 + i + 6] = 's';
        dest[6 + i + 7] = '\0';
}

void find_string (const char *haystack, const char *needle, char *found_value){
	int needle_len = strlen(needle);
	char *buffer = strstr(haystack, needle);
	char value_buffer[255];
	
	if (buffer != NULL){
		int i = 0;

		for (i = needle_len+2; buffer[i] != '\0' && buffer[i] != '\n'; i++){
			value_buffer[i-(needle_len+2)] = buffer[i];
		}
		
		value_buffer[i-(needle_len+2)] = '\0';
	}

	strcpy(found_value, value_buffer);	
}

// inits the linked list with all the entries
int init (int fd){
	proc *curr_process = malloc(sizeof(proc));

	char *buffer = malloc(1024);
	char local_name[255];
	char *local_pid = malloc(64); // must be chars and then converted.
	char *local_ppid = malloc(64); // same as above

	proc *crawler = &first_process;
	while(crawler->next_proc != NULL){
		crawler = crawler->next_proc;
	}
	
	if (read(fd, buffer, 1024) == 0){
		printf("Some error occured.\n");
		return -1;
	}

	// "Name"
	find_string(buffer, "Name", local_name);
	char *s = malloc(strlen(local_name + 1));
	strcpy(s, local_name);
	curr_process->Name = s;

	// "Pid"
	find_string(buffer, "Pid", local_pid);
	int local_pid_int = atoi(local_pid);
	curr_process->Pid = local_pid_int;

	// "PPid"
	find_string(buffer, "PPid", local_ppid);
	int local_ppid_int = atoi(local_ppid);
	curr_process->PPid = local_ppid_int;

	// "drawn"
	curr_process->drawn = 0;

	// make a link in the linked list
	crawler->next_proc = curr_process;
	curr_process->next_proc = NULL;

	curr_process->child_proc = NULL;

	free(buffer);
	return 0;
}

int init_child_proc(){
	proc *crawler = &first_process;

	while (crawler->next_proc != NULL){ 
		insert_child_proc(crawler);

		crawler = crawler->next_proc;
	}

	return 0;
}

void insert_child_proc(proc *child){
	proc *crawler = &first_process;

	while (crawler->next_proc != NULL){
		if (crawler->Pid == child->PPid && crawler->Pid != -1){
			children_proc *c_proc = crawler->child_proc;
			if (c_proc != NULL){
				while (c_proc->next_proc != NULL){
					c_proc = c_proc->next_proc;
				}
			}

			children_proc *new_child = malloc(sizeof(children_proc));

			new_child->Name = child->Name;
			new_child->next_proc = NULL;
			new_child->link_to_proc = child;
			new_child->drawn = 0;

			if (c_proc == NULL){
				crawler->child_proc = new_child;
			}
			else {
				c_proc->next_proc = new_child;
			}
		}

		crawler = crawler->next_proc;
	}
}

int draw_all_children(proc *parent, int indentation){
	int return_value = 0;
	int already_drawn = 0;
	children_proc *child = NULL;
	if (parent->child_proc != NULL){
		child = parent->child_proc;
	}
	else{
		return 1; // we need a \n
	}

	printf("  --- %s", child->Name);
	child->drawn = 1;
	if (child->link_to_proc->child_proc != NULL && child->drawn == 0){
		return_value = draw_all_children(child->link_to_proc, indentation + 6 + strlen(child->Name)); 
		if (return_value == 2){
			parent->child_proc = NULL;
		}
	} 
	else{
		already_drawn = 1;
	}

	while (child->next_proc != NULL || child->link_to_proc->child_proc != NULL){ 
		if (return_value == 0 && already_drawn == 0 && child->drawn == 0){
			for (int i = 0; i < indentation; i++){
				printf(" ");
			}
			printf(" |--- %s", child->Name);
			child->drawn = 1;
		}

		if (child->link_to_proc->child_proc != NULL){
			child->drawn = 1;
			return_value = draw_all_children(child->link_to_proc, indentation + 6 + strlen(child->Name)); // 6 because of "  --- %s"
		}

		if (return_value != 2){
			printf("\n");
		}

		if (child->next_proc != NULL){ 
			child = child->next_proc;
		}
		else{
			parent->child_proc = NULL;
			return 2; 
		}
		return_value = 0;
		already_drawn = 0;
	}

	return 0;
}

int draw(){
	proc *crawler = &first_process;
	crawler = crawler->next_proc;

	printf("%s", crawler->Name);
	draw_all_children(crawler, strlen(crawler->Name));

	crawler = crawler->next_proc;
	printf("%s", crawler->Name);
	draw_all_children(crawler, strlen(crawler->Name));

	return 0;
}

#endif

