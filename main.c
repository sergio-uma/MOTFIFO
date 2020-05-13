#include <stdlib.h> /* drand48() for test2 */
#include "motfifo.h"

// simple test
int test0()
{
	Motfifo myList;
	int err, nCrit;
	long nElem, size, i;

	createMotfifo(10, 1, &myList);
	setCriterionCmpFunction(myList, 1, DEFAULT_CMP);
	
	printMotfifo(myList,T_FIFO,0,LONGINT_PRN);
	printf("\n");
getchar();
	err=insertElemMotfifo(myList, NULL, NULL);
	printf("insert ret: %d\n",err);
	
	err=getInfoMotfifo(myList, &nElem, &nCrit, &size);
	printf("get ret: %d\tnElem: %ld\tCrit: %d\tsize: %ld\n\n",err,nElem,nCrit,size);

getchar();
	extractElemMotfifo(myList, NULL);

	printMotfifo(myList,T_FIFO,0,LONGINT_PRN);
	printf("\n");

	for (i=0; i<8; i++) insertElemMotfifo(myList, NULL, NULL);
	
	err=getInfoMotfifo(myList, &nElem, &nCrit, &size);
	printf("get ret: %d\tnElem: %ld\tCrit: %d\tsize: %ld\n\n",err,nElem,nCrit,size);
	
	printMotfifo(myList,T_FIFO,0,LONGINT_PRN);
	printf("\n");
	
	printMotfifo(myList,T_FIFO,1,LONGINT_PRN);
	printf("\n");
	
	printMotfifo(myList,1,1,LONGINT_PRN);
	printf("\n");

	printMotfifo(myList,1,0,LONGINT_PRN);
	printf("\n");

	return 0;
}

// overflow test
int test1()
{
	Motfifo myList;
	long i;
	long value;

	createMotfifo(10, 1, &myList);
	//setCriterionCmpFunction(myList, 1, DEFAULT_CMP);
	printMotfifo(myList,T_FIFO,0,sprintLong);
	printf("\n");

	for (i=0; i<50; i++) insertElemMotfifo(myList, (void*)100-i, NULL);
	
	printMotfifo(myList,T_FIFO,0,sprintLong);	
	printf("\n");

	printf("Last insertion: %ld\n",100-i);
	insertElemMotfifo(myList, (void*)100-i, (void *)&value);
	
	printMotfifo(myList,T_FIFO,0,sprintLong);	
	printf("\n");

	printf("Last extraction: %ld\n",value);
	
	return 0;
}

// test with cmp function
int myLongIntCMP(void *new, void *inList)
{
	return ((long)new >= (long)inList );
}

int test2()
{
	Motfifo myList;
	long i;
	double value;
	int ret;

	createMotfifo(10, 1, &myList);
	setCriterionCmpFunction(myList, 1, LONGINT_CMP);
	printMotfifo(myList,T_FIFO,0,LONGINT_PRN);
	printf("\n");


	for (i=0; i<8; i++)
	{
		value = drand48();
		printf("Inserting %ld: %ld\n",i,*(long*)&value);
		//insertElemMotfifo(myList, (void*)*(long*)&value, myLongIntCMP, NULL);
		insertElemMotfifo(myList, (void*)*(long*)&value, NULL);
		printMotfifo(myList,T_FIFO,0,LONGINT_PRN);
	}
	
	printMotfifo(myList,T_FIFO,0,LONGINT_PRN);
	printf("\n");

	printMotfifo(myList,1,1,LONGINT_PRN);
	printf("\n");

	ret=printMotfifo(myList,1,0,LONGINT_PRN);
	printf("\n");

	return ret;
}


int test3()
{
	Motfifo myList;
	long i;
	int ret;
	double value;
	
	createMotfifo(10, 2, &myList);
	setCriterionCmpFunction(myList, 1, DOUBLEF_CMP);

	printMotfifo(myList,T_FIFO,0,DOUBLEF_PRN);
	printf("\n");


	for (i=0; i<11; i++)
	{
		value=drand48();
		printf("---> Inserting: %lf\n",value);
		insertElemMotfifo(myList, (void*)*(long*)&value, NULL);
	}
	printf("\n");
	printf("Sorting criterion: %d\tDirection:%d\n",T_FIFO,0);		
//getchar();
	printMotfifo(myList,T_FIFO,0,DOUBLEF_PRN);
	printf("\n");

	printf("Sorting criterion: %d\tDirection:%d\n",1,1);
//getchar();
	printMotfifo(myList,1,1,DOUBLEF_PRN);
	printf("\n");

	printf("Sorting criterion: %d\tDirection:%d\n",1,0);
//getchar();
	printMotfifo(myList,1,0,DOUBLEF_PRN);
	printf("\n");

	printMotfifo(myList,T_FIFO,0,LONGINT_PRN);
	printf("\n");

	// trying to use criterion 2 before assigning a CMPfunction
	printMotfifo(myList,2,1,LONGINT_PRN);
	printf("\n");

	setCriterionCmpFunction(myList, 2, LONGINT_CMP); // should update sorting

	ret=printMotfifo(myList,2,0,LONGINT_PRN);
	printf("\n");

	return ret;
}


int main(int argc, char *argv[])
{
//	return test0();
//	return test1();
//	return test2();
	return test3();
}
