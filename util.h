struct list{
	struct list *prev;
	struct list *next;
	char *ptr;
};

void new_queue(struct list *lst);
void add_queue(struct list *lst, struct list *item);
void del_queue(struct list *lst);