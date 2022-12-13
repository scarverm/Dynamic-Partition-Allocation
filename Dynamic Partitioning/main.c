#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <time.h>
#include <stdlib.h>
#include "partition.h"
#include "mode.h"

int main() {

	//设置内存大小
	int size;
	printf("设置内存大小:\n");
	scanf("%d", &size);
	while (size <= 0) {
		printf("内存大小设置错误，请重新设置:\n");
		scanf("%d", &size);
	}

	//设置最大作业数
	int maxJobNum;
	printf("设置最大作业数:\n");
	scanf("%d", &maxJobNum);
	while (maxJobNum <= 0) {
		printf("最大作业数设置错误，请重新设置:\n");
		scanf("%d", &maxJobNum);
	}

	//设置请求序列数量
	int reqNum;
	printf("设置请求序列数量:\n");
	scanf("%d", &reqNum);
	while (reqNum <= 0) {
		printf("请求序列设置错误，请重新设置:\n");
		scanf("%d", &reqNum);
	}
	
	//设置分配算法
	char c;
	enum Mode mode;
	printf("设置分配算法(首次适应算法: F, 循环首次适应算法: N, 最佳适应算法: B, 最差适应算法: W, 默认运行所有算法):\n");
	getchar();	//过滤前面输入reqNum值后按下的回车
	scanf("%c", &c);
	while (1) {
		if (c == 'F' || c == 'f') { getchar(); mode = FIRST_FIT; break; }
		else if (c == 'N' || c == 'n') { getchar(); mode = NEXT_FIT; break; }
		else if (c == 'B' || c == 'b') { getchar(); mode = BEST_FIT; break; }
		else if (c == 'W' || c == 'w') { getchar(); mode = WORST_FIT; break; }
		else if (c == '\n') { mode = ALL_RUN; break; }
		else {
			printf("分配算法设置错误，请重新设置:\n");
			getchar();		//过滤回车
			scanf("%c", &c);
		}
	}

	//是否隐藏细节
	char hideDetails = 'n';
	printf("是否隐藏细节(y/n):\n");
	scanf("%c", &hideDetails);
	while (1) {
		if (hideDetails == 'y' || hideDetails == 'Y') { hideDetails = 'y'; break; }
		else if (hideDetails == 'n' || hideDetails == 'N') { hideDetails = 'n'; break; }
		else if (hideDetails == '\n') { hideDetails = 'n'; break; }
		else {
			printf("错误的参数，请重新设置:\n");
			getchar();
			scanf("%c", &hideDetails);
		}
	}
	
	//获取随机数种子
	srand((unsigned)time(NULL));

	int FFcnt = 0, NFcnt = 0, BFcnt = 0, WFcnt = 0;

	for (int i = 0; i < reqNum; i++) {
		int job = rand() % (maxJobNum - maxJobNum / 2) + maxJobNum / 2;			//随机生成作业数量
		int reqLen = 2 * job;	//随机生成请求序列长度，默认一个作业最多只申请一次内存

		//作业状态序列，work[i]=0表示作业i没申请内存，work[i]=-1表示作业i已经申请了内存，work[i]=-2表示作业i申请的内存已被回收
		//这样只要执行work[i]--即可改变作业i的状态
		int* work = (int*)malloc(job * sizeof(int));
		memset(work, 0, job * sizeof(int));	//初始化为0

		//作业请求序列，queue[i]=作业编号，表示第i时刻请求内存的作业或被释放内存的作业
		int* request = GenerateReq(reqLen, job, work);
		printf("\n作业请求序列%d: ", i);
		/*for (int j = 0; j < reqLen; j++) {
			printf("作业%d ", request[j]);
		}*/
		printf("\n");
		
		
		//内存申请序列，用于储存作业申请的内存大小，
		int* memory = InitMemory(job, 10, 200);

		//作业被分配空间的起始地址，作为回收内存时的标记
		int* startPos = (int*)malloc(job * sizeof(int));
		memset(startPos, -1, job * sizeof(int));

		//开始分区
		//首次适应算法
		if (mode == ALL_RUN || mode == FIRST_FIT) {
			printf("首次适应算法FF: \n");
			memset(work, 0, job * sizeof(int));
			memset(startPos, -1, job * sizeof(int));
			Partition* head = CreateHead(size);
			if (hideDetails == 'n') {
				printf("初始分区链表:\n");
				ShowParts(head);
			}
			for (int j = 0; j < reqLen; j++) {
				if (work[request[j]] == 0) {
					//分配内存
					if (hideDetails == 'n') printf("作业%d请求分配内存:%d\n", request[j], memory[request[j]]);
					int index = FF(head, memory[request[j]]);
					if (index == -1) {
						printf("分区失败!\n");
						FFcnt++;
						break;
					}
					else {
						startPos[request[j]] = index;
					}
				}
				else if (work[request[j]] == -1) {
					//回收内存
					if (hideDetails == 'n')
						printf("回收作业%d的内存:  起始地址:%d\t内存大小:%d\n", request[j], startPos[request[j]], memory[request[j]]);
					OrderRecycle(head, head, startPos[request[j]], memory[request[j]], FIRST_FIT);
				}
				work[request[j]]--;
				if (hideDetails == 'n') ShowParts(head);
				if (j == reqLen - 1) printf("分区成功!\n");
			}
			DestroyPart(head);
		}

		//循环首次适应算法
		if (mode == ALL_RUN || mode == NEXT_FIT) {
			printf("循环首次适应算法NF: \n");
			memset(work, 0, job * sizeof(int));
			memset(startPos, -1, job * sizeof(int));
			Partition* head = CreateHead(size);
			Partition* start = head->next;
			if (hideDetails == 'n') {
				printf("初始分区链表:\n");
				ShowParts(head);
			}
			for (int j = 0; j < reqLen; j++) {
				if (work[request[j]] == 0) {
					//分配内存
					if (hideDetails == 'n') printf("作业%d请求分配内存:%d\n", request[j], memory[request[j]]);
					int index = NF(head, &start, memory[request[j]]);
					if (index == -1) {
						printf("分区失败!\n");
						NFcnt++;
						break;
					}
					else {
						startPos[request[j]] = index;
					}
				}
				else if (work[request[j]] == -1) {
					//回收内存
					if (hideDetails == 'n')
						printf("回收作业%d的内存:  起始地址:%d\t内存大小:%d\n", request[j], startPos[request[j]], memory[request[j]]);
					OrderRecycle(head, &start, startPos[request[j]], memory[request[j]], NEXT_FIT);
				}
				work[request[j]]--;
				if (hideDetails == 'n') ShowParts(head);
				if (j == reqLen - 1) printf("分区成功!\n");
			}
			DestroyPart(head);
		}

		//最佳适应算法
		if (mode == ALL_RUN || mode == BEST_FIT) {
			printf("最佳适应算法BF: \n");
			memset(work, 0, job * sizeof(int));
			memset(startPos, -1, job * sizeof(int));
			Partition* head = CreateHead(size);
			Partition* sequence = CreateHead(size);
			if (hideDetails == 'n') {
				printf("初始分区链表:\n");
				ShowParts(head);
			}
			for (int j = 0; j < reqLen; j++) {
				if (work[request[j]] == 0) {
					//分配内存
					if (hideDetails == 'n') printf("作业%d请求分配内存:%d\n", request[j], memory[request[j]]);
					int index = BWF(head, memory[request[j]], BEST_FIT, sequence);
					if (index == -1) {
						printf("分区失败!\n");
						BFcnt++;
						break;
					}
					else {
						startPos[request[j]] = index;
					}
				}
				else if (work[request[j]] == -1) {
					//回收内存
					if (hideDetails == 'n')
						printf("回收作业%d的内存:  起始地址:%d\t内存大小:%d\n", request[j], startPos[request[j]], memory[request[j]]);
					UnorderedRecycle(head, startPos[request[j]], memory[request[j]], BEST_FIT, sequence);
				}
				work[request[j]]--;
				if (hideDetails == 'n') ShowParts(sequence);
				if (j == reqLen - 1) printf("分区成功!\n");
			}
			DestroyPart(head);
			DestroyPart(sequence);
		}

		//最差适应算法
		if (mode == ALL_RUN || mode == WORST_FIT) {
			printf("最佳适应算法BF: \n");
			memset(work, 0, job * sizeof(int));
			memset(startPos, -1, job * sizeof(int));
			Partition* head = CreateHead(size);
			Partition* sequence = CreateHead(size);
			if (hideDetails == 'n') {
				printf("初始分区链表:\n");
				ShowParts(head);
			}
			for (int j = 0; j < reqLen; j++) {
				if (work[request[j]] == 0) {
					//分配内存
					if (hideDetails == 'n') printf("作业%d请求分配内存:%d\n", request[j], memory[request[j]]);
					int index = BWF(head, memory[request[j]], WORST_FIT, sequence);
					if (index == -1) {
						printf("分区失败!\n");
						WFcnt++;
						break;
					}
					else {
						startPos[request[j]] = index;
					}
				}
				else if (work[request[j]] == -1) {
					//回收内存
					if (hideDetails == 'n')
						printf("回收作业%d的内存:  起始地址:%d\t内存大小:%d\n", request[j], startPos[request[j]], memory[request[j]]);
					UnorderedRecycle(head, startPos[request[j]], memory[request[j]], WORST_FIT, sequence);
				}
				work[request[j]]--;
				if (hideDetails == 'n') ShowParts(sequence);
				if (j == reqLen - 1) printf("分区成功!\n");
			}
			DestroyPart(head);
			DestroyPart(sequence);
		}

		free(work);
		free(request);
		free(memory);
		free(startPos);
	}

	printf("\n共%d组请求队列，其中\n", reqNum);
	if (mode == FIRST_FIT || mode == ALL_RUN)
		printf("首次适应算法分区失败%d次，失败率：%.2lf%%\n", FFcnt, (double)FFcnt / reqNum * 100);
	if (mode == NEXT_FIT || mode == ALL_RUN)
		printf("循环首次适应算法分区失败%d次，失败率：%.2lf%%\n", NFcnt, (double)NFcnt / reqNum * 100);
	if (mode == BEST_FIT || mode == ALL_RUN)
		printf("最佳适应算法分区失败%d次，失败率：%.2lf%%\n", BFcnt, (double)BFcnt / reqNum * 100);
	if (mode == WORST_FIT || mode == ALL_RUN)
		printf("最差适应算法分区失败%d次，失败率：%.2lf%%\n", WFcnt, (double)WFcnt / reqNum * 100);

	return 0;
}
