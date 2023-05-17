#include "appFunctions.h"

APP_STATES appIdle(msg_t *in_msg) {
  APP_STATES next_state = IdleA;
  msg_t out_msg; /* output message */

  switch (in_msg->signal) {
  case updateRule:
    out_msg.signal = ruleUpdated;
    out_msg.value_int = in_msg->value_int;
    out_msg.value_float = in_msg->value_float;
    sendMessage(&(queue[CLOUD_Q]), out_msg);
    printf("\t--- App sent signal: ruleUpdated(%d, %f) TO Cloud\n",
           out_msg.value_int, out_msg.value_float);
    next_state = WaitConfirmUpdate;
    break;

  case appDecisionRequest:
    out_msg.signal = userDecisionRequest;
    /* Send message to user */
    printf("\t--- App sent signal: userDecisionRequest\n");
    printf("\t--- App waiting for USER INPUT\n");
    next_state = WaitUserDecision;

    // TODO: Set timer
    break;

  case consumptionReport:
    out_msg.signal = showReport;
    out_msg.value_float = in_msg->value_float;
    /* Send message to user */
    printf("\t--- App sent signal: showReport %f\n", out_msg.value_float);
    next_state = IdleA;
    break;

  default:
    break;
  }

  fflush(stdout);
  return next_state;
}

APP_STATES appWaitConfirmUpdate(msg_t *in_msg) {
  APP_STATES next_state = IdleA;
  msg_t out_msg; /* output message */

  switch (in_msg->signal) {
  case confirmUpdate:
    out_msg.signal = confirmRule;
    out_msg.value_int = in_msg->value_int;
    /* Send message to user */
    printf("\t--- App sent signal: confirmRule(%d) TO User\n",
           out_msg.value_int);
    next_state = IdleA;
    break;

  default:
    break;
  }

  fflush(stdout);
  return next_state;
}

APP_STATES appWaitUserDecision(msg_t *in_msg) {
  APP_STATES next_state = IdleA;
  msg_t out_msg; /* output message */

  switch (in_msg->signal) {
  case userDecision:
    out_msg.signal = appDecision;
    out_msg.value_int = in_msg->value_int;
    sendMessage(&(queue[CLOUD_Q]), out_msg);
    printf("\t--- App sent signal: appDecision(%d) TO Cloud\n",
           out_msg.value_int);
    // printf("\t--- App sent signal: confirmRule %d\n", out_msg.value_int);
    next_state = IdleA;
    break;

  default:
    break;
  }

  fflush(stdout);
  return next_state;
}