/*
 * Proyect: Multiple Ordering Travesal - FIFO: MOTFIFO
 * 		/main.c				-> demo program using libmotfifo.a + motfifo.h
 * 		/Makefile			-> invoke $make to generate the demo program.
 * 		/lib				-> library directory
 * 		/lib/libmotfifo.a	-> library file to link with -lmotfifo
 * 		/include			-> public headers directory
 * 		/include/motfifo.h	-> header file to include in aplication source file
 * 		/src				-> source directory
 * 		/src/motfifo.c		-> source of library
 * 		/src/motfifo_priv.h	-> headers for library private functions
 * 		/src/Makefile		-> invoke make to generate libmotfifo.a
 *
 * Author: Sergio Romero (sromero@uma.es)
 * Date: 2018-Nov-18
 */


#include "motfifo_priv.h"

/******************************************************************************
 * Constructor & destructor
 ******************************************************************************/
int createMotfifo(long size, int nCrit, Motfifo *pM)
{
	Motfifo m;
	int i;

	if ((size < 1) || (nCrit < 0))
	{
		*pM=NULL;
		return E_VALUE;
	}

	m=(Motfifo)malloc(sizeof(struct Motfifo_t));
	m->size=size;
	m->nElem=0;
	m->nCrit=nCrit;
	m->head=0;
	m->tail=-1;
	m->efree=NULL;
	m->sprint=NULL;
	m->ehandler=(void **)malloc(size*sizeof(void *));
	if (nCrit)
	{
		m->sort=(struct Slot_t*)malloc(nCrit*sizeof(struct Slot_t));
		m->sort--; /* from 1 to nCrit */
	}
	else
	{
		m->sort=NULL;
	}
	for (i=1; i<=nCrit; i++)
	{
		m->sort[i].head=-1;
		m->sort[i].tail=-1;
		m->sort[i].ecmp=NULL;
		m->sort[i].index=(struct Index_t*)malloc(size*sizeof(struct Index_t));
	}
	*pM=m;
	return OK_RET;
}
/*
 * deallocate MOTFIFO memory, elements inhere are not freed neither returned
 */
void in_destroyMotfifo(Motfifo m)
{
	int i;

	for (i=1; i <= m->nCrit; i++)
		free(m->sort[i].index);
	m->sort++; // now points to malloc returned address
	free(m->sort);
	free(m->ehandler);
	free(m);
}
/*
 * destroy the MOTFIFO object
 */
int destroyMotfifo(Motfifo m)
{
	long nElem;
	int ret;

	if ((ret=getInfoMotfifo(m,&nElem,NULL,NULL)) != OK_RET) return ret;
	if (nElem > 0)
	{
		if (m->efree)
			ret = traverseMotfifo(m, T_FIFO, 0, m->efree);
		else
			ret = W_DROP;
	}
		
	in_destroyMotfifo(m);
	return ret;
}
/******************************************************************************/
inline int checkMotfifo(Motfifo m)
{
	if (m==NULL) return E_NOLIST;
	if ((m->sort == NULL)||(m->ehandler == NULL)) return E_NOLIST;
	return OK_RET;
}
inline void in_setFreeElementFunction(Motfifo m, FreeElementFunction function)
{
	m->efree = function;
}
int setFreeElementFunction(Motfifo m, FreeElementFunction function)
{
	int ret;

	if ((ret=checkMotfifo(m)) != OK_RET) return ret; //some check
	in_setFreeElementFunction(m,function);
	return OK_RET;
}
inline void in_setCriterionCmpFunction(Motfifo m,int crit,CmpFunction function)
{
	m->sort[crit].ecmp = function;
}
int setCriterionCmpFunction(Motfifo m, int crit, CmpFunction function)
{
	long nElem;
	int nCrit;
	int ret;

	if ((ret=getInfoMotfifo(m,&nElem,&nCrit,NULL)) != OK_RET) return ret;
	if ((crit < 1) || (crit > nCrit)) return E_VALUE;

	in_setCriterionCmpFunction(m,crit,function);	// ecmp -> function
	in_setFirstLocationbyOrder(m,crit,0,-1);		// head -> -1
	in_setFirstLocationbyOrder(m,crit,1,-1);		// tail -> -1
	if ((nElem > 0)&&(function!=NULL)) buildSorting(m,crit);
	return OK_RET;
}
CmpFunction in_getCriterionCmpfunction(Motfifo m, int crit)
{
	return m->sort[crit].ecmp;
}
/******************************************************************************/

