 #ifndef _H_vizList
 #define _H_vizList

 #include "vizBasics.h"

 // forward declaration of class List
 template <class T>
 class List;

 // ****************  Node ************
 // Generic node, can be added to a list
 // ************************************

 template <class T>
 class Node
 {
 public:
    friend class List<T>;
    Node (T*);
    ~Node();
    void SetNext(Node * node) { itsNext = node; }
    Node * GetNext() const;
    T * GetObject() const;
    void DelObject();

 private:
    T* itsObject;
    Node * itsNext;
 };

 // ****************  List ************
 // Generic list template
 // Works with any numbered object
 // ***********************************
 template <class T>
 class List
 {
 public:
    List();
    ~List();

    void     Iterate(void (T::*f)()const) const;
    T*       Find(ULONG& position, vizBool (T::*f)(T*), T* t);
    vizBool  IsItemPresent(T* t);
    T*       GetFirst() const;
    void     Insert(T *);
    void     Insert(T *,int pos);
    void     Insert(T *, vizBool (T::*f)(T*));
    vizBool  Remove(ULONG position);
    T*       operator[](unsigned long) const;
    void     Clear();
    void     DeleteObjects();
    unsigned long    GetCount() const { return itsCount; }
 private:
    Node<T> * pHead;
    Node<T> * pTail;
    unsigned long itsCount;
 };

#ifdef LINUX
#include "vizList.c"
#endif

#endif vizList

