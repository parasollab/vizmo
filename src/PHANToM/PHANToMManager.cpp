#ifdef USE_PHANTOM

#include "PHANToMManager.h"

#include <iostream>
#include <cstdlib>

//////////////////////////////////////////////////////////////////////
//Define Phantom Manager singleton
PhantomManager g_phantomManager;
PhantomManager& GetPhantomManager(){ return g_phantomManager; }
//////////////////////////////////////////////////////////////////////

// PhantomManager
void exitHandler()
{
   hdStopScheduler();
   hdUnschedule(gSchedulerCallback);

   // Free the haptics device
   if (ghHD != HD_INVALID_HANDLE)
   {
      hdDisableDevice(ghHD);
      ghHD = HD_INVALID_HANDLE;
   }
}

/*******************************************************************************
  Client callback.
  Use this callback synchronously.
  Gets data, in a thread safe manner, that is constantly being modified by the
  haptics thread.
 *******************************************************************************/
#ifdef LINUX
HDCallbackCode
#else
HDCallbackCode HDCALLBACK
#endif
DeviceStateCallback(void *pUserData) {
   DeviceDisplayState *pDisplayState =
      static_cast<DeviceDisplayState *>(pUserData);

   hdGetDoublev(HD_CURRENT_POSITION, pDisplayState->position);
   hdGetDoublev(HD_CURRENT_FORCE, pDisplayState->force);
   hdGetDoublev(HD_CURRENT_GIMBAL_ANGLES, pDisplayState->rotation);
   hdGetDoublev(HD_CURRENT_VELOCITY, pDisplayState->velocity);

   // execute this only once.
   return HD_CALLBACK_DONE;
}

/*******************************************************************************
  Main callback that calculates and sets the force.
 *******************************************************************************/
#ifdef LINUX
HDCallbackCode
#else
HDCallbackCode HDCALLBACK
#endif
MainCallback(void *data) {
   HHD hHD = hdGetCurrentDevice();

   hdBeginFrame(hHD);


   hduVector3Dd pos;
   hdGetDoublev(HD_CURRENT_POSITION,pos);

   hduVector3Dd forceVeci;
   forceVeci[0] = 0;
   forceVeci[1] = 0;
   forceVeci[2] = 0;
   hdSetDoublev(HD_CURRENT_FORCE, forceVeci);

   if(GetPhantomManager().CDOn){
      if(GetPhantomManager().fpos.size() > 0){
         {
            if(GetPhantomManager().Collision > .5 && GetPhantomManager().validpos.size() > 0){

               hduVector3Dd forceVec;
               forceVec[0] = GetPhantomManager().phantomforce*(GetPhantomManager().validpos[0] - GetPhantomManager().fpos[0]);
               forceVec[1] = GetPhantomManager().phantomforce*(GetPhantomManager().validpos[1] - GetPhantomManager().fpos[1]);
               forceVec[2] = GetPhantomManager().phantomforce*(GetPhantomManager().validpos[2] - GetPhantomManager().fpos[2]);
               //TODO FIX
               //GetCamera()->ReverseTransform(forceVec[0],forceVec[1],forceVec[2]);
               double R = pow(pow(forceVec[0],2)+pow(forceVec[1],2)+pow(forceVec[2],2),.5);
               if(R < 1)
                  hdSetDoublev(HD_CURRENT_FORCE, forceVec);
               else{
                  forceVec[0]/=R;forceVec[1]/=R;forceVec[2]/=R;
                  hdSetDoublev(HD_CURRENT_FORCE, forceVec);
               }

            }


            if(fabs(GetPhantomManager().Collision) < .5){

               GetPhantomManager().validpos = GetPhantomManager().fpos;
            }
            GetPhantomManager().proceed = false;
         }
      }
   }

   hdEndFrame(hHD);

   HDErrorInfo error;
   if (HD_DEVICE_ERROR(error = hdGetError()))
   {
      hduPrintError(stderr, &error, "Error during scheduler callback");
      if (hduIsSchedulerError(&error))
      {
         return HD_CALLBACK_DONE;
      }
   }

   return HD_CALLBACK_CONTINUE;
}