/*
 * Function name: getSizeMotfifo
 * Arguments:
 * 		Motfifo *l -> pointer to a Motfifo type object
 * 		long *s -> the place to return size
 * Return:
 * 		RET_OK if no error
 * 		E_NOLIST in case of error
 */
int getSizeMotfifo(Motfifo m, long *s)
{
	int ret;
	if ((ret=checkMotfifo(m)) != OK_RET) return ret;
	if (m->size <= 0) return E_NOLIST;
	if (s != NULL) *s = m->size;
	return OK_RET;
}


/*
 * Function name: getInfoMotfifo
 * Arguments:
 * 		Motfifo *m -> pointer to a Motfifo type object
 * 		long *n -> the place to retrun the number of elements
 * 		int  *c -> the place to return number of sorting criterion
 * 		long *s -> the place to return the size if not NULL
 * Return:
 * 		RET_OK if no error
 * 		E_NOLIST in case of error
 */
int getInfoMotfifo(Motfifo m, long *n, int *c, long *s)
{
	long size;
	int ret;

	if ((ret = getSizeMotfifo(m,&size)) != OK_RET) return ret;
	if (s != NULL) *s=size; // if place for 'size' is provided

	if (m->nCrit < 0) return E_NOLIST;
	if (c != NULL) *c=m->nCrit;

	if ((m->nElem > size)||(m->nElem < 0)) return E_NOLIST;	
	if (n != NULL) *n=m->nElem;

	return OK_RET;
}



/*
 * Private
 * No error check
 */
inline long in_getEnqueueLocation(Motfifo m)
{
	m->nElem++;
	return m->tail = (m->tail + 1) % m->size; // before: (m->nElem < m->size)
}
inline long in_getDequeueLocation(Motfifo m)
{
	long loc = m->head;
	m->nElem--;
	m->head = (loc + 1) % m->size; // before: (m->nElem > 0)
	return loc;
}
inline long in_getFirstLocation(Motfifo m, int dir)
{
	return (dir==0)? m->head: m->tail;
}
inline long in_getNextLocation(Motfifo m, long loc, int dir)
{
	if (loc==in_getFirstLocation(m,!dir)) return -1;
	return (dir==0)? (loc+1)%m->size: (loc-1+m->size)%m->size;
}
inline long in_getFirstLocationbyOrder(Motfifo m, int crit, int dir)
{
	return (dir==0)? m->sort[crit].head: m->sort[crit].tail;
}
inline void in_setFirstLocationbyOrder(Motfifo m, int crit, int dir, long loc)
{
	if (dir==0)
		m->sort[crit].head = loc;
	else
		m->sort[crit].tail = loc;
}
inline long in_getNextLocationbyOrder(Motfifo m, long loc, int crit, int dir)
{
	if (loc==-1) return -1; //unnecesary
	return (dir==0)?m->sort[crit].index[loc].next:m->sort[crit].index[loc].prev;
}
inline void in_setNextLocationbyOrder(Motfifo m, long loc, int crit, int dir, long nloc)
{
	if (dir==0)
		m->sort[crit].index[loc].next = nloc;
	else
		m->sort[crit].index[loc].prev = nloc;
}
inline long in_getLocationbyPosition(Motfifo m, int dir, long ordinal)
{
	return (dir==0)?(m->head+ordinal-1)%m->size:
					(m->tail-ordinal+1+m->size)%m->size;
}
inline long in_getPositionbyLocation(Motfifo m, long loc, int dir)
{
	return (dir==0)?(loc - m->head + 1 + m->size) % m->size:
					(loc + m->tail - 1) % m->size;
}
inline void* in_getElembyLocation(Motfifo m, long loc)
{
	return m->ehandler[loc];
}
inline void in_setElementbyLocation(Motfifo m, long loc, void *e)
{
	m->ehandler[loc]=e;
}

/*
 * Private functions
 */
