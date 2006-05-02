#ifndef _VIZABSTRACTOBJ_H
#define _VIZABSTRACTOBJ_H

#include "vizBasics.h"


 // **************** vizAbstractObj ************
 // Abstract base class of vizAbstractObjs
 class vizAbstractObj
 {
 public:
    // constructor and destructor
    vizAbstractObj(){itsObjectNumber = 1;}
    virtual ~vizAbstractObj(){}
    ULONG GetObjectNumber() const { return itsObjectNumber; }

 protected:
    ULONG itsObjectNumber;
 };

#endif 
