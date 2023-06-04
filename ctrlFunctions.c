#include "ctrlFunctions.h"

// Function for timer that turns off light
void *pTimerLight(void *arg) {
  int pid = *((int *)arg);
  TIMER_STATES state = IdleLT;
  msg_t out_msg;
  msg_t in_msg;

  printf("\t--- TimerLight[%d] init \n", pid);
  fflush(stdout);

  while (TRUE) {
    in_msg = receiveMessage(&(timer_q[pid]));
    switch (state) {
    case IdleLT:
      if (in_msg.signal == setTimer) {
        int seconds = getDurationLightOn();
        sleep(seconds);
        out_msg.signal = expiredTimer;
        sendMessage(&(timer_q[pid]), out_msg);
        printf("\t--- TimerLight[%d] sent signal: expiredTimer TO "
               "SELF\n",
               pid);
        state = TimerExpired;
      }
      break;

    case TimerExpired:
      if (in_msg.signal == resetTimer) {
        state = IdleLT;
      } //
      else if (in_msg.signal == expiredTimer) {
        out_msg.signal = timerOffLight;
        sendMessage(&(main_q[CONTROLLER_Q]), out_msg);
        printf("\t--- TimerLight[%d] sent signal: timerOffLight TO "
               "Controller\n",
               pid);
      }
      state = IdleLT;
      break;

    default:
      break;
    }
    fflush(stdout);
  }

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
        ctrl_data.min_temp = in_msg->value_float;
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
    sendMessage(&(main_q[CLOUD_Q]), out_msg);

    break;

  case temperature:
    next_state = IdleC;
    out_msg.value_int = in_msg->value_int;
    // Turn on AC
    if (in_msg->value_float > ctrl_data.max_temp) {
      out_msg.signal = turnOnAC;
      out_msg.value_float = ctrl_data.desired_temp;
      /* Send message to actuator AC */
      out_msg.signal = turnOffH;
      /* Send message to actuator H */
      printf("\t--- Controller sent signal: turnOnAC(%d)\n", out_msg.value_int);
      printf("\t--- Controller sent signal: turnOffH(%d)\n", out_msg.value_int);
    } // Turn on H
    else if (in_msg->value_float < ctrl_data.min_temp) {
      out_msg.signal = turnOnH;
      out_msg.value_float = ctrl_data.desired_temp;
      /* Send message to actuator H */
      out_msg.signal = turnOffAC;
      /* Send message to actuator AC */
      printf("\t--- Controller sent signal: turnOnH(%d)\n", out_msg.value_int);
      printf("\t--- Controller sent signal: turnOffAC(%d)\n",
             out_msg.value_int);
    } // Turn off both
    else {
      out_msg.signal = turnOffH;
      /* Send message to actuator H */
      out_msg.signal = turnOffAC;
      /* Send message to actuator AC */
      printf("\t--- Controller sent signal: turnOffH(%d)\n", out_msg.value_int);
      printf("\t--- Controller sent signal: turnOffAC(%d)\n",
             out_msg.value_int);
    }
    break;

  case movementDetected:
    out_msg.signal = intensityRequest;
    sendMessage(&(light_q[in_msg->value_int]), out_msg);
    printf(
        "\t--- Controller sent signal: intensityRequest TO LightSensor[%d]\n",
        in_msg->value_int);
    next_state = WaitIntensity;
    break;

  case cloudConsumptionReq:
    out_msg.signal = reportConsumptionRequest;
    // Send reportConsumptionRequest to all wattmeters
    for (int i = 0; i < NUM_WATT_S; i++) {
      sendMessage(&(watt_q[i]), out_msg);
      printf("\t--- Controller sent signal: reportConsumptionRequest TO "
             "Wattmeter[%d]\n",
             i);
    }
    ctrl_data.idx = 0;
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
    for (int i = 0; i < NUM_WATT_S; i++) {
      /* Send message to actuator OUTLET */
      printf("\t--- Controller sent signal: turnOnOutlet(%d)\n", i);
    }
    next_state = IdleC;
    break;

  case timerOffOL:
    out_msg.signal = consumptionRequest;
    for (int i = 0; i < NUM_WATT_S; i++) {
      sendMessage(&(watt_q[i]), out_msg);
      printf("\t--- Controller sent signal: consumptionRequest TO Wattmeter\n");
    }
    ctrl_data.idx = 0;
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
    ctrl_data.idx++;
    if (in_msg->value_float > ctrl_data.consump_th) {
      out_msg.signal = turnOffOutlet;
      out_msg.value_int = in_msg->value_int;
      printf("\t--- Controller sent signal: turnOffOutlet(%d)\n",
             out_msg.value_int);
    }

    if (ctrl_data.idx < NUM_WATT_S) {
      next_state = WaitConsumption;
    } else {
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
      printf("\t--- Controller sent signal: turnOnLight(%d)\n",
             in_msg->value_int);
      next_state = IdleC;

      // LAUNCH TIMER
      out_msg.signal = resetTimer;
      sendMessage(&(timer_q[in_msg->value_int]), out_msg);
      printf("\t--- Controller sent signal: resetTimer TO TimerLight[%d]\n",
             in_msg->value_int);

      out_msg.signal = setTimer;
      sendMessage(&(timer_q[in_msg->value_int]), out_msg);
      printf("\t--- Controller sent signal: setTimer TO TimerLight[%d]\n",
             in_msg->value_int);

    } // If light intensity is high
    else {
      out_msg.signal = turnOffLight;
      /* Send message to actuator OUTLET */
      printf("\t--- Controller sent signal: turnOffLight(%d)\n",
             in_msg->value_int);
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
  float averageConsumption = 0.0;
  int i;

  switch (in_msg->signal) {
  case reportConsumption:

    // TODO SET ValuesPerDevice(idx) := DeviceValue
    // it goes here
    ctrl_data.valuesPerDevice[ctrl_data.idx] = in_msg->value_float;
    ctrl_data.idx++;

    if (ctrl_data.idx >= NUM_WATT_S) {
      out_msg.signal = consumptionDevices;
      for (i = 0; i < NUM_WATT_S; i++) {
        averageConsumption += ctrl_data.valuesPerDevice[i];
      }
      averageConsumption /= NUM_WATT_S;
      out_msg.value_float = averageConsumption; // send average consumption
      sendMessage(&(main_q[CLOUD_Q]), out_msg);
      printf("\t--- Controller sent signal: consumptionDevices(%f) TO Cloud\n",
             out_msg.value_float);
      next_state = IdleC;
    } //
    else {
      next_state = WaitReportC;
    }

    break;
  default:
    break;
  }

  fflush(stdout);

  return next_state;
}