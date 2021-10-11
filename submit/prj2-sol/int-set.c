#include "int-set.h"

/** Abstract data type for set of int's.  Note that sets do not allow
 *  duplicates.
 */

/** Return a new empty int-set.  Returns NULL on error with errno set.
 */
void *newIntSet() {
    //by using calloc() we set nElements to 0 and succ to NULL.
    return calloc(1, sizeof(Header));
}

/** Return # of elements in intSet */
int nElementsIntSet(void *intSet) {
    const Header *header = (Header *)intSet;
    return header->nElements;
}

/** Return non-zero iff intSet contains element. */
int isInIntSet(void *intSet, int element) {
    Header *header = (Header *)intSet;
    for (Node *p = header->dummy.succ; p != NULL; p = p->succ) {
        if (p->value == element) {
            return 1;
        }
    }
    return 0;
}

/** Change intSet by adding element to it.  Returns # of elements
 *  in intSet after addition.  Returns < 0 on error with errno
 *  set.
 */
int addIntSet(void *intSet, int element) {
    Header *header = (Header *)intSet;
    Node *p0; //lagging pointer: will insert after p0.
    for (p0 = &header->dummy; p0->succ != NULL && p0->succ->value <= element; p0 = p0->succ) {}
    assert(p0->succ == NULL || p0->succ->value > element);
    //create and insert new node after p0
    if (!linkNewNodeAfter(p0, element)) return -1;
    ++header->nElements;
    //remove duplicates to turn list into set
    int nRemoved = 0;
    for (Node *p = header->dummy.succ; p != NULL; p = p->succ) {
        while (p->succ && p->succ->value == p->value) {
            unlinkNodeAfter(p);
            nRemoved++;
        }
    }
    header->nElements -= nRemoved;
    return header->nElements;
}

/** Change intSet by adding all elements in array elements[nElements] to
 *  it.  Returns # of elements in intSet after addition.  Returns
 *  < 0 on error with errno set.
 */
int addMultipleIntSet(void *intSet, const int elements[], int nElements) {
    int nAdd = 0;
    for(int i=0; i < nElements; i++) {
        nAdd = addIntSet(intSet, elements[i]);
        if(nAdd == -1) {
            return -1;
        }
    }
    return nAdd;
}

/** Set intSetA to the union of intSetA and intSetB.  Return # of
 *  elements in the updated intSetA.  Returns < 0 on error.
 */
int unionIntSet(void *intSetA, void *intSetB) {
    //init stuff
    Header *headerA = (Header *)intSetA;
    Node *pA0 = &headerA->dummy;
    Header *headerB = (Header *)intSetB;
    Node *pB = headerB->dummy.succ;

    for(;pA0->succ != NULL && pB != NULL;) {
        if(pA0->succ->value < pB->value) {
            pA0 = pA0->succ;
        }
        else if(pA0->succ->value == pB->value) {
            pA0 = pA0->succ;
            pB = pB->succ;
        }
        else if(pA0->succ->value > pB->value) {
            pA0->succ = linkNewNodeAfter(pA0, pB->value);
            pA0 = pA0->succ;
            pB = pB->succ;
        }
    }
    if(pA0->succ == NULL && pB != NULL) {
        while(pB != NULL) {
            pA0->succ = linkNewNodeAfter(pA0, pB->value);
            pA0 = pA0->succ;
                pB = pB->succ;
        }
    }
    int count = 0;
    for(Node *pA = headerA->dummy.succ; pA != NULL; pA = pA->succ) {
        count++;
    }
    return count;
}

/** Set intSetA to the intersection of intSetA and intSetB.  Return #
 *  of elements in the updated intSetA.  Returns < 0 on error.
 */
int intersectionIntSet(void *intSetA, void *intSetB) {
    //init stuff
    Header *headerA = (Header *)intSetA;
    Node *pA0 = &headerA->dummy;
    Header *headerB = (Header *)intSetB;
    Node *pB = headerB->dummy.succ;

    for(;pA0->succ != NULL && pB != NULL;) {
        if(pA0->succ->value < pB->value) {
            pA0->succ = unlinkNodeAfter(pA0);
        }
        else if(pA0->succ->value == pB->value) {
            pA0 = pA0->succ;
            pB = pB->succ;
        }
        else if(pA0->succ->value > pB->value) {
            pB = pB->succ;
        }
    }
    while(pA0->succ != NULL) {
        pA0->succ = unlinkNodeAfter(pA0);
        //if(pA0->succ) {
        //    pA0 = pA0->succ;
        //}
    }
    int count = 0;
    for(Node *pA = headerA->dummy.succ; pA != NULL; pA = pA->succ) {
        count++;
    }
    return count;
}

/** Free all resources used by previously created intSet. */
void freeIntSet(void *intSet) {
    Header *header = (Header *)intSet;
    Node *p1;
    for (Node *p = header->dummy.succ; p != NULL; p = p1) {
        p1 = p->succ;  //must complete accesses to p before freeing p
        free(p);
    }
    free(header);
}

/** Return a new iterator for intSet.  Returns NULL if intSet
 *  is empty.
 */
const void *newIntSetIterator(const void *intSet) {
    Header *header = (Header *)intSet;
    int empty = 1;
    Node *p;
    for (p = header->dummy.succ; p != NULL; p = p->succ) {
        empty = 0;
    }
    p = header->dummy.succ;
    return empty == 1 ? NULL : (const void *) p;
}

/** Return current element for intSetIterator. */
int intSetIteratorElement(const void *intSetIterator) {
    Node *iterator = (Node *)intSetIterator;
    return iterator->value;
}

/** Step intSetIterator and return stepped iterator.  Return
 *  NULL if no more iterations are possible.
 */
const void *stepIntSetIterator(const void *intSetIterator) {
    Node *iterator = (Node *)intSetIterator;
    return iterator->succ ? iterator->succ : NULL;
}
