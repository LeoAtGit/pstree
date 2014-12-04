#ifndef FUNCTIONS_C
#define FUNCTIONS_C

#include "functions.h"

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>
#include <stdio.h>

/*
 * Define an entry point for your linked list.
 *
 * */
proc first_process = {
	.Name = NULL,
	.Pid = -1, // I chose the value -1 so that it wont get confused with systemd etc
	.PPid = -1, // same reason as above
	.next_proc = NULL,
	.child_proc = NULL,
	.drawn = 0,
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

	// find the string and format it
	char *buffer = strstr(haystack, needle);
	char value_buffer[255];
	
	if (buffer != NULL){
		int i = 0;

		for (i = needle_len+2; buffer[i] != '\0' && buffer[i] != '\n'; i++){
			value_buffer[i-(needle_len+2)] = buffer[i];
		}
		
		// format it correctly with an terminating \0 at the end.
		value_buffer[i-(needle_len+2)] = '\0';
	}

	strcpy(found_value, value_buffer);	
}

// initializes the linked list with all the entries
int init (int fd){
	proc *curr_process = malloc(sizeof(proc));

	char *buffer = malloc(1024);
	char local_name[255];
	char *local_pid = malloc(64); // must be chars and then converted. (64 is just an arbitrary number)
	char *local_ppid = malloc(64); // same as above

	// start at the startpoint of the linked list and go to the end of the linked list
	proc *crawler = &first_process;
	while(crawler->next_proc != NULL){
		crawler = crawler->next_proc;
	}
	
	if (read(fd, buffer, 1024) == 0){
		printf("Some error occured.\n");
		return -1;
	}

	// search for the string "Name" and save the string in the new node of the linked list
	find_string(buffer, "Name", local_name);
	char *s = malloc(strlen(local_name + 1));
	strcpy(s, local_name);
	curr_process->Name = s;

	// search for the string "Pid" and convert the number to an int and save this number in the new node of the linked list
	find_string(buffer, "Pid", local_pid);
	int local_pid_int = atoi(local_pid);
	curr_process->Pid = local_pid_int;

	// search for the string "PPid" and convert the number to an int and save this number in the new node of the linked list
	find_string(buffer, "PPid", local_ppid);
	int local_ppid_int = atoi(local_ppid);
	curr_process->PPid = local_ppid_int;

	// initialize the variable "drawn" and set it to false in the new node of the linked list
	curr_process->drawn = 0;

	// make a new link in the linked list
	crawler->next_proc = curr_process;
	curr_process->next_proc = NULL;

	curr_process->child_proc = NULL;

	// clean up
	free(buffer);
	free(local_pid);
	free(local_ppid);

	return 0;
}

int init_child_proc(){
	proc *crawler = &first_process;

	// crawl along the linked list and add a child_proc to the nodes
	while (crawler->next_proc != NULL){ 
		insert_child_proc(crawler);

		crawler = crawler->next_proc;
	}

	return 0;
}

void insert_child_proc(proc *child){
	proc *crawler = &first_process;

	// goes through the whole linked list and looks for matches of ones process PPid with the others process Pid
	while (crawler->next_proc != NULL){
		if (crawler->Pid == child->PPid && crawler->Pid != -1){
			children_proc *c_proc = crawler->child_proc;
			if (c_proc != NULL){
				while (c_proc->next_proc != NULL){
					c_proc = c_proc->next_proc;
				}
			}

			children_proc *new_child = malloc(sizeof(children_proc));

			// initialize the new child process
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
	int return_value = 0; 	// 0 means there are more children of the process. 
				// 1 means that we reached the end of a linked list and therefore need an \n
				// 2 means that we ran through all entries in one linked list of childs and therefore dont need a second \n
	int already_drawn = 0;  
	children_proc *child = NULL;

	// check if the process has a child process
	if (parent->child_proc != NULL){
		child = parent->child_proc;
	}
	else{
		return 1; // we need a \n
	}

	printf("  --- %s", child->Name);
	child->drawn = 1;

	// if the process has a child process, which wasnt already drawn on the screen then start with the recursion
	if (child->link_to_proc->child_proc != NULL && child->drawn == 0){
		return_value = draw_all_children(child->link_to_proc, indentation + 6 + strlen(child->Name)); // the number 6 is the strlen of " |--- %s"

		// this is a workaround so that there are no duplicate entries on the screen. //TODO implement here a cleaner() function
		if (return_value == 2){
			parent->child_proc = NULL;
		}
	} 
	else{
		already_drawn = 1;
	}

	// go through the linked list of the child processes
	while (child->next_proc != NULL || child->link_to_proc->child_proc != NULL){ 
		// indentation with spaces
		if (return_value == 0 && already_drawn == 0 && child->drawn == 0){
			for (int i = 0; i < indentation; i++){
				printf(" ");
			}
			printf(" |--- %s", child->Name);
			child->drawn = 1;
		}

		// if the child has more children then print them first, with recursion
		if (child->link_to_proc->child_proc != NULL){
			child->drawn = 1;
			return_value = draw_all_children(child->link_to_proc, indentation + 6 + strlen(child->Name)); // the number 6 is the strlen of "  --- %s"
		}

		if (return_value != 2){
			printf("\n");
		}

		if (child->next_proc != NULL){ 
			child = child->next_proc;
		}
		else{
			// again here the workaround so that we dont see duplicated entries on the screen. //TODO implement here a cleaner() function
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

	// this is going to be the process with Pid = 1, i.e. systemd or init or whatever
	printf("%s", crawler->Name);
	draw_all_children(crawler, strlen(crawler->Name));

	crawler = crawler->next_proc;

	// this is going to be the process with the Pid = 2, i.e. kthreadd. 
	// This is normally not shown on the original pstree, if you want the original functionality then comment the next part out.
	printf("%s", crawler->Name);
	draw_all_children(crawler, strlen(crawler->Name));

	return 0;
}

#endif