/*******************************************************************************
  Schedules the force callback.
 *******************************************************************************/
void PhantomManager::ScheduleForceCallback()
{
   std::cout << "haptics callback" << std::endl;
   gSchedulerCallback = hdScheduleAsynchronous(
         MainCallback, 0, HD_DEFAULT_SCHEDULER_PRIORITY);

   HDErrorInfo error;
   if (HD_DEVICE_ERROR(error = hdGetError()))
   {
      hduPrintError(stderr, &error, "Failed to initialize haptic device");
      fprintf(stderr, "\nPress any key to quit.\n");
      getchar();
      exit(-1);
   }

}

/*********************************************************************************
  Constructor
 *********************************************************************************/
PhantomManager::PhantomManager(){
   CDOn = false;
   proceed = false;
   phantomforce = .5;
   Collision = -1;
   UseFeedback = true;
   initPhantom();
}

/*********************************************************************************
  Initialize Phantom Device
 *********************************************************************************/
void PhantomManager::initPhantom(){


   HDErrorInfo error;

   printf("initializing Phantom\n");

   atexit(exitHandler);

   // Initialize the device.  This needs to be called before any other
   // actions on the device are performed.
   ghHD = hdInitDevice(HD_DEFAULT_DEVICE);
   if (HD_DEVICE_ERROR(error = hdGetError()))
   {
      hduPrintError(stderr, &error, "Failed to initialize haptic device");
      fprintf(stderr, "\nPress any key to quit.\n");
      getchar();
      exit(-1);
   }

   printf("Found device %s\n",hdGetString(HD_DEVICE_MODEL_TYPE));

   // Create a haptic context for the device.  The haptic context maintains
   // the state that persists between frame intervals and is used for
   // haptic rendering.
   //ghHLRC = hlCreateContext(ghHD);
   //hlMakeCurrent(ghHLRC);

   hdEnable(HD_FORCE_OUTPUT);
   hdEnable(HD_MAX_FORCE_CLAMPING);

   hdStartScheduler();
   if (HD_DEVICE_ERROR(error = hdGetError()))
   {
      hduPrintError(stderr, &error, "Failed to start scheduler");
      fprintf(stderr, "\nPress any key to quit.\n");
      getchar();
      exit(-1);
   }


   // Get the workspace dimensions.
   HDdouble maxWorkspace[6];
   hdGetDoublev(HD_MAX_WORKSPACE_DIMENSIONS, maxWorkspace);

   // Low/left/back point of device workspace.
   hduVector3Dd LLB(maxWorkspace[0], maxWorkspace[1], maxWorkspace[2]);
   // Top/right/front point of device workspace.
   hduVector3Dd TRF(maxWorkspace[3], maxWorkspace[4], maxWorkspace[5]);


   ScheduleForceCallback();
}

/**************************************************************************
  Get Position of End-Effector
 **************************************************************************/
hduVector3Dd PhantomManager::getEndEffectorPosition(){
   DeviceDisplayState state;
   hdScheduleSynchronous(DeviceStateCallback, &state,
         HD_MIN_SCHEDULER_PRIORITY);
   return state.position;
}

/**************************************************************************
  Get Rotation
 **************************************************************************/
hduVector3Dd PhantomManager::getRotation(){
   DeviceDisplayState state;
   hdScheduleSynchronous(DeviceStateCallback, &state,
         HD_MIN_SCHEDULER_PRIORITY);
   return state.rotation;
}


/**************************************************************************
  Get Velocity of End-Effector
 **************************************************************************/
hduVector3Dd PhantomManager::getVelocity(){
   DeviceDisplayState state;
   hdScheduleSynchronous(DeviceStateCallback, &state,
         HD_MIN_SCHEDULER_PRIORITY);
   return state.velocity;
}

///////////////////////////////////////////////////////////////////////////
#endif
