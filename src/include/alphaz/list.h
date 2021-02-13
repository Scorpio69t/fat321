#ifndef _ALPHAZ_LIST_H_
#define _ALPHAZ_LIST_H_

#include <alphaz/type.h>


/*
 * 通用双向链表
 */
struct list_head
{
    struct list_head *prev, *next;
};


#define container_of(ptr, type, member) ({			\
	const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
	(type *)( (char *)__mptr - offsetof(type,member) );})


#define offsetof(type, member) ((size_t) &((type *)0)->member)


/**
 * list_entry - 获取宿主地址
 * @ptr:     成员变量member的地址
 * @type:    包含成员变量member的宿主结构体类型
 * @member:  宿主结构体中member成员变量的名称
 */
#define list_entry(ptr, type, member)                   \
    container_of(ptr, type, member)


/**
 * list_first_entry - 获取链表中第一个元素（宿主）
 * @ptr:     链表头结点
 * @type:    包含成员变量member的宿主结构体类型
 * @member:  宿主结构体中member成员变量的名称
 */
#define list_first_entry(ptr, type, member) \
    list_entry((ptr)->next, type, member)


/**
 * list_next_entry - 获取链表中下一个元素（宿主）
 * @pos:    当前位置，宿主类型的指针
 * @member: 宿主结构体中member成员变量的名称
 */
#define list_next_entry(pos, member) \
    list_entry((pos)->member.next, typeof(*(pos)), member)


/**
 * 初始化链表头，将链表头的prve和next指针都指向自己
 */
static inline void list_head_init(struct list_head *head)
{
    head->prev = head->next = head;
}


/**
 * list_is_null - 判断当前链表是否为空
 * @head: 链表的头结点
 */
static inline int list_is_null(struct list_head *head)
{
    return (head->next == head || head->prev == head);
}


/**
 * 在prev和next节点之间插入节点
 */
static inline void __list_add(struct list_head *new, struct list_head *prev,
                                 struct list_head *next)
{
    new->next = next;
    new->prev = prev;
    prev->next = new;
    next->prev = new;
}


/**
 * list_add - 在链表头部插入新节点
 * @new:  要被插入的新节点
 * @head: 链表的头结点
 */
static inline void list_add(struct list_head *new, struct list_head *head)
{
    __list_add(new, head, head->next);
}


/**
 * list_add_tail - 在链表尾部插入节点
 * @new:  要被插入的新节点
 * @head: 链表的头结点
 */
static inline void list_add_tail(struct list_head *new, struct list_head *head)
{
    __list_add(new, head->prev, head);
}


/**
 * 删除prev和next之间的节点
 */
static inline void __list_del(struct list_head *prev, struct list_head *next)
{
    prev->next = next;
    next->prev = prev;
}


#define __list_del_entry_valid(entry) (entry->prev != NULL && \
                                        entry->next != NULL)

static inline void __list__del_entry(struct list_head *entry)
{
    if(!__list_del_entry_valid(entry))
        return;
    __list_del(entry->prev, entry->next);
}


/**
 * list_del - 在链表中删除节点
 * @entry: 被删除的节点
 */
static inline void list_del(struct list_head *entry)
{
    __list__del_entry(entry);
    entry->next = NULL;
    entry->prev = NULL;
}


/**
 * list_for_each - 遍历链表
 * @pos:  &struct list_head，用于遍历的游标
 * @head: 链表的头结点
 */
#define list_for_each(pos, head) \
    for (pos = (head)->next; pos != (head); pos = pos->next)

/**
 * list_for_each_safe - 遍历链表，可在遍历时删除链表中的元素
 * @pos: 用于遍历的游标
 * @n: 用于临时保存游标
 * @head: 链表的头结点
 */
#define list_for_each_safe(pos, n, head) \
	for (pos = (head)->next, n = pos->next; pos != (head); \
		pos = n, n = pos->next)

/**
 * list_for_each_prev - 反向遍历链表
 * @pos: &struct list_head，用于遍历的游标
 * @head: 链表的头结点
 */
#define list_for_each_prev(pos, head) \
    for (pos = (head)->prev; pos != (head); pos = pos->prev)


/**
 * list_for_each_entry - 对宿主结构进行遍历
 * @pos:    宿主结构体类型指针，作为遍历游标
 * @head:   链表的头结点
 * @member: 结构体内链表成员变量的名称
 */
#define list_for_each_entry(pos, head, member)             \
    for (pos = list_first_entry(head, typeof(*pos), member); \
         &pos->member != (head);                               \
         pos = list_next_entry(pos, member))

#endif
