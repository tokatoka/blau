#include "./util.h"

void new_queue(struct list *lst){
	lst -> prev = lst;
	lst -> next = lst;
}

void add_queue(struct list *lst, struct list *item){
	lst -> prev -> next = item;
	lst -> prev = item;
	item -> prev = lst -> prev;
	item -> next = lst;
}

void del_queue(struct list *lst){
	lst -> prev -> next = lst -> next;
	lst -> next -> prev = lst -> prev;
}