int checkLocation(Motfifo m, long loc)
{
	/*
	long size;
	int ret;
	
	if ((ret=getSizeMotfifo(m, &size)) != OK_RET) return ret;
	*/
	if ((loc < 0) || (loc >= m->size)) return E_RANGE; // unnecesary
	if (m->head <= m->tail)
	{
		if ((loc < m->head) || (loc > m->tail))
			return E_VALUE;
	}
	else
	{
		if ((loc > m->tail) && (loc < m->head))
			return E_VALUE;
	}
	return OK_RET;
}



/*
 * Public
 * Function name: getOrderbyOrdinal
 *
 * Rationale:
 * 	Find an element following a sort criterion, direction and ordinal (from:
 *  icrit, idir, iordinal) and determine the corresponding ordinal (oordinal)
 * 	following another criterion and direction (from ocrit, odir).
 * 
 * Arguments:
 * 		Motfifo *m -> pointer to a Motfifo type object
 * 
 * 		int icrit		-> criterion to find the element
 * 		int idir		-> direction to find the element
 * 		long iordinal	-> ordinal in the idir of icrit to find the element
 * 
 * 		int ocrit		-> criterion in which determine the oordinal
 * 		int odir		-> direction in which determine the oordinal
 * 		long *oordinal	-> the ordinal of the element following ocrit and odir
 */
extern int getOrderbyOrdinal(Motfifo m, int icrit, int idir, long iordinal,
										int ocrit, int odir, long *oordinal)
{
	long loc;
	int ret;

	*oordinal = -1;
	if ((ret=getLocationbyOrder(m,icrit,idir,iordinal,&loc)) != OK_RET) return ret;
	if ((ret=getOrderbyLocation(m,loc,ocrit,odir,oordinal)) != OK_RET) return ret;

	return OK_RET;
}





/*
 * Private
 * 
 * Function name: getLocationbyOrder
 * Arguments:
 * 		Motfifo *l -> pointer to a Motfifo type object
 * 		long ordinal -> i-th element in order view
 * 		int direction -> if (direction==0) top-down else botton-up
 * 		long *loc -> place to return the location in data array
 */
int getLocationbyOrder(Motfifo m, int crit, int dir, long ordinal, long *loc)
{
	long i;
	long nElem;
	int nCrit;
	int ret;

	*loc = -1;
	if ((ordinal < 1) || (crit<0)) return E_VALUE;
	if ((ret=getInfoMotfifo(m,&nElem,&nCrit,NULL)) != OK_RET) return ret;
	if (crit > nCrit) return E_VALUE;
	if (nElem == 0) return E_NOELEM;
	if (ordinal > nElem) return E_NOELEM; // E_VALUE ?

	if (crit==T_FIFO)
	{
		i=in_getLocationbyPosition(m,dir,ordinal);
		ret = OK_RET;
	}
	else
	{
		if (in_getCriterionCmpfunction(m,crit) == NULL) return E_NOCRIT;
		for (i=in_getFirstLocationbyOrder(m,crit,dir);
				((i!=-1)&&(--ordinal));
				i=in_getNextLocationbyOrder(m,i,crit,dir)) {/*nothing*/};

		if (i == -1) ret = E_NOLIST; // data may be corrupted
	}
	*loc=i;	
	return ret;
}
/*
 * Private
 */
int getOrderbyLocation(Motfifo m, long loc, int crit, int dir, long *ordinal)
{
	long nElem,size;
	int nCrit;
	int ret;
	long i;
	long counter=1;

	*ordinal = -1;
	if ((ret=getInfoMotfifo(m,&nElem,&nCrit,&size)) != OK_RET) return ret;
	if ((crit < 0) || (crit > nCrit)) return E_VALUE;
	if ((ret=checkLocation(m,loc)) != OK_RET) return ret;

	if (crit == T_FIFO)
	{
		*ordinal = in_getPositionbyLocation(m,loc,dir);
		ret = OK_RET;
	}
	else
	{
		if (in_getCriterionCmpfunction(m,crit) == NULL) return E_NOCRIT;
		for (i=in_getFirstLocationbyOrder(m,crit,dir);
				((i!=loc)&&(i!=-1));
				i=in_getNextLocationbyOrder(m,i,crit,dir)) counter++;
		
		*ordinal = counter;
		if (i == -1) ret = E_NOLIST; // data may be corrupted
	}
	return ret;
}



/*
 * Private
 * return the location in the array of the first element by order
 */
