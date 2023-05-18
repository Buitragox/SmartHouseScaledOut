#include "ctrlFunctions.h"

// Function for timer that turns off light
void *pTimerTurnOffLight(void *arg) {
  printf("\t--- TimerTurnOffLight init\n");
  fflush(stdout);
  msg_t out_msg; /* output message */
  int seconds = getDurationLightOn();

  sleep(seconds);

  out_msg.signal = timerOffLight;
  sendMessage(&(queue[CONTROLLER_Q]), out_msg);
  printf("\t--- TimerTurnOffLight sent signal: timerOffLight TO "
         "Controller\n");
  return NULL;
}

CONTROLLER_STATES ctrlIdle(msg_t *in_msg) {
  CONTROLLER_STATES next_state;
  msg_t out_msg; /* output message */

  switch (in_msg->signal) {
  case changeParameter:
    out_msg.signal = confirmChange;
    out_msg.value_int = FALSE; // Assume information is invalid, check later
    next_state = IdleC;

    switch (in_msg->value_int) {
    // Update max temperature
    case MaxTempRule:
      // Check for valid max temperature value
      if (in_msg->value_float > ctrl_data.min_temp) {
        out_msg.value_int = TRUE;
        ctrl_data.max_temp = in_msg->value_float;
      }
      break;

    // Update min temperature
    case MinTempRule:
      // Check for valid min temperature value
      if (in_msg->value_float < ctrl_data.max_temp) {
        out_msg.value_int = TRUE;
        ctrl_data.max_temp = in_msg->value_float;
      }
      break;

    case ConsumptionTHRule:
      // Consumption must be greater than 0
      if (in_msg->value_float > 0) {
        out_msg.value_int = TRUE;
        ctrl_data.consump_th = in_msg->value_float;
      }
      break;

    case OutletTimeOnRule:
      // Time to set on the outlet must be a valid hour
      if (in_msg->value_float >= 0 && in_msg->value_int < 24) {
        out_msg.value_int = TRUE;
        setTimeOutletOn((int)in_msg->value_float);
      }
      break;

    case OutletTimeOffRule:
      // Time to set on the outlet must be a valid hour
      if (in_msg->value_int >= 0 && in_msg->value_int < 24) {
        out_msg.value_int = TRUE;
        setTimeOutletOff((int)in_msg->value_float);
      }
      break;

    case LightIntensityTHRule:
      // Amount of lux must be greater than 0
      if (in_msg->value_float > 0) {
        out_msg.value_int = TRUE;
        ctrl_data.light_intensity_th = in_msg->value_float;
      }
      break;

    case LightDurationOnRule:
      // Amount of time (in seconds) must be greater than 0
      if (in_msg->value_float > 0) {
        out_msg.value_int = TRUE;
        setDurationLightOn((int)in_msg->value_float);
      }
      break;

    default:
      break;
    }
    printf("\t--- Controller sent signal: confirmChange(%d) TO Cloud \n",
           out_msg.value_int);
    fflush(stdout);
    sendMessage(&(queue[CLOUD_Q]), out_msg);

    break;

  case temperature:
    next_state = IdleC;
    if (in_msg->value_float > ctrl_data.max_temp) {
      out_msg.signal = turnOnAC;
      out_msg.value_float = ctrl_data.desired_temp;
      /* Send message to actuator AC */
      out_msg.signal = turnOffH;
      /* Send message to actuator H */
      printf("\t--- Controller sent signal turnOnAC\n");
      printf("\t--- Controller sent signal turnOffH\n");
    } //
    else if (in_msg->value_float < ctrl_data.min_temp) {
      out_msg.signal = turnOnH;
      out_msg.value_float = ctrl_data.desired_temp;
      /* Send message to actuator H */
      out_msg.signal = turnOffAC;
      /* Send message to actuator AC */
      printf("\t--- Controller sent signal: turnOnH\n");
      printf("\t--- Controller sent signal: turnOffAC\n");
    } else {
      out_msg.signal = turnOffH;
      /* Send message to actuator H */
      out_msg.signal = turnOffAC;
      /* Send message to actuator AC */
      printf("\t--- Controller sent signal: turnOffH\n");
      printf("\t--- Controller sent signal: turnOffAC\n");
    }
    break;

  case movementDetected:
    out_msg.signal = intensityRequest;
    sendMessage(&(queue[LIGHT_Q]), out_msg);
    printf("\t--- Controller sent signal: intensityRequest TO LightSensor\n");
    next_state = WaitIntensity;
    break;

  case cloudConsumptionReq:
    out_msg.signal = reportConsumptionRequest;
    sendMessage(&(queue[WATT_Q]), out_msg);
    printf("\t--- Controller sent signal: reportConsumptionRequest TO "
           "Wattmeter\n");
    next_state = WaitReportC;
    break;

  case timerOffLight:
    out_msg.signal = turnOffLight;
    /* Send message to actuator LIGHT */
    printf("\t--- Controller sent signal: turnOffLight\n");
    next_state = IdleC;
    break;

  case timerOnOL:
    out_msg.signal = turnOnOutlet;
    /* Send message to actuator OUTLET */
    printf("\t--- Controller sent signal: turnOnOutlet\n");
    next_state = IdleC;
    break;

  case timerOffOL:
    out_msg.signal = consumptionRequest;
    sendMessage(&(queue[WATT_Q]), out_msg);
    printf("\t--- Controller sent signal: consumptionRequest TO Wattmeter\n");
    next_state = WaitConsumption;
    break;

  default:
    break;
  }

  fflush(stdout);

  return next_state;
}

CONTROLLER_STATES ctrlWaitConsumption(msg_t *in_msg) {
  CONTROLLER_STATES next_state;
  msg_t out_msg; /* output message */

  switch (in_msg->signal) {
  case consumption:
    if (in_msg->value_float > ctrl_data.consump_th) {
      out_msg.signal = turnOffOutlet;
      printf("\t--- Controller sent signal: turnOffOutlet\n");
      // sendMessage(&(queue[CLOUD_Q]), out_msg);
      next_state = IdleC;
    }
    break;

  default:
    break;
  }

  fflush(stdout);

  return next_state;
}

CONTROLLER_STATES ctrlWaitIntensity(msg_t *in_msg) {
  CONTROLLER_STATES next_state;
  msg_t out_msg; /* output message */

  switch (in_msg->signal) {
  case lightIntensity:
    // If light is too low, turn on
    if (in_msg->value_float < ctrl_data.light_intensity_th) {
      out_msg.signal = turnOnLight;
      /* Send message to actuator LIGHT */
      printf("\t--- Controller sent signal: turnOnLight\n");
      next_state = IdleC;

      // LAUNCH TIMER
      // TODO: cancel running timer if necessary
      pthread_t timerOff_tid;
      pthread_create(&timerOff_tid, NULL, pTimerTurnOffLight, NULL);

    } // If light intensity is high
    else {
      out_msg.signal = turnOffLight;
      /* Send message to actuator OUTLET */
      printf("\t--- Controller sent signal: turnOffOutlet\n");
      next_state = IdleC;
    }
    break;

  default:
    break;
  }

  fflush(stdout);

  return next_state;
}

CONTROLLER_STATES ctrlWaitReport(msg_t *in_msg) {
  CONTROLLER_STATES next_state;
  msg_t out_msg; /* output message */

  switch (in_msg->signal) {
  case reportConsumption:
    out_msg.signal = consumptionDevices;
    out_msg.value_float = in_msg->value_float;
    next_state = IdleC;
    break;

  default:
    break;
  }

  fflush(stdout);

  return next_state;
}