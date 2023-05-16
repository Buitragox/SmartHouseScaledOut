#ifndef APP_FUNCTIONS_H
#define APP_FUNCTIONS_H

#include "msgLayer.h"
#include <stdio.h>

/* Behavior of controller process in each state */
APP_STATES appIdle(msg_t *in_msg);
APP_STATES appWaitConfirmUpdate(msg_t *in_msg);
APP_STATES appWaitUserDecision(msg_t *in_msg);

#endif