#ifndef __WORK_QUE_H__			   //中间键宏的常用写法
#define __WORK_QUE_H__
#include "head.h"

//用链表来实现队列
typedef struct tag_node			   	      //链表结点的结构体
{
	int new_fd;
	struct tag_node* pNext;
}node_t, *pnode_t;

typedef struct					          //队列的结构体
{
	pnode_t que_head,que_tail;	          //队列的头、尾指针
	int que_capacity;              		  //队列能力（最大负荷）
	int size;    				   		  //记录当前队列的实时的大小
	pthread_mutex_t que_mutex;
}que_t, *pque_t;

void que_insert(pque_t,pnode_t);   		  //尾插法
void que_get(pque_t,pnode_t*);
void que_insert_exit(pque_t,pnode_t);   //头插法标志位（用于退出机制）
void judge_thread_exit(pque_t);
#endif

