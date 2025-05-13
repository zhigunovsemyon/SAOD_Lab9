#include "tree.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/*
В некоторой древовидной бинарной структуре опытным
путем измеряется частота обращения к каждому из элементов. Для
этого с любым из элементов связан счетчик обращений. По
прошествии определенного периода времени дерево реорганизуется.
Для этого оно просматривается и с помощью соответствующей
программы строится новое дерево, в которое ключи включаются в
порядке убывания счетчиков частот обращения. Напишите
программы, выполняющие такую реорганизацию. Будет ли средняя
длина пути в новом дереве равна, больше или даже значительно
меньше длины пути в оптимальном дереве?
*/

static int cmp_int(void const * a, void const * b)
{
	if (*(int *)a > *(int *)b)
		return 1;
	if (*(int *)a < *(int *)b)
		return -1;
	else /*if ==*/
		return 0;
}

static void printIntTreeNLR_(struct TreeNode * pNode)
{
	if (!pNode)
		return;
	pNode->access_count++;

	if (pNode->l != NULL) {
		printf("{%d|%d}", pNode->l->access_count,
		       *(int *)pNode->l->data);
	}
	printf(" - {%d|%d} - ", pNode->access_count, *(int *)pNode->data);
	if (pNode->r != NULL) {
		printf("{%d|%d}", pNode->r->access_count,
		       *(int *)pNode->r->data);
	}
	putc('\n', stdout);

	printf("l:\n");
	printIntTreeNLR_(pNode->l);
	printf("r:\n");
	printIntTreeNLR_(pNode->r);
	putchar('\n');
}

static void printIntTree(Tree * pTree)
{
	printIntTreeNLR_(pTree->root);
}

static void ra_tree(Tree * t, int arr[], size_t arrsize, int count)
{
	while (--count)
		TreeBelongs(t, arr + (rand() % arrsize));
}

int main()
{
	srand((unsigned)time(NULL));
	int array[] = {1, 4, 7, 2, 3, -8, 0};
	size_t size_arr = sizeof(array) / sizeof(*array);

	/*Инициализация дерева, заполнение из массива*/
	Tree * mt = TreeInit(sizeof(int), cmp_int);
	TreeInsertArray(mt, array, size_arr);

	printIntTree(mt);
	ra_tree(mt, array, size_arr, 100);
	
	printIntTree(mt);

	TreeFree(mt);
	return 0;
}
