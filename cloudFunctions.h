#ifndef CLOUD_FUNCTIONS_H
#define CLOUD_FUNCTIONS_H

#include "msgLayer.h"
#include <stdio.h>
#include <unistd.h>

/* Behavior of controller process in each state */
CLOUD_STATES cloudIdle(msg_t *in_msg);
CLOUD_STATES cloudWaitConfirmRule(msg_t *in_msg);
CLOUD_STATES cloudWaitReport(msg_t *in_msg);

#endif