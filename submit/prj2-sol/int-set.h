#include <assert.h>
#include <stdlib.h>
#ifndef INT_SET_H_
#define INT_SET_H_

typedef struct NodeStruct { //linked-list node for int
  int value;               //stored value
  struct NodeStruct *succ; //point to successor node; NULL if none.
} Node;

typedef struct { //header for sorted-list
  int nElements; //# of elements currently in list
  Node dummy;    //dummy Node; value never used
                 //dummy facilitates adding elements to list.
} Header;

static Node *
linkNewNodeAfter(Node *p0, int value)
{
    //create and insert new node after p0
    Node *p = malloc(sizeof(Node));   //create a new Node to hold value
    if (!p) return NULL;              //malloc failure
    p->value = value;
    p->succ = p0->succ; p0->succ = p; //link new node into list
    return p;
}

/** Remove node after p0 from link-list freeing it up.  Link p0 to the
 *  successor of the unlinked node.  Return succ of node free'd up.
 */
static Node *
unlinkNodeAfter(Node *p0)
{
    Node *p = p0->succ;
    p0->succ = p->succ;
    free(p);
    return p0->succ;
}

/** Abstract data type for set of int's.  Note that sets do not allow
 *  duplicates.
 */

/** Return a new empty int-set.  Returns NULL on error with errno set.
 */
void *newIntSet();

/** Return # of elements in intSet */
int nElementsIntSet(void *intSet);

/** Return non-zero iff intSet contains element. */
int isInIntSet(void *intSet, int element);

/** Change intSet by adding element to it.  Returns # of elements
 *  in intSet after addition.  Returns < 0 on error with errno
 *  set.
 */
int addIntSet(void *intSet, int element);

/** Change intSet by adding all elements in array elements[nElements] to
 *  it.  Returns # of elements in intSet after addition.  Returns
 *  < 0 on error with errno set.
 */
int addMultipleIntSet(void *intSet, const int elements[], int nElements);

/** Set intSetA to the union of intSetA and intSetB.  Return # of
 *  elements in the updated intSetA.  Returns < 0 on error.
 */
int unionIntSet(void *intSetA, void *intSetB);

/** Set intSetA to the intersection of intSetA and intSetB.  Return #
 *  of elements in the updated intSetA.  Returns < 0 on error.
 */
int intersectionIntSet(void *intSetA, void *intSetB);

/** Free all resources used by previously created intSet. */
void freeIntSet(void *intSet);

/** Return a new iterator for intSet.  Returns NULL if intSet
 *  is empty.
 */
const void *newIntSetIterator(const void *intSet);

/** Return current element for intSetIterator. */
int intSetIteratorElement(const void *intSetIterator);

/** Step intSetIterator and return stepped iterator.  Return
 *  NULL if no more iterations are possible.
 */
const void *stepIntSetIterator(const void *intSetIterator);


#endif //ifndef INT_SET_H_
