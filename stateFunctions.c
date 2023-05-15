#include "stateFunctions.h"

CONTROLLER_STATES ctrlIdle(msg_t *in_msg) {
  CONTROLLER_STATES next_state;
  msg_t out_msg; /* output message */

  switch (in_msg->signal) {
  case changeParameter:
    out_msg.signal = confirmChange;

    switch (in_msg->value_int) {
    // Update max temperature
    case MaxTempRule:
      // Check for invalid temperature value
      if (in_msg->value_float >= ctrl_data.min_temp) {
        out_msg.value_int = FALSE;
      } // If valid, update the values
      else {
        out_msg.value_int = TRUE;
        ctrl_data.max_temp = in_msg->value_float;
      }
      break;

    // Update min temperature
    case MinTempRule:
      // Check for invalid temperature value
      if (in_msg->value_float <= ctrl_data.max_temp) {
        out_msg.value_int = FALSE;
      } // If valid, update the values
      else {
        out_msg.value_int = TRUE;
        ctrl_data.max_temp = in_msg->value_float;
      }
      break;

    case ConsumptionTHRule:
      // Consumption must be greater than 0
      if (in_msg->value_float <= 0) {
        out_msg.value_int = FALSE;
      } //
      else {
        out_msg.value_int = TRUE;
        ctrl_data.consump_th = in_msg->value_float;
      }
      break;

    case OutletTimeOnRule:
      // Time to set on the outlet must be a valid hour
      if (in_msg->value_int < 0 || in_msg->value_int >= 24) {
        out_msg.value_int = FALSE;
      } //
      else {
        out_msg.value_int = TRUE;
        setTimeOutletOn(in_msg->value_int);
      }
      break;

    case OutletTimeOffRule:
      // Time to set on the outlet must be a valid hour
      if (in_msg->value_int < 0 || in_msg->value_int >= 24) {
        out_msg.value_int = FALSE;
      } //
      else {
        out_msg.value_int = TRUE;
        setTimeOutletOff(in_msg->value_int);
      }
      break;

    case LightIntensityTHRule:
      // Amount of lux must be greater than 0
      if (in_msg->value_float < 0) {
        out_msg.value_int = FALSE;
      } //
      else {
        out_msg.value_int = TRUE;
        ctrl_data.light_intensity_th = in_msg->value_float;
      }
      break;

    case LightDurationOnRule:
      // Amount of time (in seconds) must be greater than 0
      if (in_msg->value_float < 0) {
        out_msg.value_int = FALSE;
      } //
      else {
        out_msg.value_int = TRUE;
        ctrl_data.light_intensity_th = in_msg->value_float;
      }
      break;

    default:
      break;
    }

    break;

  default:
    break;
  }

  return next_state;
}
