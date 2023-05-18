#include "cloudFunctions.h"

CLOUD_STATES cloudIdle(msg_t *in_msg) {
  CLOUD_STATES next_state = IdleCl;
  msg_t out_msg; /* output message */

  switch (in_msg->signal) {
  case ruleUpdated:
    out_msg.signal = changeParameter;
    out_msg.value_int = in_msg->value_int;
    out_msg.value_float = in_msg->value_float;
    sendMessage(&(queue[CONTROLLER_Q]), out_msg);
    printf("\t--- Cloud sent signal: changeParameter(%d, %f) TO Controller\n",
           out_msg.value_int, out_msg.value_float);
    next_state = WaitConfirmRule;
    break;

  case makeReport:
    out_msg.signal = cloudConsumptionReq;
    sendMessage(&(queue[CONTROLLER_Q]), out_msg);
    printf("\t--- Cloud sent signal: cloudConsumptionReq TO Controller\n");
    next_state = WaitReportCl;
    break;

  /*
  case decisionRequest:
    out_msg.signal = appDecisionRequest;
    sendMessage(&(queue[APP_Q]), out_msg);
    printf("\t--- Cloud sent signal: appDecisionRequest TO App\n");
    next_state = IdleCl;
    break;
  

  case appDecision:
    out_msg.signal = cloudDecision;
    out_msg.value_int = in_msg->value_int;
    sendMessage(&(queue[CONTROLLER_Q]), out_msg);
    printf("\t--- Cloud sent signal: cloudDecision(%d) TO Controller\n",
           out_msg.value_int);
    next_state = IdleCl;
    break;
  */

  default:
    break;
  }

  fflush(stdout);
  return next_state;
}

CLOUD_STATES cloudWaitConfirmRule(msg_t *in_msg) {
  CLOUD_STATES next_state = IdleCl;
  msg_t out_msg; /* output message */

  switch (in_msg->signal) {
  case confirmChange:
    out_msg.signal = confirmUpdate;
    out_msg.value_int = in_msg->value_int;
    sendMessage(&(queue[APP_Q]), out_msg);
    printf("\t--- Cloud sent signal: confirmUpdate(%d) TO App\n",
           out_msg.value_int);
    next_state = IdleCl;
    break;

  default:
    break;
  }

  fflush(stdout);
  return next_state;
}

CLOUD_STATES cloudWaitReport(msg_t *in_msg) {
  CLOUD_STATES next_state = IdleCl;
  msg_t out_msg; /* output message */

  switch (in_msg->signal) {
  case consumptionDevices:
    out_msg.signal = consumptionReport;
    out_msg.value_int = in_msg->value_int;
    out_msg.value_float = in_msg->value_float;
    sendMessage(&(queue[APP_Q]), out_msg);
    printf("\t--- Cloud sent signal: consumptionReport(%f) TO App\n",
           out_msg.value_float);
    next_state = IdleCl;
    break;

  default:
    break;
  }

  fflush(stdout);
  return next_state;
}
