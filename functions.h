#ifndef FUNCTIONS_H
#define FUNCTIONS_H

// in before declaration to solve circular dependency
struct proc;

typedef struct children_proc {
	char *Name;
	struct proc *link_to_proc;
	struct children_proc *next_proc;
} children_proc;

typedef struct proc {
	char *Name;
	int Pid;
	int PPid;
	struct proc *next_proc;
	children_proc *child_proc;
} proc;

void change_pid_dir_name(char *dest, char *appendix);

void find_string(const char *haystack, const char *needle, char *found_value); 

int init(int fd);

int init_child_proc();

void insert_child_proc(proc *child);

int draw();

void draw_proc(children_proc *child_crawler);

int check_for_children(proc *process);

int draw_children(proc *process, int strlen_parent);

int draw_all_children(proc *parent, int indentation);

#endif

