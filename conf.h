#ifndef CONF_H
#define CONF_H

/*******************************************************************************
 *
 *   conf.h - Manifest Constants and Types for concurrent access to a
 *                   circular buffer modelling a message queue
 *
 *   Notes:          User defined according to application
 *
 *******************************************************************************/

/***( Manifest constants for user-defined queuing system  )********************/

#define BUFSIZE 8      /* number of slots in queues */
#define NUM_QUEUES 6   /* number of queues */
#define CLOUD_Q 0      /* queue 0: cloud process */
#define CONTROLLER_Q 1 /* queue 1: controller process */
#define APP_Q 2        /* queue 2: app process */
#define LIGHT_Q 3      /* queue 3: light sensor process */
#define TEMP_Q 4       /* queue 4: temperature sensor process */
#define WATT_Q 5       /* queue 5: wattmeter sensor process */

/***( User-defined message structure )*****************************************/

typedef struct {
  int signal;
  int value_int;
  float value_float;
} msg_t;

/***( User defined signals )***************************************************
 */

/* MSC On Off Light */

/* Signals to light sensor */
typedef enum { intensityRequest } TO_LIGHT_SENSOR;

/* Signals to light actuator */
typedef enum { turnOnLight, turnOffLight } TO_LIGHT_ACTUATOR;

typedef enum { turnOnAC, turnOffAC } TO_AIR_CONDITIONER;

typedef enum { turnOnH, turnOffH } TO_HEATER;

typedef enum { turnOnOutlet, turnOffOutlet } TO_OUTLET_ACTUATOR;

typedef enum { showReport, confirmRule, userDecisionRequest } TO_USER;

typedef enum {
  userDecision,
  confirmUpdate,
  appDecisionRequest,
  consumptionReport,
  updateRule
} TO_APP;

typedef enum { reportConsumptionRequest, consumptionRequest } TO_WATTMETER;

typedef enum {
  decisionRequest,
  appDecision,
  confirmChange,
  ruleUpdated,
  consumptionDevices
} TO_CLOUD;

typedef enum {
  movementDetected,
  lightIntensity,
  temperature,
  cloudConsumptionReq,
  reportConsumption,
  consumption,
  cloudDecision,
  changeParameter,
  timerOffLight,
  timerOnOL,
  timerOffOL
} TO_CONTROLLER;

typedef enum {
  sGetId
} TO_CENTRAL; /* Signals sent from process pLocal
                to process pCentral */

/***( User-defined EFSM states )********************

STATES

LIGHT_SENSOR
IdleL

TEMP_SENSOR
IdleT

WATT_SENSOR
IdleW

CLOUD_STATES
IdleCl
WaitConfirmRule
WaitReportCl

APP_STATES
IdleA
WaitConfirmUpdate
WaitUserDecision

CONTROLLER_STATES
IdleC
HighConsumption
WaitReportC
WaitConsumption
WaitIntensity

 ****************************/
typedef enum { IdleL } LIGHT_SENSOR;

typedef enum { IdleT } TEMP_SENSOR;

typedef enum { IdleW } WATT_SENSOR;

typedef enum { IdleCl, WaitConfirmRule, WaitReportCl } CLOUD_STATES;

typedef enum { IdleA, WaitConfirmUpdate, WaitUserDecision } APP_STATES;

typedef enum {
  IdleC,
  HighConsumption,
  WaitReportC,
  WaitConsumption,
  WaitIntensity
} CONTROLLER_STATES;

#endif