int getFirstLocationbyOrder(Motfifo m, int crit, int dir, long *loc)
{
	int ret;
	int nCrit;
	long nElem;
	long size;
	
	if ((ret=getInfoMotfifo(m,&nElem,&nCrit,&size)) != OK_RET) return ret;
	if ((crit < 0) || (crit > nCrit)) return E_VALUE;
	
	if (crit==T_FIFO)
	{
		*loc=in_getFirstLocation(m,dir);
	}
	else
	{
		if (in_getCriterionCmpfunction(m,crit) == NULL)
		{
			*loc = -1;
			return E_NOCRIT;
		}
		*loc=in_getFirstLocationbyOrder(m,crit,dir);
	}
	return OK_RET;
}

/*
 * Private
 *
 * Return the location in the array of the next (by order) element with
 * respect to the given element
 *
 * Note:
 * 	First call to this function should be after a call to:
 * 			getLocationbyOrder() or getFirstLocation() returning OK_RET
 *
 */
int getNextLocationbyOrder(Motfifo m, int crit, int dir, long *loc)
{
	int ret;
	int nCrit;
	long nElem;
	long size;
	
	if ((ret=getInfoMotfifo(m,&nElem,&nCrit,&size)) != OK_RET) return ret;
	if ((crit < 0) || (crit > nCrit)) return E_VALUE;
	if ((ret=checkLocation(m,*loc)) != OK_RET)
	{
		*loc = -1;
		return ret;
	}
	if (crit==T_FIFO)
		*loc=in_getNextLocation(m,*loc,dir);
	else
	{
		if (in_getCriterionCmpfunction(m,crit) == NULL)
		{
			*loc = -1;
			return E_NOCRIT;
		}
		*loc=in_getNextLocationbyOrder(m,*loc,crit,dir);
	}
	return OK_RET;
}



/******************************************************************************
 * Get elements by any reference (order, fifo, tag)
 ******************************************************************************/
/* Function name: getElembyLocation
 * Arguments:
 * 		Motfifo *m -> pointer to a Motfifo type object
 * 		long index -> index in the data array
 * 		void **elem -> place to return a pointer to the element
 */
int getElembyLocation(Motfifo m, long loc, void **elem)
{
	int ret;

	*elem=NULL;
	if ((ret=checkLocation(m,loc)) != OK_RET) return ret;

	*elem=in_getElembyLocation(m,loc);
	return OK_RET;
}

/*
 * Function name: getElembyOrder
 * Arguments:
 * 		Motfifo *m -> pointer to a Motfifo type object
 *		int criteria -> T_FIFO:0, 1..nCrit
 * 		int direction -> if (direction==0) top-down else botton-up
 * 		int ordinal -> i-th element in order view
 * 		void **elem -> place to return a pointer to the element
 */
int getElembyOrder(Motfifo m, int crit, int dir, long ordinal, void **elem)
{
	long loc;
	int ret;

	*elem=NULL;
	if ((ret=getLocationbyOrder(m,crit, dir, ordinal, &loc)) != OK_RET) return ret;

	*elem=in_getElembyLocation(m,loc);
	return OK_RET;
}


/******************************************************************************/



/******************************************************************************
 * Commodity comparison functions
 ******************************************************************************/
// Funcion comparador nuevo mayor que anteriores (orden == input sequence: FIFO)
int defaultCMP(void *dummy1, void *dummy2)
{
	return 1;
}
// comparador de enteros
int cmpLong(void *new, void *inList)
{
	return ((long)new >= (long)inList );
}
// comparador de flotantes
int cmpDouble(void *new, void *inList)
{
	return (*(double*)&new >= *(double*)&inList );
}
/******************************************************************************/



/******************************************************************************
 * Inserts/extracts elements in the list in sequence order FIFO
 ******************************************************************************/
/*
 * Inserts a new element in the list. Ordering information is uptated using
 * 	function 'cmp' to compare the new element provided against the previous
 * 	elements in the list.
 * 
 * Arguments:
 * 		Motfifo *l -> pointer to a Motfifo object
 * 		void *newE -> pointer to the new Element in list
 * 		cmp() -> return 0: newElement is lower than the compared element in list
 * 				return 1: newElement is higher or equal than the element in list
 * 			could be 'defaultCMP()' which use the input secuence as order
 * 		void**oldE -> element extracted of exhausted list or NULL if not full
 *
 * Return:
 * 		OK_RET: if the number of elements in the list before calling this
 * 				function is less than the 'size' determined on creation.
 * 		LOSSING: If the number of elements in the list has reached the 'size'
 * 				the firts element in sequence (FIFO) is extracted from list
 * 				before new element is inserted. The extracted element pointer is
 * 				placed in the memory position indicated by oldE (if not NULL).
 */
