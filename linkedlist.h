#ifndef _LINKEDLIST_H_
#define _LINKEDLIST_H_

typedef struct process_list process_list_t;
struct process_list
{
    pid_t pid;
    char path[50];
    process_list_t *next;
};

void *emalloc(size_t n);
process_list_t *new_process();
process_list_t *add_front(process_list_t *p_list, char *p_path, pid_t pid);
process_list_t *remove_process(process_list_t *p_list, pid_t pid);
int find_process(process_list_t *p_list, pid_t pid);

#endif