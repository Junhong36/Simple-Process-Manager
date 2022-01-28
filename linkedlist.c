#include <stdio.h>
#include <stdlib.h>
#include <string.h> // define,string processing,memset,etc
#include "linkedlist.h"

/*
 * Wrapper around malloc() -- ends the program if malloc() fails
 * (i.e., eliminates the need of an "if" statement around each
 * call to malloc() in the main body of code).
 */
void *emalloc(size_t n)
{
    void *p;

    p = malloc(n);
    if (p == NULL)
    {
        fprintf(stderr, "malloc of %zu bytes failed", n);
        exit(1);
    }

    return p;
}

/*
 * New linked-list node. Beware as that only "next" field and "path" 
 * field get initialized;  everything else is uninitialized.
 */
process_list_t *new_process()
{
    process_list_t *new_p = NULL;

    new_p = (process_list_t *)emalloc(sizeof(process_list_t));
    memset(new_p->path, '\0', sizeof(new_p->path));
    new_p->next = NULL;

    return new_p;
}

/*
 * At the end of this operation, the "p_path" and "pid" passed in 
 * will be at the head of the returned list. Note that "list" here 
 * really is an address to some process_list_t.
 */
process_list_t *add_front(process_list_t *p_list, char *p_path, pid_t pid)
{
    process_list_t *new_t = new_process();

    new_t->pid = pid;
    strncpy(new_t->path, p_path, strlen(p_path));
    new_t->next = p_list;
    return new_t;
}

/*
 * De-allocate memory for a linked-list node.
 */
process_list_t *remove_process(process_list_t *p_list, pid_t pid)
{
    process_list_t *temp = NULL;
    process_list_t *pre = NULL;

    if (p_list == NULL)
    {
        return p_list;
    }

    for (temp = p_list; temp != NULL; temp = temp->next)
    {

        if (temp->pid == pid)
        {
            if (pre == NULL)
            {
                p_list = temp->next;
            }
            else
            {
                pre->next = temp->next;
            }

            free(temp);

            return p_list;
        }
        pre = temp;
    }

    return p_list;
}

/*
 * Find out if the node with the "pid" exists, then return 1 if yes.
 * Otherwise, return 0.
 */
int find_process(process_list_t *p_list, pid_t pid)
{
    process_list_t *temp = NULL;

    if (p_list == NULL)
    {
        return 0;
    }

    for (temp = p_list; temp != NULL; temp = temp->next)
    {

        if (temp->pid == pid)
        {
            return 1;
        }
    }

    return 0;
}