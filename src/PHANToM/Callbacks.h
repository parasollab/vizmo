#ifndef CALL_BACKS_H_
#define CALL_BACKS_H_

#include <HD/hd.h>

namespace PHANToM {

/* Client callback.
 *
 * Use this callback synchronously.
 * Gets data, in a thread safe manner, that is constantly being modified by the
 * haptics thread.
 */
HDCallbackCode GetDeviceState(void* _data);

/* Main callback that calculates and sets the force.
*/
HDCallbackCode ForceFeedback(void* _data);

}

#endif
