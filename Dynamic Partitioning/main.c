#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <time.h>
#include <stdlib.h>
#include "partition.h"
#include "mode.h"

int main() {

	//�����ڴ��С
	int size;
	printf("�����ڴ��С:\n");
	scanf("%d", &size);
	while (size <= 0) {
		printf("�ڴ��С���ô�������������:\n");
		scanf("%d", &size);
	}

	//���������ҵ��
	int maxJobNum;
	printf("���������ҵ��:\n");
	scanf("%d", &maxJobNum);
	while (maxJobNum <= 0) {
		printf("�����ҵ�����ô�������������:\n");
		scanf("%d", &maxJobNum);
	}

	//����������������
	int reqNum;
	printf("����������������:\n");
	scanf("%d", &reqNum);
	while (reqNum <= 0) {
		printf("�����������ô�������������:\n");
		scanf("%d", &reqNum);
	}
	
	//���÷����㷨
	char c;
	enum Mode mode;
	printf("���÷����㷨(�״���Ӧ�㷨: F, ѭ���״���Ӧ�㷨: N, �����Ӧ�㷨: B, �����Ӧ�㷨: W, Ĭ�����������㷨):\n");
	getchar();	//����ǰ������reqNumֵ���µĻس�
	scanf("%c", &c);
	while (1) {
		if (c == 'F' || c == 'f') { getchar(); mode = FIRST_FIT; break; }
		else if (c == 'N' || c == 'n') { getchar(); mode = NEXT_FIT; break; }
		else if (c == 'B' || c == 'b') { getchar(); mode = BEST_FIT; break; }
		else if (c == 'W' || c == 'w') { getchar(); mode = WORST_FIT; break; }
		else if (c == '\n') { mode = ALL_RUN; break; }
		else {
			printf("�����㷨���ô�������������:\n");
			getchar();		//���˻س�
			scanf("%c", &c);
		}
	}

	//�Ƿ�����ϸ��
	char hideDetails = 'n';
	printf("�Ƿ�����ϸ��(y/n):\n");
	scanf("%c", &hideDetails);
	while (1) {
		if (hideDetails == 'y' || hideDetails == 'Y') { hideDetails = 'y'; break; }
		else if (hideDetails == 'n' || hideDetails == 'N') { hideDetails = 'n'; break; }
		else if (hideDetails == '\n') { hideDetails = 'n'; break; }
		else {
			printf("����Ĳ���������������:\n");
			getchar();
			scanf("%c", &hideDetails);
		}
	}
	
	//��ȡ���������
	srand((unsigned)time(NULL));

	int FFcnt = 0, NFcnt = 0, BFcnt = 0, WFcnt = 0;

	for (int i = 0; i < reqNum; i++) {
		int job = rand() % (maxJobNum - maxJobNum / 2) + maxJobNum / 2;			//���������ҵ����
		int reqLen = 2 * job;	//��������������г��ȣ�Ĭ��һ����ҵ���ֻ����һ���ڴ�

		//��ҵ״̬���У�work[i]=0��ʾ��ҵiû�����ڴ棬work[i]=-1��ʾ��ҵi�Ѿ��������ڴ棬work[i]=-2��ʾ��ҵi������ڴ��ѱ�����
		//����ֻҪִ��work[i]--���ɸı���ҵi��״̬
		int* work = (int*)malloc(job * sizeof(int));
		memset(work, 0, job * sizeof(int));	//��ʼ��Ϊ0

		//��ҵ�������У�queue[i]=��ҵ��ţ���ʾ��iʱ�������ڴ����ҵ���ͷ��ڴ����ҵ
		int* request = GenerateReq(reqLen, job, work);
		printf("\n��ҵ��������%d: ", i);
		/*for (int j = 0; j < reqLen; j++) {
			printf("��ҵ%d ", request[j]);
		}*/
		printf("\n");
		
		
		//�ڴ��������У����ڴ�����ҵ������ڴ��С��
		int* memory = InitMemory(job, 10, 200);

		//��ҵ������ռ����ʼ��ַ����Ϊ�����ڴ�ʱ�ı��
		int* startPos = (int*)malloc(job * sizeof(int));
		memset(startPos, -1, job * sizeof(int));

		//��ʼ����
		//�״���Ӧ�㷨
		if (mode == ALL_RUN || mode == FIRST_FIT) {
			printf("�״���Ӧ�㷨FF: \n");
			memset(work, 0, job * sizeof(int));
			memset(startPos, -1, job * sizeof(int));
			Partition* head = CreateHead(size);
			if (hideDetails == 'n') {
				printf("��ʼ��������:\n");
				ShowParts(head);
			}
			for (int j = 0; j < reqLen; j++) {
				if (work[request[j]] == 0) {
					//�����ڴ�
					if (hideDetails == 'n') printf("��ҵ%d��������ڴ�:%d\n", request[j], memory[request[j]]);
					int index = FF(head, memory[request[j]]);
					if (index == -1) {
						printf("����ʧ��!\n");
						FFcnt++;
						break;
					}
					else {
						startPos[request[j]] = index;
					}
				}
				else if (work[request[j]] == -1) {
					//�����ڴ�
					if (hideDetails == 'n')
						printf("������ҵ%d���ڴ�:  ��ʼ��ַ:%d\t�ڴ��С:%d\n", request[j], startPos[request[j]], memory[request[j]]);
					OrderRecycle(head, head, startPos[request[j]], memory[request[j]], FIRST_FIT);
				}
				work[request[j]]--;
				if (hideDetails == 'n') ShowParts(head);
				if (j == reqLen - 1) printf("�����ɹ�!\n");
			}
			DestroyPart(head);
		}

		//ѭ���״���Ӧ�㷨
		if (mode == ALL_RUN || mode == NEXT_FIT) {
			printf("ѭ���״���Ӧ�㷨NF: \n");
			memset(work, 0, job * sizeof(int));
			memset(startPos, -1, job * sizeof(int));
			Partition* head = CreateHead(size);
			Partition* start = head->next;
			if (hideDetails == 'n') {
				printf("��ʼ��������:\n");
				ShowParts(head);
			}
			for (int j = 0; j < reqLen; j++) {
				if (work[request[j]] == 0) {
					//�����ڴ�
					if (hideDetails == 'n') printf("��ҵ%d��������ڴ�:%d\n", request[j], memory[request[j]]);
					int index = NF(head, &start, memory[request[j]]);
					if (index == -1) {
						printf("����ʧ��!\n");
						NFcnt++;
						break;
					}
					else {
						startPos[request[j]] = index;
					}
				}
				else if (work[request[j]] == -1) {
					//�����ڴ�
					if (hideDetails == 'n')
						printf("������ҵ%d���ڴ�:  ��ʼ��ַ:%d\t�ڴ��С:%d\n", request[j], startPos[request[j]], memory[request[j]]);
					OrderRecycle(head, &start, startPos[request[j]], memory[request[j]], NEXT_FIT);
				}
				work[request[j]]--;
				if (hideDetails == 'n') ShowParts(head);
				if (j == reqLen - 1) printf("�����ɹ�!\n");
			}
			DestroyPart(head);
		}

		//�����Ӧ�㷨
		if (mode == ALL_RUN || mode == BEST_FIT) {
			printf("�����Ӧ�㷨BF: \n");
			memset(work, 0, job * sizeof(int));
			memset(startPos, -1, job * sizeof(int));
			Partition* head = CreateHead(size);
			Partition* sequence = CreateHead(size);
			if (hideDetails == 'n') {
				printf("��ʼ��������:\n");
				ShowParts(head);
			}
			for (int j = 0; j < reqLen; j++) {
				if (work[request[j]] == 0) {
					//�����ڴ�
					if (hideDetails == 'n') printf("��ҵ%d��������ڴ�:%d\n", request[j], memory[request[j]]);
					int index = BWF(head, memory[request[j]], BEST_FIT, sequence);
					if (index == -1) {
						printf("����ʧ��!\n");
						BFcnt++;
						break;
					}
					else {
						startPos[request[j]] = index;
					}
				}
				else if (work[request[j]] == -1) {
					//�����ڴ�
					if (hideDetails == 'n')
						printf("������ҵ%d���ڴ�:  ��ʼ��ַ:%d\t�ڴ��С:%d\n", request[j], startPos[request[j]], memory[request[j]]);
					UnorderedRecycle(head, startPos[request[j]], memory[request[j]], BEST_FIT, sequence);
				}
				work[request[j]]--;
				if (hideDetails == 'n') ShowParts(sequence);
				if (j == reqLen - 1) printf("�����ɹ�!\n");
			}
			DestroyPart(head);
			DestroyPart(sequence);
		}

		//�����Ӧ�㷨
		if (mode == ALL_RUN || mode == WORST_FIT) {
			printf("�����Ӧ�㷨BF: \n");
			memset(work, 0, job * sizeof(int));
			memset(startPos, -1, job * sizeof(int));
			Partition* head = CreateHead(size);
			Partition* sequence = CreateHead(size);
			if (hideDetails == 'n') {
				printf("��ʼ��������:\n");
				ShowParts(head);
			}
			for (int j = 0; j < reqLen; j++) {
				if (work[request[j]] == 0) {
					//�����ڴ�
					if (hideDetails == 'n') printf("��ҵ%d��������ڴ�:%d\n", request[j], memory[request[j]]);
					int index = BWF(head, memory[request[j]], WORST_FIT, sequence);
					if (index == -1) {
						printf("����ʧ��!\n");
						WFcnt++;
						break;
					}
					else {
						startPos[request[j]] = index;
					}
				}
				else if (work[request[j]] == -1) {
					//�����ڴ�
					if (hideDetails == 'n')
						printf("������ҵ%d���ڴ�:  ��ʼ��ַ:%d\t�ڴ��С:%d\n", request[j], startPos[request[j]], memory[request[j]]);
					UnorderedRecycle(head, startPos[request[j]], memory[request[j]], WORST_FIT, sequence);
				}
				work[request[j]]--;
				if (hideDetails == 'n') ShowParts(sequence);
				if (j == reqLen - 1) printf("�����ɹ�!\n");
			}
			DestroyPart(head);
			DestroyPart(sequence);
		}

		free(work);
		free(request);
		free(memory);
		free(startPos);
	}

	printf("\n��%d��������У�����\n", reqNum);
	if (mode == FIRST_FIT || mode == ALL_RUN)
		printf("�״���Ӧ�㷨����ʧ��%d�Σ�ʧ���ʣ�%.2lf%%\n", FFcnt, (double)FFcnt / reqNum * 100);
	if (mode == NEXT_FIT || mode == ALL_RUN)
		printf("ѭ���״���Ӧ�㷨����ʧ��%d�Σ�ʧ���ʣ�%.2lf%%\n", NFcnt, (double)NFcnt / reqNum * 100);
	if (mode == BEST_FIT || mode == ALL_RUN)
		printf("�����Ӧ�㷨����ʧ��%d�Σ�ʧ���ʣ�%.2lf%%\n", BFcnt, (double)BFcnt / reqNum * 100);
	if (mode == WORST_FIT || mode == ALL_RUN)
		printf("�����Ӧ�㷨����ʧ��%d�Σ�ʧ���ʣ�%.2lf%%\n", WFcnt, (double)WFcnt / reqNum * 100);

	return 0;
}
