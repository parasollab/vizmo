#include <vizList.h>

 // Node Implementations...

 template <class T>
 Node<T>::Node(T* pOjbect):
 itsObject(pOjbect),
 itsNext(0)
 {}

 template <class T>
 Node<T>::~Node()
 {
    itsObject = 0;
    itsNext = 0;
 }

 // deletes the object stored inside the node.
 template <class T>
 void Node<T>::DelObject()
 {
    delete itsObject;
 }

 // Returns NULL if no next Node
 template <class T>
 Node<T> * Node<T>::GetNext() const
 {
       return itsNext;
 }

 template <class T>
 T * Node<T>::GetObject() const
 {
    if (itsObject)
       return itsObject;
    else
       throw NullNode();
 }

 // Implementations for Lists...
 template <class T>
 List<T>::List():
    pHead(0),
    pTail(0),
    itsCount(0)
    {}

 template <class T>
 List<T>::~List()
 {
    Clear();
 }

 template <class T>
 T*   List<T>::GetFirst() const
 {
    if (pHead)
       return pHead->itsObject;
    else
       return NULL;
 }

 template <class T>
 T*   List<T>::operator[](unsigned long offSet) const
 {
    Node<T>* pNode = pHead;

    if (!pHead)
       return NULL;

    if (offSet > itsCount)
       return NULL;

    for (unsigned long i=0;i<offSet; i++)
       pNode = pNode->itsNext;

   return   pNode->itsObject;
 }


 // find a given object in list based on function func
 template <class T>
 T*   List<T>::Find(ULONG& position, vizBool (T::*func)(T*), T* t )
 {
    Node<T> * pNode = 0;
    for (pNode = pHead, position = 0;
          pNode!=NULL;
          pNode = pNode->itsNext, position++)
    {
       if ( (pNode->itsObject->*func)(t))
          break;
    }
    if (pNode == NULL)
       return NULL;
    else
       return pNode->itsObject;
 }


 // find given object t
 template <class T>
 vizBool  List<T>::IsItemPresent(T* t )
 {
    Node<T> * pNode = 0;
    for (pNode = pHead; pNode!=NULL; pNode = pNode->itsNext)
    {
      if ( pNode->itsObject == t)
         return TRUE;
    }
    return FALSE;
 }


 // call function for every object in list
 template <class T>
 void List<T>::Iterate(void (T::*func)()const) const
 {
    if (!pHead)
       return;
    Node<T>* pNode = pHead;
    do
       (pNode->itsObject->*func)();
	 while (pNode = pNode->itsNext);
 }

 // insert object at end of list.
 template <class T>
 void List<T>::Insert(T* pObject)
 {
    Node<T> * pNode = new Node<T>(pObject);

    itsCount++;

    if (!pHead)
    {
       pHead = pTail = pNode;
       return;
    }

    // Append to last element
    pTail->itsNext = pNode;
    pTail = pNode;
 }
 template <class T>
 void List<T>::Insert(T* pObject, int pos)
 {
    Node<T> * pNode = new Node<T>(pObject);
    Node<T> *tNode ;
    int i;
    itsCount++;

    //printf("Inserting to pos %d\n",pos); 
    if (!pHead)
    {
       pHead = pTail = pNode;
       return;
    }
     tNode=pHead;
    for(i=0;i<pos;i++)
    {
      tNode= tNode->itsNext;  
    }
    // Append to last element
    if(tNode==pTail) pTail=pNode;
    pNode->itsNext = tNode->itsNext;
    tNode->itsNext=pNode;
    //printf("Inserted\n");
 }


 // insert object at first position where next node is greater than this node
 // as defined by a comparison function.
 template <class T>
 void List<T>::Insert(T* pObject, vizBool(T::*func)(T*) )
 {
    Node<T> * pNode = new Node<T>(pObject);
    Node<T> * pCur;
    ULONG i = 0;

    if (!pHead)
    {
       pHead = pTail = pNode;
       itsCount++;
       return;
    }

    // new element is first
    if( (pNode->itsObject->*func)(pHead->itsObject) ){
      pNode->itsNext = pHead;
      pHead = pNode;
      itsCount++;
      return;
    }

    // compare and insert at appropriate place.
    pCur = pHead;
    while(i++ < GetCount()-1){
      if( (pNode->itsObject->*func)(pCur->itsNext->itsObject) )
        break;
      pCur = pCur->itsNext;
    }
    pNode->itsNext = pCur->itsNext;
    pCur->itsNext = pNode;
    if(i == GetCount()-1) pTail = pNode;
    itsCount++;
        
 }

  // removes node at position, indexing starts at zero
  template <class T>
  vizBool List<T>::Remove(ULONG position)
  {
    ULONG    i = 0;
    Node<T> *pNode, *pNext;
    //printf(" List<T>::Remove at position %d", position);

    if( position > GetCount()-1 || pHead == NULL)
      return FALSE;
    
    // assign new head if first element removed.
    if(position==0){
	//printf("deleting head node!");
      pNode = pHead;
      pHead = pHead->itsNext;
      if(GetCount() == 1) pTail = pHead;
      delete pNode;
      itsCount--;
      return TRUE;
    }
    //printf("deleting intermediate node!");
    // count until position is reached.
    pNode = pHead;
    while(i+1 < position){
      pNode = pNode->itsNext;
      i++;
    }
    pNext = pNode->itsNext->itsNext;
    delete pNode->itsNext;
    pNode->itsNext = pNext;
    if(position == GetCount()-1) pTail = pNode;
    itsCount--;
    return TRUE;
  }

  // clears the list without deleting the objects pointed to
  template <class T>
  void List<T>::Clear()
  {  
     if (!pHead)
       return;
    Node<T>* pNode;
    long int goFlag = 0;
    do
    {   pNode = pHead;
        pHead = pHead->itsNext;
        goFlag = (long int)pNode->itsNext;
        delete pNode;
	}  while (goFlag);
    itsCount = 0;
   }

  // deletes all the objects pointed to and clears the list
  template <class T>
  void List<T>::DeleteObjects()
  {
     if (!pHead)
       return;
    Node<T>* pNode;
    long int goFlag = 0;

    do
    {  pNode = pHead;
       pHead = pHead->itsNext;
       goFlag = (long int)pNode->itsNext;
       pNode->DelObject();
       delete pNode;
       }  while (goFlag);
    itsCount = 0;
   }
