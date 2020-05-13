#ifndef MOTFIFO_PRIV_H
#define MOTFIFO_PRIV_H

#include "motfifo.h" // public header

struct Index_t {
	long next;
	long prev;
};

struct Slot_t {
	long head;
	long tail;
	CmpFunction ecmp;
	struct Index_t *index;
};

struct Motfifo_t {
	long size;
	long nElem;
	int nCrit;
	long head;
	long tail;
	FreeElementFunction efree;
	SprintElementFunction sprint;
	void **ehandler;
	struct Slot_t *sort;
};

// Private functions prototypes
// create/setup/destroy
int checkMotfifo(Motfifo m);
void in_setFreeElementFunction(Motfifo m, FreeElementFunction function);
void in_setCriterionCmpFunction(Motfifo m,int crit,CmpFunction function);
void buildSorting(Motfifo m, int crit);
void updateSorting(Motfifo m, int crit, long loc);
void in_destroyMotfifo(Motfifo m);
// insert/extract
long in_insertElemMotfifo(Motfifo m, void *newE);
void in_setElementbyLocation(Motfifo m, long i, void *newE);
void* dropElem(Motfifo m);
// translations: order provided by CMP <---> location in array
long in_getEnqueueLocation(Motfifo m);
long in_getDequeueLocation(Motfifo m);
long in_getFirstLocation(Motfifo m, int dir);
long in_getNextLocation(Motfifo m, long loc, int dir);
long in_getLocationbyPosition(Motfifo m, int dir, long ordinal);
long in_getFirstLocationbyOrder(Motfifo m, int crit, int dir);
long in_getNextLocationbyOrder(Motfifo m, long loc, int crit, int dir);
void in_setFirstLocationbyOrder(Motfifo m, int crit, int dir, long loc);
void in_setNextLocationbyOrder(Motfifo m,long loc,int crit,int dir,long nloc);

// translations: FIFO positions <---> locations in array
long in_getPositionbyLocation(Motfifo m, long loc, int dir);

// extract element from array location
void* in_getElembyLocation(Motfifo m, long index);
void in_setElementbyLocation(Motfifo m, long i, void *e);

// cmp functions
CmpFunction in_getCriterionCmpfunction(Motfifo m, int crit);
#endif
