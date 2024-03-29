#pragma once
#include "mode.h"

typedef struct Partition Partition;

void ShowParts(Partition* head);

struct Partition	//空闲分区
{
	int startPosi;		//起始地址
	int size;			//大小
	Partition* next;	//下一结点
	Partition* last;	//上一结点
};

Partition* InitNode(int startPos, int memSize, Partition* next, Partition* last) {
	Partition* newNode = (Partition*)malloc(sizeof(Partition));
	newNode->startPosi = startPos;
	newNode->size = memSize;
	newNode->next = next;
	if (next != NULL) next->last = newNode;
	newNode->last = last;
	if (last != NULL) last->next = newNode;
	return newNode;
}

Partition* CreateHead(int size) {
	Partition* part = (Partition*)malloc(sizeof(Partition));
	part->next = NULL;
	part->size = size;
	part->startPosi = 0;
	Partition* head = (Partition*)malloc(sizeof(Partition));
	head->next = part;
	head->last = NULL;
	part->last = head;
	return head;
}

//销毁链表
void DestroyPart(Partition* head) {
	Partition* p = head;
	Partition* q = head->next;
	while (q != NULL) {
		free(p);
		p = q;
		q = q->next;
	}
	free(p);
}

//概率判断
int Check(double prob) {
	double d = (double)rand() / RAND_MAX;
	if (d < prob) return 1;
	else return 0;
}

/******************************************************
	生成请求序列
	初始化请求序列需要模拟一遍作业请求内存的过程，因此传入work序列
******************************************************/
int* GenerateReq(int reqLen, int jobNum, int* work) {
	int* request = (int*)malloc(reqLen * sizeof(int));
	for (int i = 0; i < reqLen; i++) {
		int job = rand() % jobNum;
		while (1) {
			int check = Check(0.65);
			if (check && work[job] == -1) break;	//40%的概率回收内存，60%的概率分配内存
			else if (!check && work[job] == 0) break;
			job = rand() % jobNum;
		}	//如果该作业已经执行完毕，就重新选择作业
		work[job]--;	//改变作业状态
		request[i] = job;
	}
	return request;
}

/******************************************************
	初始化每个作业需要占用的内存
	jobNum: 作业的数量；
	maxMemory、minMemory: 每个作业允许占用的最大、最小内存
*******************************************************/
int* InitMemory(int jobNum, int minMemory, int maxMemory) {
	int* memory = (int*)malloc(jobNum * sizeof(int));
	for (int i = 0; i < jobNum; i++) {
		memory[i] = rand() % (maxMemory - minMemory) + minMemory;
	}
	return memory;
}

/******************************************************
	首次适应算法：从链首开始顺序查找，直到找到一个大小满足要求的分区
	然后按照作业的大小，从该分区中划出一块内存空间分配给请求者，余下的空闲空间仍留在空闲链中
	head: 空闲分区链的头结点
	memSize: 需要分配的内存大小
	若分区成功，返回作业的首地址，否则返回-1
******************************************************/
int FF(Partition* head, int memSize) {
	Partition* p = head->next;
	while (p != NULL && p->size < memSize) {
		p = p->next;
	}
	int startPos;
	if (p == NULL) return -1;	//p == NULL表示已经查找到链尾
	else if (p->size == memSize) {
		Partition* last = p->last;
		startPos = p->startPosi;
		last->next = p->next;
		if (p->next != NULL) p->next->last = last;
		free(p);
	}
	else {
		startPos = p->startPosi;
		p->size -= memSize;
		p->startPosi += memSize;
	}
	return startPos;
}

