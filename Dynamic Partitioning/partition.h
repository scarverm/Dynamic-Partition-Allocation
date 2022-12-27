#pragma once
#include "mode.h"

typedef struct Partition Partition;

void ShowParts(Partition* head);

struct Partition	//���з���
{
	int startPosi;		//��ʼ��ַ
	int size;			//��С
	Partition* next;	//��һ���
	Partition* last;	//��һ���
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

//��������
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

//�����ж�
int Check(double prob) {
	double d = (double)rand() / RAND_MAX;
	if (d < prob) return 1;
	else return 0;
}

/******************************************************
	������������
	��ʼ������������Ҫģ��һ����ҵ�����ڴ�Ĺ��̣���˴���work����
******************************************************/
int* GenerateReq(int reqLen, int jobNum, int* work) {
	int* request = (int*)malloc(reqLen * sizeof(int));
	for (int i = 0; i < reqLen; i++) {
		int job = rand() % jobNum;
		while (1) {
			int check = Check(0.65);
			if (check && work[job] == -1) break;	//40%�ĸ��ʻ����ڴ棬60%�ĸ��ʷ����ڴ�
			else if (!check && work[job] == 0) break;
			job = rand() % jobNum;
		}	//�������ҵ�Ѿ�ִ����ϣ�������ѡ����ҵ
		work[job]--;	//�ı���ҵ״̬
		request[i] = job;
	}
	return request;
}

/******************************************************
	��ʼ��ÿ����ҵ��Ҫռ�õ��ڴ�
	jobNum: ��ҵ��������
	maxMemory��minMemory: ÿ����ҵ����ռ�õ������С�ڴ�
*******************************************************/
int* InitMemory(int jobNum, int minMemory, int maxMemory) {
	int* memory = (int*)malloc(jobNum * sizeof(int));
	for (int i = 0; i < jobNum; i++) {
		memory[i] = rand() % (maxMemory - minMemory) + minMemory;
	}
	return memory;
}

/******************************************************
	�״���Ӧ�㷨�������׿�ʼ˳����ң�ֱ���ҵ�һ����С����Ҫ��ķ���
	Ȼ������ҵ�Ĵ�С���Ӹ÷����л���һ���ڴ�ռ����������ߣ����µĿ��пռ������ڿ�������
	head: ���з�������ͷ���
	memSize: ��Ҫ������ڴ��С
	�������ɹ���������ҵ���׵�ַ�����򷵻�-1
******************************************************/
int FF(Partition* head, int memSize) {
	Partition* p = head->next;
	while (p != NULL && p->size < memSize) {
		p = p->next;
	}
	int startPos;
	if (p == NULL) return -1;	//p == NULL��ʾ�Ѿ����ҵ���β
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
	����ַ������������ڴ����
	head: ���з���������
	start: ��ʼ��Ѱ�Ľ�㣬������״���Ӧ�㷨������ȡNULL�������ֵ
	memSize: ��Ҫ�ͷŵ��ڴ��С
	startPos: ��Ҫ�ͷ��ڴ����ʼ��ַ
	mode: �㷨���
******************************************************/
void OrderRecycle(Partition* head, Partition** start, int startPos, int memSize, enum Mode mode) {
	Partition* p = head->next;
	Partition* last = head;	//��Ҫ�ж�ָ��p�Ƿ�ΪNULL����p=NULLʱ���ܵ���p->last������Ҫʹ��һ��lastָ��
	while (p != NULL && p->startPosi < startPos) {
		//��p��last�ƶ���startPos������
		last = p;
		p = p->next;
	}
	if (last == head && p == NULL) {
		//��������ǿյ�
		Partition* node = InitNode(startPos, memSize, NULL, last);
		if (mode == NEXT_FIT) (*start) = node;
	}
	else if (p != NULL && last == head && startPos + memSize == p->startPosi) {
		//�����ף���Ҫ�ж��Ƿ�Ҫ��head->next���ϲ�
		p->startPosi = startPos;
		p->size += memSize;
	}
	else if (p == NULL && last->startPosi + last->size == startPos) {
		//����β����Ҫ�ж��Ƿ�Ҫ��β���ϲ�
		last->size += memSize;
	}
	else if (p != NULL && last->startPosi + last->size == startPos && startPos + memSize == p->startPosi) {
		//�����ߺϲ�
		int flag = 0;
		if (*start == p) flag = 1;
		last->size += memSize + p->size;
		last->next = p->next;
		if (p->next != NULL) p->next->last = last;
		free(p);
		if (mode == NEXT_FIT && flag) *start = last->next == NULL ? head->next : last->next;
	}
	else if (last->startPosi + last->size == startPos) {
		//��ǰ�ϲ�
		last->size += memSize;
	}
	else if (p != NULL && startPos + memSize == p->startPosi) {
		//���ϲ�
		p->startPosi = startPos;
		p->size += memSize;
	}
	else {
		//���ܺϲ�
		Partition* node = InitNode(startPos, memSize, p, last);
	}
}

/******************************************************
	ѭ���״���Ӧ�㷨�����ϴ��ҵ��Ŀ��з�������һ���з�����ʼ����
	head: ���з���������
	start: ��ʼ��Ѱָ��
	memSize: ��Ҫ������ڴ��С
	������ҵ���׵�ַ��������ʧ�ܣ��򷵻�-1
******************************************************/
int NF(Partition* head, Partition** start, int memSize) {
	//if (*start != head && *start != NULL) printf("����ǰstart: %d %d\n", (*start)->startPosi, (*start)->size);
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
	Ϊ���ܸ��õ���ʾ���з����ķֲ������BF��WF�㷨ͬʱҲά����һ�Ű���ַ��С��������Ŀ��з�������
	sequence: ���з���������
	startPos: �����׵�ַ
	memSize: �����ռ���ڴ��С
******************************************************/
void BWFSequence(Partition* sequence, int startPos, int memSize) {
	Partition* p = sequence->next;
	while (p != NULL) {
		//�����ڴ�����
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
		//�����ڴ�����
		else if (p != sequence && p->next != NULL && p->startPosi == startPos && p->next->startPosi + p->next->size == startPos + memSize) {
			//�ڴ����ʱ�����ߺϲ�
			p->size = memSize;
			p = p->next;
			p->last->next = p->next;
			if (p->next != NULL) p->next->last = p->last;
			free(p);
			return;
		}
		else if (p->startPosi == startPos) {
			//�ڴ����ʱ��ǰ�ϲ�
			p->size = memSize;
			return;
		}
		else if (startPos + memSize == p->startPosi + p->size) {
			//�ڴ����ʱ���ϲ�
			p->startPosi = startPos;
			p->size = memSize;
			return;
		}
		else if (startPos + memSize < p->startPosi) {
			//�ڴ����ʱδ�ϲ�
			Partition* node = InitNode(startPos, memSize, p, p->last);
			return;
		}
		p = p->next;
	}
	if (p == NULL && sequence->next == NULL) {
		InitNode(startPos, memSize, NULL, sequence);
	}
	else if (p == NULL) {	//�ڿ��з�����ĩβ�����ڴ�
		Partition* last = sequence->next;
		while (last->next != NULL) last = last->next;
		p = InitNode(startPos, memSize, NULL, last);
	}
}

/******************************************************
	�����Ӧ�㷨�������Ӧ�㷨
	�����Ӧ�㷨�����п��з������������Դ�С�����˳���ų�һ�����з�����
	���Ӧ�㷨�����еĿ��з����������ԴӴ�С��˳���ų�һ�����з�����
	head: ���з���������
	memSize: ��Ҫ������ڴ��С
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
		//���Ҫ����Ŀ��з���������������Ҫ������ڴ��С����ô�ڷ����ڴ����Ҫ���µ����ÿ��з����ڷ������ϵ�λ��
		startIndex = p->startPosi;
		p->size -= memSize;
		p->startPosi += memSize;
		BWFSequence(sequence, p->startPosi, p->size);
		p->last->next = p->next;	//��p�������
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
	�����Ӧ�㷨�����Ӧ�㷨���ڴ����
	���ߵĲ�֮ͬ�������ڵ���ĳһ���з���λ�õ��ж�����
******************************************************/
void UnorderedRecycle(Partition* head, int startPos, int memSize, enum Mode mode, Partition* sequence) {
	Partition* forward = head->next;	//��ǰ�ϲ��Ľ��
	Partition* backward = head->next;	//���ϲ��Ľ��
	while (forward != NULL && backward != NULL) {
		if (forward->startPosi + forward->size != startPos) forward = forward->next;
		if (startPos + memSize != backward->startPosi) backward = backward->next;
		if (forward != NULL && backward != NULL && forward->startPosi + forward->size == startPos && startPos + memSize == backward->startPosi) break;
	}
	//��������Ҫ�����ж�λ�õĽ������Ϊforward��������һ��λ�õ�����
	if (forward != NULL && backward != NULL) {
		//�����ߺϲ�
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
		//��ǰ�ϲ�
		forward->last->next = forward->next;
		if (forward->next != NULL) forward->next->last = forward->last;
		forward->size += memSize;
	}
	else if (backward != NULL) {
		//���ϲ�
		backward->last->next = backward->next;
		if (backward->next != NULL) backward->next->last = backward->last;
		backward->startPosi = startPos;
		backward->size += memSize;
		forward = backward;
	}
	else {
		//���ܺϲ�
		Partition* node = InitNode(startPos, memSize, NULL, NULL);
		forward = node;
	}
	BWFSequence(sequence, forward->startPosi, forward->size);
	Partition* p = head->next;
	Partition* last = head;	//�������Ϊ��, ������Ϊ��ʱ�޷�����p->last����Ҫ��lastָ������ʾp����һ�����
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

//��ʾ���з�������
void ShowParts(Partition* head) {
	Partition* p = head->next;
	int partNum = 0;
	printf("=====================================\n");
	printf(" ������      ��ʼ��ַ      ������С  \n");
	while (p != NULL) {
		printf("   %d\t\t%d\t     %d\n", partNum + 1, p->startPosi, p->size);
		p = p->next;
		partNum++;
	}
	printf("=====================================\n\n");
}