int insertElemMotfifo(Motfifo m,void *newE,void **oldE)
{
	int ret;
	long size;
	int nCrit;
	long nElem;
	int retval;
	int c;
	void *eptr;

	if (oldE)
		*oldE=NULL;
	if ((ret=getInfoMotfifo(m,&nElem,&nCrit,&size)) != OK_RET) return ret;

	if (nElem < size)
	{
		retval=OK_RET;		// there were room for new element in list
	}
	else
	{
		eptr=dropElem(m);	// extract element from tail, caller will free
		if (oldE)
			*oldE=eptr;		// return element if a place for it is provided
		retval=W_DROP;		// dropping the oldest element
	}

	// Now insert in FIFO
	long loc = in_getEnqueueLocation(m);
	in_setElementbyLocation(m,loc,newE);
	// Now update each order criteria, inserting search: dir 0 -> top-down
	for (c=1; c<=nCrit; c++)
	{
		updateSorting(m, c, loc);
	}
	return retval;
}
/*
 * m -> motfifo
 * crit -> criterion to update
 * loc -> new location to insert
 */
void updateSorting(Motfifo m, int crit, long loc)
{
	long i;
	int found;
	CmpFunction cmpFunction;
	void *newE;

	if ((cmpFunction=in_getCriterionCmpfunction(m,crit)) == NULL) return;
	
	newE = in_getElembyLocation(m, loc);	
	long head=in_getFirstLocationbyOrder(m,crit,0);
	for (i=head, found=0; (i != -1); i=in_getNextLocationbyOrder(m,i,crit,0))
	{
		if ( cmpFunction(newE,in_getElembyLocation(m,i)) )
		{
			found = 1;
			break;
		}
	}
	if (i == head)
		in_setFirstLocationbyOrder(m,crit,0,loc);
	in_setNextLocationbyOrder(m,loc,crit,0,i);

	if (found)
	{
		long prev=in_getNextLocationbyOrder(m,i,crit,1);
		in_setNextLocationbyOrder(m,loc,crit,1,prev);
		if (prev != -1)
			in_setNextLocationbyOrder(m,prev,crit,0,loc);
		in_setNextLocationbyOrder(m,i,crit,1,loc);
	}
	else // (i == -1)
	{
		long tail=in_getFirstLocationbyOrder(m,crit,1);
		if (tail != -1)
			in_setNextLocationbyOrder(m,tail,crit,0,loc);
		in_setNextLocationbyOrder(m,loc,crit,1,tail);

		in_setFirstLocationbyOrder(m,crit,1,loc);
	}
}
/*
 * build a new sorting from a populated fifo
 */
void buildSorting(Motfifo m, int crit)
{
	long i;
	for (i=in_getFirstLocation(m,0); (i!=-1); i=in_getNextLocation(m,i,0))
		updateSorting(m,crit,i);
}
/* Private function (not part of the user interface)
 * Assuming correct list and at least one element
 */
void* dropElem(Motfifo m)
{
	int c;

	long loc = in_getDequeueLocation(m);
	void *element=in_getElembyLocation(m,loc); // element to drop
	
	// for crit
	for (c=1; c<=m->nCrit; c++)
	{
		if (in_getCriterionCmpfunction(m,c) == NULL) continue;

		long next = in_getNextLocationbyOrder(m,loc,c,0);
		long prev = in_getNextLocationbyOrder(m,loc,c,1);
		
		if (loc == in_getFirstLocationbyOrder(m,c,0))
			in_setFirstLocationbyOrder(m,c,0,next);
		else
			in_setNextLocationbyOrder(m,prev,c,0,next);

		if (loc == in_getFirstLocationbyOrder(m,c,1))
			in_setFirstLocationbyOrder(m,c,1,prev);
		else
			in_setNextLocationbyOrder(m,next,c,1,prev);
	}

	return element;
}
/*
 * Extract first element in sequence (FIFO)
 */
