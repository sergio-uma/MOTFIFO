#ifndef MOTFIFO_H
#define MOTFIFO_H

#include <malloc.h> // NULL

// ok, warning and errors returned by functions
#define W_DROP   (int)  1
#define OK_RET   (int)  0
#define E_NOELEM (int) -1
#define E_NOLIST (int) -2
#define E_NOCRIT (int) -3
#define E_VALUE  (int) -4
#define E_RANGE  (int) -5

// handler for Motfifo (struct hidden)
typedef struct Motfifo_t * Motfifo;
// helpers types
typedef int(*CmpFunction)(void*,void*);
typedef void(*FreeElementFunction)(void*);
typedef char*(*SprintElementFunction)(void*);

// arguments for default functionality
// for traverse criterion: FIFO 0, other ordering: 1, 2, ...
#define T_FIFO	(int)0
// searching direction
#define DIRECT_ORDER  (int)0
#define REVERSE_ORDER (int)1
// for commodity CMP functions
#define DEFAULT_CMP (CmpFunction)defaultCMP
#define LONGINT_CMP (CmpFunction)cmpLong
#define DOUBLEF_CMP (CmpFunction)cmpDouble
// for commodity print functions
#define DEFAULT_PRN (SprintElementFunction)sprintDfl
#define LONGINT_PRN (SprintElementFunction)sprintLong
#define DOUBLEF_PRN (SprintElementFunction)sprintDouble
// for commodity element destrutor
#define DEFAULT_DTOR (FreeElementFunction)NULL
#define FREEMEM_DTOR (FreeElementFunction)free




// public functions: list interface
// constructor, setup and destructor
extern int createMotfifo(long size, int nCrit, Motfifo *pM);
extern int setCriterionCmpFunction(Motfifo m, int crit, CmpFunction function);
extern int setFreeElementFunction(Motfifo m, FreeElementFunction function);
extern int destroyMotfifo(Motfifo m);
// get list info
extern int getSizeMotfifo(Motfifo m, long *size);
extern int getInfoMotfifo(Motfifo m, long *nElem, int *nCrit, long *size);
// insert/extract elements in sequence order: FIFO
extern int insertElemMotfifo(Motfifo m, void *newE, void **oldE);
extern int extractElemMotfifo(Motfifo m, void **elem);
// get element reference (keep it in fifo)
extern int getElembyOrder(Motfifo m, int crit, int dir, long ordinal, void **elem);
// ordinal translation from one criterion to another
extern int getOrderbyOrdinal(Motfifo m, int icrit, int idir, long iordinal,
										int ocrit, int odir, long *oordinal);
// transtions: FIFO position <---> Order provided by 'CMP' function
#define getPositionbyOrder(m, crit, dir, ordinal, seq) \
		getOrderbyOrdinal(m,crit,dir,ordinal,0,0,seq)
#define getOderbyPosition(m, seq, crit, dir, ordinal) \
		getOrderbyOrdinal(m,0,0,seq,crit,dir,ordinal)
// traversing MOTFIFO
extern int traverseMotfifo(Motfifo m,int criterion,int dir,void(*funct)(void*));
// tag handling for explicit traversing (getFirstTag, getNextTag, getElembyTag)
// do not modify Motfifo (insert) while traversing using tags!
#define getTagbyOrder(m, crit, dir, ordinal, tag) \
		getLocationbyOrder(m,crit,dir,ordinal,tag)
#define getFisrtTagbyOrder(m, crit, dir, tag) \
		getFirstLocationbyOrder(m,crit,dir,tag)
#define getNextTagbyOrder(m, crit, dir, tag) \
		getNextLocationbyOrder(m,crit,dir,tag)
#define getOrderbyTag(m, tag, crit, dir, ordinal) \
		getOrderbyLocation(m,tag,crit,dir,ordinal)
#define getElembyTag(m, tag, elem) \
		getElembyLocation(m,tag,elem)


// print list on stdio
extern int printHeaderMotfifo(Motfifo m);
extern int printMotfifo(Motfifo m,int criterion,int dir,char*(*sprint)(void*));


// function to support tags
int getLocationbyOrder(Motfifo m, int crit, int dir, long ordinal, long *loc);
int getFirstLocationbyOrder(Motfifo m, int crit, int dir, long *loc);
int getNextLocationbyOrder(Motfifo m, int crit, int dir, long *loc);
int getOrderbyLocation(Motfifo m, long loc, int crit, int dir, long *ordinal);
int getElembyLocation(Motfifo m, long loc, void **elem);

// comodity sprint functions - Usage: DEFAULT_PRN, LONGINT_PRN or DOUBLEF_PRN
extern char* sprintDfl(void *element);
extern char* sprintLong(void *element);
extern char* sprintDouble(void *element);

// comodity CMP functions - Usage: DEFAULT_CMP, LONGINT_CMP or DOUBLEF_CMP
extern int defaultCMP(void *dummy1, void *dummy2);
extern int cmpLong(void *new, void *inList);
extern int cmpDouble(void *new, void *inList);


#endif
