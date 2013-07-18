#ifndef PHANTOM_H_
#define PHANTOM_H_

#ifdef USE_PHANTOM

#include <HD/hd.h>
#include <HDU/hduError.h>
#include <HDU/hduVector.h>
//#include <HL/hl.h>
//#include <HDU/hduMath.h>
//#include <HDU/hduMatrix.h>
//#include <HDU/hduQuaternion.h>
//#include <HDU/hduError.h>
//#include <HLU/hlu.h>

//Define singleton
class PhantomManager;
PhantomManager& GetPhantomManager();

//static HHLRC ghHLRC = NULL;
static HHD ghHD = HD_INVALID_HANDLE;
static HDSchedulerHandle gSchedulerCallback = HD_INVALID_HANDLE;

/* Haptic device record. */
struct DeviceDisplayState
{
   HHD m_hHD;
   hduVector3Dd position;
   hduVector3Dd velocity;
   hduVector3Dd force;
   hduVector3Dd rotation;
};

void exitHandler();

/*******************************************************************************
  Client callback.
  Use this callback synchronously.
  Gets data, in a thread safe manner, that is constantly being modified by the
  haptics thread.
 *******************************************************************************/
HDCallbackCode HDCALLBACK DeviceStateCallback(void *pUserData);

/*******************************************************************************
  Main callback that calculates and sets the force.
 *******************************************************************************/
HDCallbackCode HDCALLBACK MainCallback(void *data);

class PhantomManager
{
   /*******************************************************************************
     Schedules the force callback.
    *******************************************************************************/
   void ScheduleForceCallback();

   public:
   /*******************************************************************************
     Previous Valid Configuration Data
    *******************************************************************************/
   vector<double> validpos;

   /*******************************************************************************
     Potential Valid Configuration Data
    *******************************************************************************/
   vector<double> fpos;

   /*******************************************************************************
     Collision
    *******************************************************************************/
   int Collision;

   /*******************************************************************************
     Use Feedback?
    *******************************************************************************/
   bool UseFeedback;

   /*******************************************************************************
     CDOn
    *******************************************************************************/
   bool CDOn;

   /*******************************************************************************
     proceed
    *******************************************************************************/
   bool proceed;

   /*******************************************************************************
     Phantom Force Factor
    *******************************************************************************/
   double phantomforce;

   /*******************************************************************************
     Constructor
    *******************************************************************************/
   PhantomManager();

   /*******************************************************************************
     Initialize Phantom Device
    *******************************************************************************/
   void initPhantom();

   /*******************************************************************************
     Get Position of End-Effector
    *******************************************************************************/
   hduVector3Dd getEndEffectorPosition();

   /*******************************************************************************
     Get Roatation
    *******************************************************************************/
   hduVector3Dd getRotation();

   /*******************************************************************************
     Get Velocity of End-Effector
    *******************************************************************************/
   hduVector3Dd getVelocity();

};

/////////////////////////////////////////////////////////////////////////////////

#endif
#endif