int extractElemMotfifo(Motfifo m, void **elem)
{
	int ret;
	long nElem;
	void *eptr;

	if (elem)
		*elem = NULL;	// nullify if element required and return with error

	if ((ret=getInfoMotfifo(m,&nElem,NULL,NULL)) != OK_RET)	return ret;
	if (nElem == 0) return E_NOELEM;
	
	eptr = dropElem(m); // EXTRACTION

	if (m->efree)
	{
		m->efree(eptr);	// if destrutor available, invoke it
		ret=OK_RET;		// return OK
		eptr=NULL;		// Nullify output
	}
	else
	{
		ret=W_DROP;		// return Warning
	}

	if (elem)
		*elem = eptr;	// leave available if required

	return ret;
}
/******************************************************************************/



/******************************************************************************
 * Traversing
 ******************************************************************************/
int traverseMotfifo(Motfifo m, int crit, int dir, void(*function)(void *))
{
	long nElem;
	long i;
	int nCrit;
	int ret;

	if ((function == NULL)||(crit < 0)) return E_VALUE;
	if ((ret=getInfoMotfifo(m,&nElem,&nCrit,NULL)) != OK_RET) return ret;
	if (crit > nCrit) return E_VALUE;
	if ((crit)&&(in_getCriterionCmpfunction(m,crit) == NULL)) return E_NOCRIT;
	
	for (ret=getFirstLocationbyOrder(m,crit,dir,&i);
			(ret==OK_RET) && (i != -1);
			ret=getNextLocationbyOrder(m,crit,dir,&i))
	{
		function(in_getElembyLocation(m,i));
	}
	return ret;
}
/******************************************************************************/



/******************************************************************************
 * Commodity element printing functions
 ******************************************************************************/
// print elements of type long to new allocated string, caller must free
char* sprintDfl(void *e)
{
	char *str=malloc(21);
	snprintf(str, 21, "0x%016lx", (long)e);
	return str;
}
// print elements of type long to new allocated string, caller must free
char* sprintLong(void *e)
{
	char *str=malloc(21);
	snprintf(str, 21, "%20ld", (long)e);
	return str;
}
// print elements of type double to new allocated string, caller must free
char* sprintDouble(void *e)
{
	char *str=malloc(21);
	snprintf(str, 21, "%20lf", *(double*)&e);
	return str;
}
/******************************************************************************/


/******************************************************************************
 * Commodity list printing functions
 ******************************************************************************/
// print header motfifo
int printHeaderMotfifo(Motfifo m)
{
	long size;
	long nElem;
	int nCrit;
	int ret;

	if ((ret=getInfoMotfifo(m,&nElem,&nCrit,&size)) != OK_RET)
	{
		printf("No MOTFIFO or object corrupted.\n");
		return ret;
	}
	printf("Allocated size: %3ld\n",size);
	printf("Number of Elements: %3ld",nElem);
	if (nElem == 0)
	{
		printf("\t(MOTFIFO empty)\n");
		ret = E_NOELEM;
	}
	else
	{
		printf("\n");
		ret = OK_RET;
	}
	printf("Number of sorting criteria: %d\n",nCrit);

	return ret;
}
// print list in order
int printMotfifo(Motfifo m,int crit,int dir,SprintElementFunction sprint)
{
	long i;
	char *elementStr;
	void *eptr;
	int ret;
	
	if ((ret=printHeaderMotfifo(m)) != OK_RET) return ret; // E_NOELEM
	if ((crit)&&(in_getCriterionCmpfunction(m,crit) == NULL))
	{
		printf("No CMPfunction for criterion %d\n",crit);
		return E_NOCRIT;
	}

if (crit==T_FIFO)
printf("Head: %ld\tTail: %ld\n",m->head,m->tail);
else
printf("Head: %ld\tTail: %ld\n",m->sort[crit].head,m->sort[crit].tail);

	for (ret=getFirstLocationbyOrder(m,crit,dir,&i);
			(ret==OK_RET) && (i != -1);
			ret=getNextLocationbyOrder(m,crit,dir,&i))
	{
		eptr=in_getElembyLocation(m,i);
		elementStr=sprint(eptr);
		printf("[%3ld] value: %20s\n",i,elementStr);
		free(elementStr);
	}
	return ret;
}
/******************************************************************************/
