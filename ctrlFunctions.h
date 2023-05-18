#ifndef CTRL_FUNCTIONS_H
#define CTRL_FUNCTIONS_H

#include "msgLayer.h"
#include <stdio.h>
#include <unistd.h>

typedef enum {
  MaxTempRule,
  MinTempRule,
  ConsumptionTHRule,
  OutletTimeOnRule,
  OutletTimeOffRule,
  LightIntensityTHRule,
  LightDurationOnRule
} RULE_ID;

/* Behavior of controller process in each state */
CONTROLLER_STATES ctrlIdle(msg_t *in_msg);
CONTROLLER_STATES ctrlWaitConsumption(msg_t *in_msg);
// CONTROLLER_STATES ctrlHighConsumption(msg_t *in_msg);
CONTROLLER_STATES ctrlWaitIntensity(msg_t *in_msg);
CONTROLLER_STATES ctrlWaitReport(msg_t *in_msg);
void *pTimerTurnOffLight(void *arg);

#endif