/******************************************************
	按地址递增的链表的内存回收
	head: 空闲分区链链首
	start: 开始查寻的结点，如果是首次适应算法，可以取NULL外的任意值
	memSize: 需要释放的内存大小
	startPos: 需要释放内存的起始地址
	mode: 算法类别
******************************************************/
void OrderRecycle(Partition* head, Partition** start, int startPos, int memSize, enum Mode mode) {
	Partition* p = head->next;
	Partition* last = head;	//需要判断指针p是否为NULL，当p=NULL时不能调用p->last，所以要使用一个last指针
	while (p != NULL && p->startPosi < startPos) {
		//将p和last移动到startPos的两边
		last = p;
		p = p->next;
	}
	if (last == head && p == NULL) {
		//如果链表是空的
		Partition* node = InitNode(startPos, memSize, NULL, last);
		if (mode == NEXT_FIT) (*start) = node;
	}
	else if (p != NULL && last == head && startPos + memSize == p->startPosi) {
		//在链首，需要判断是否要与head->next结点合并
		p->startPosi = startPos;
		p->size += memSize;
	}
	else if (p == NULL && last->startPosi + last->size == startPos) {
		//在链尾，需要判断是否要与尾结点合并
		last->size += memSize;
	}
	else if (p != NULL && last->startPosi + last->size == startPos && startPos + memSize == p->startPosi) {
		//向两边合并
		int flag = 0;
		if (*start == p) flag = 1;
		last->size += memSize + p->size;
		last->next = p->next;
		if (p->next != NULL) p->next->last = last;
		free(p);
		if (mode == NEXT_FIT && flag) *start = last->next == NULL ? head->next : last->next;
	}
	else if (last->startPosi + last->size == startPos) {
		//向前合并
		last->size += memSize;
	}
	else if (p != NULL && startPos + memSize == p->startPosi) {
		//向后合并
		p->startPosi = startPos;
		p->size += memSize;
	}
	else {
		//不能合并
		Partition* node = InitNode(startPos, memSize, p, last);
	}
}

/******************************************************
	循环首次适应算法：从上次找到的空闲分区的下一空闲分区开始查找
	head: 空闲分区链链首
	start: 起始查寻指针
	memSize: 需要分配的内存大小
	返回作业的首地址，若分区失败，则返回-1
******************************************************/
int NF(Partition* head, Partition** start, int memSize) {
	//if (*start != head && *start != NULL) printf("运行前start: %d %d\n", (*start)->startPosi, (*start)->size);
	if (head->next == NULL) return -1;
	Partition* sstart = (*start);
	while ((*start)->size < memSize) {
		*start = (*start)->next == NULL ? head->next : (*start)->next;
		if (*start == sstart) return -1;
	}
	int startIndex = (*start)->startPosi;
	if ((*start)->size > memSize) {
		(*start)->size -= memSize;
		(*start)->startPosi += memSize;
		*start = (*start)->next == NULL ? head->next : (*start)->next;
	}
	else {
		Partition* last = (*start)->last;
		(*start)->last->next = (*start)->next;
		if ((*start)->next != NULL) (*start)->next->last = (*start)->last;
		free(*start);
		*start = last->next == NULL ? head->next : last->next;
	}
	return startIndex;
}

/******************************************************
	为了能更好地显示空闲分区的分布情况，BF和WF算法同时也维护着一张按地址从小到大排序的空闲分区链表
	sequence: 空闲分区链链首
	startPos: 结点的首地址
	memSize: 结点所占的内存大小
******************************************************/
void BWFSequence(Partition* sequence, int startPos, int memSize) {
	Partition* p = sequence->next;
	while (p != NULL) {
		//分配内存的情况
		if (startPos + memSize == p->startPosi + p->size) {
			if (memSize == 0) {
				p->last->next = p->next;
				if (p->next != NULL) p->next->last = p->last;
				free(p);
			}
			else {
				p->startPosi = startPos;
				p->size = memSize;
			}
			return;
		}
		//回收内存的情况
		else if (p != sequence && p->next != NULL && p->startPosi == startPos && p->next->startPosi + p->next->size == startPos + memSize) {
			//内存回收时向两边合并
			p->size = memSize;
			p = p->next;
			p->last->next = p->next;
			if (p->next != NULL) p->next->last = p->last;
			free(p);
			return;
		}
		else if (p->startPosi == startPos) {
			//内存回收时向前合并
			p->size = memSize;
			return;
		}
		else if (startPos + memSize == p->startPosi + p->size) {
			//内存回收时向后合并
			p->startPosi = startPos;
			p->size = memSize;
			return;
		}
		else if (startPos + memSize < p->startPosi) {
			//内存回收时未合并
			Partition* node = InitNode(startPos, memSize, p, p->last);
			return;
		}
		p = p->next;
	}
	if (p == NULL && sequence->next == NULL) {
		InitNode(startPos, memSize, NULL, sequence);
	}
	else if (p == NULL) {	//在空闲分区链末尾回收内存
		Partition* last = sequence->next;
		while (last->next != NULL) last = last->next;
		p = InitNode(startPos, memSize, NULL, last);
	}
}

