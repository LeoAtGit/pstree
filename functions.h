#ifndef FUNCTIONS_H
#define FUNCTIONS_H

// in before declaration to solve circular dependency
struct proc;

/* data structure which holds some information about the children processes.
 *
 * Name: Name of the child process
 * drawn: 0 = not already drawn on the screen, 1 = already drawn on the screen -> no need to draw it again
 * link_to_proc: A link to the proc data structure of that process for further information about the process
 * next_proc: A linked list of all the children of that one process. 
 *
*/
typedef struct children_proc {
	char *Name;
	int drawn;
	struct proc *link_to_proc;
	struct children_proc *next_proc;	
} children_proc;

/*
 * Name: Name of the process
 * Pid: Pid of the process
 * PPid: PPid of the process
 * drawn: 0 = not already drawn on the screen, 1 = already drawn on the screen -> no need to draw it again
 * next_proc: A linked list of all the processes which had an entry in /proc/xxx
 * child_proc: A link to the linked list of all of the children of the process
 *
*/
typedef struct proc {
	char *Name;
	int Pid;
	int PPid;
	int drawn;
	struct proc *next_proc;
	children_proc *child_proc;
} proc;

/*
 * Changes the directory name from appendix to 
 * "/proc/appendix/status"
 * This new string is saved in dest
 *
 * */
void change_pid_dir_name(char *dest, char *appendix);

/*
 * Find the string needle in the haystack 
 * Returns the value associated with the string in the variable found_value
 *
 * */
void find_string(const char *haystack, const char *needle, char *found_value); 

/*
 * Initializes the linked list of all the processes, but with no entries on the child_proc.
 * Return -1 on error
 *
 * */
int init(int fd);

/*
 * Goes through the currently available linked list and adds the entries child_proc to each process that has a child
 *
 * */
int init_child_proc();

/*
 * Inserts child to the process where it belongs to
 *
 * */
void insert_child_proc(proc *child);

/*
 * Draws the first entries 
 * Which should be the processes with Pid = 1 (i.e. systemd or similar)
 * And the process		 with Pid = 2 (should be kthreadd)
 *
 * */
int draw();

/*
 * Draws all of the children of parent with an indentation of indentation
 *
 * */
int draw_all_children(proc *parent, int indentation);

#endif