/******************************************************
	最佳适应算法和最差适应算法
	最佳适应算法：所有空闲分区按其容量以从小到大的顺序，排成一个空闲分区链
	最坏适应算法：所有的空闲分区按容量以从大到小的顺序，排成一个空闲分区链
	head: 空闲分区链链首
	memSize: 需要分配的内存大小
******************************************************/
int BWF(Partition* head, int memSize, enum Mode mode, Partition* sequence) {
	Partition* p = head->next;
	while (p != NULL && p->size < memSize) {
		p = p->next;
	}
	int startIndex;
	if (p == NULL) return -1;
	else if (p->size == memSize) {
		BWFSequence(sequence, p->startPosi + p->size, 0);
		startIndex = p->startPosi;
		p->last->next = p->next;
		if (p->next != NULL) p->next->last = p->last;
		free(p);
	}
	else {
		//如果要分配的空闲分区的容量大于需要分配的内存大小，那么在分配内存后，需要重新调整该空闲分区在分区链上的位置
		startIndex = p->startPosi;
		p->size -= memSize;
		p->startPosi += memSize;
		BWFSequence(sequence, p->startPosi, p->size);
		p->last->next = p->next;	//将p提出链表
		if (p->next != NULL) p->next->last = p->last;
		Partition* last = head;
		while (last->next != NULL) {
			if (mode == BEST_FIT && last->next->size > p->size) break;
			if (mode == WORST_FIT && last->next->size < p->size) break;
			last = last->next;
		}
		if (last->next == NULL) {
			last->next = p;
			p->next = NULL;
			p->last = last;
		}
		else {
			p->next = last->next;
			if (last->next != NULL) last->next->last = p;
			last->next = p;
			p->last = last;
		}
	}
	return startIndex;
}

/******************************************************
	最佳适应算法和最坏适应算法的内存回收
	两者的不同之处仅在于调整某一空闲分区位置的判断条件
******************************************************/
void UnorderedRecycle(Partition* head, int startPos, int memSize, enum Mode mode, Partition* sequence) {
	Partition* forward = head->next;	//向前合并的结点
	Partition* backward = head->next;	//向后合并的结点
	while (forward != NULL && backward != NULL) {
		if (forward->startPosi + forward->size != startPos) forward = forward->next;
		if (startPos + memSize != backward->startPosi) backward = backward->next;
		if (forward != NULL && backward != NULL && forward->startPosi + forward->size == startPos && startPos + memSize == backward->startPosi) break;
	}
	//将所有需要重新判断位置的结点设置为forward，便于下一步位置的设置
	if (forward != NULL && backward != NULL) {
		//向两边合并
		if (forward == backward->last) {
			forward->last->next = backward->next;
			if (backward->next != NULL) backward->next->last = forward->last;
		}
		else if (backward == forward->last) {
			backward->last->next = forward->next;
			if (forward->next != NULL) forward->next->last = backward->last;
		}
		else {
			backward->last->next = backward->next;
			if (backward->next != NULL) backward->next->last = backward->last;
			forward->last->next = forward->next;
			if (forward->next != NULL) forward->next->last = forward->last;
		}
		forward->size = forward->size + memSize + backward->size;
		free(backward);
	}
	else if (forward != NULL) {
		//向前合并
		forward->last->next = forward->next;
		if (forward->next != NULL) forward->next->last = forward->last;
		forward->size += memSize;
	}
	else if (backward != NULL) {
		//向后合并
		backward->last->next = backward->next;
		if (backward->next != NULL) backward->next->last = backward->last;
		backward->startPosi = startPos;
		backward->size += memSize;
		forward = backward;
	}
	else {
		//不能合并
		Partition* node = InitNode(startPos, memSize, NULL, NULL);
		forward = node;
	}
	BWFSequence(sequence, forward->startPosi, forward->size);
	Partition* p = head->next;
	Partition* last = head;	//链表可能为空, 当链表为空时无法调用p->last，需要用last指针来表示p的上一个结点
	while (p != NULL){
		if (mode == BEST_FIT && p->size > forward->size || mode == WORST_FIT && p->size < forward->size) break;
		last = p;
		p = p->next;
	}
	last->next = forward;
	forward->next = p;
	forward->last = last;
	if (p != NULL) p->last = forward;
}

//显示空闲分区链表
void ShowParts(Partition* head) {
	Partition* p = head->next;
	int partNum = 0;
	printf("=====================================\n");
	printf(" 分区号      起始地址      分区大小  \n");
	while (p != NULL) {
		printf("   %d\t\t%d\t     %d\n", partNum + 1, p->startPosi, p->size);
		p = p->next;
		partNum++;
	}
	printf("=====================================\n\n");
}