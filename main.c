#include "appFunctions.h"
#include "cloudFunctions.h"
#include "ctrlFunctions.h"
#include "msgLayer.h"
#include <stdio.h>
#include <stdlib.h>

void *pOutletClock(void *arg) {
  int off_time, on_time, now;
  msg_t out_msg;
  while (TRUE) {
    now = getTime();
    off_time = getTimeOutletOff();
    on_time = getTimeOutletOn();
    if (now == off_time) {
      out_msg.signal = timerOffOL;
      sendMessage(&(queue[CONTROLLER_Q]), out_msg);
      addTime();
    } //
    else if (now == on_time) {
      out_msg.signal = timerOnOL;
      sendMessage(&(queue[CONTROLLER_Q]), out_msg);
      addTime();
    }
    sleep(2);
  }
}

void *pController(void *arg) {
  CONTROLLER_STATES state = IdleC;
  msg_t in_msg; /* input message */

  pthread_t clock_tid;
  pthread_create(&clock_tid, NULL, pOutletClock, NULL);

  printf("Controller init\n");
  fflush(stdout);

  while (TRUE) {
    in_msg = receiveMessage(&(queue[CONTROLLER_Q]));
    switch (state) {
    case IdleC:
      state = ctrlIdle(&in_msg);
      break;

    case WaitConsumption:
      state = ctrlWaitConsumption(&in_msg);
      break;

    case HighConsumption:
      state = ctrlHighConsumption(&in_msg);
      break;

    case WaitIntensity:
      state = ctrlWaitIntensity(&in_msg);
      break;

    case WaitReportC:
      state = ctrlWaitReport(&in_msg);
      break;

    default:
      break;
    }
  }

  pthread_join(clock_tid, NULL);

  return NULL;
}

void *pApp(void *arg) {
  APP_STATES state = IdleA;
  msg_t in_msg; /* input message */

  printf("App init\n");
  fflush(stdout);

  while (TRUE) {
    in_msg = receiveMessage(&(queue[APP_Q]));
    switch (state) {
    case IdleA:
      state = appIdle(&in_msg);
      break;

    case WaitConfirmUpdate:
      state = appWaitConfirmUpdate(&in_msg);
      break;

    case WaitUserDecision:
      state = appWaitUserDecision(&in_msg);
      break;

    default:
      break;
    }
  }

  return NULL;
}

void *pCloud(void *arg) {
  CLOUD_STATES state = IdleCl;
  msg_t in_msg; /* input message */

  printf("Cloud init\n");
  fflush(stdout);

  while (TRUE) {
    in_msg = receiveMessage(&(queue[CLOUD_Q]));
    switch (state) {
    case IdleCl:
      state = cloudIdle(&in_msg);
      break;

    case WaitConfirmRule:
      state = cloudWaitConfirmRule(&in_msg);
      break;

    case WaitReportCl:
      state = cloudWaitReport(&in_msg);
      break;

    default:
      break;
    }
  }

  return NULL;
}

void *pUser(void *arg) {
  int opt, in_int;
  float in_float;
  msg_t out_msg;

  while (TRUE) {
    printf("[%d:00] Choose an action:\n", getTime());
    printf("1. Change rule value\n");
    printf("2. Activate movement sensor\n");
    printf("3. Send temperature read\n");
    printf("4. Add 1 hour to clock\n");
    fflush(stdout);
    fflush(stdin);
    scanf("%d", &opt);

    switch (opt) {
    // CHANGE RULE
    case 1:
      printf("What rule do you want to change?\n");
      printf("1. Temperature\n");
      printf("2. Consumption\n");
      printf("3. Outlet set time on\n");
      printf("4. Outlet set time off\n");
      printf("5. Light Intensity\n");
      printf("6. Light Duration\n");
      printf("Select one option: ");
      fflush(stdout);
      fflush(stdin);
      scanf("%d", &opt);
      switch (opt) {
      case 1:
        printf("Enter the maximum value of the temperature: ");
        fflush(stdout);
        fflush(stdin);
        scanf("%f", &in_float);

        out_msg.signal = (int)updateRule;
        out_msg.value_int = (int)MaxTempRule;
        out_msg.value_float = in_float;

        sendMessage(&(queue[APP_Q]), out_msg);

        printf("Enter the minimun value of the temperature: ");
        fflush(stdout);
        fflush(stdin);
        scanf("%f", &in_float);

        out_msg.signal = (int)updateRule;
        out_msg.value_int = (int)MinTempRule;
        out_msg.value_float = in_float;

        sendMessage(&(queue[APP_Q]), out_msg);
        break;

      case 2:
        printf("Enter the consumption threshold: ");
        fflush(stdout);
        fflush(stdin);
        scanf("%f", &in_float);

        out_msg.signal = (int)updateRule;
        out_msg.value_int = (int)ConsumptionTHRule;
        out_msg.value_float = in_float;

        sendMessage(&(queue[APP_Q]), out_msg);
        break;

      case 3:
        printf("Enter the turn on time: ");
        fflush(stdout);
        fflush(stdin);
        scanf("%f", &in_float);

        out_msg.signal = (int)updateRule;
        out_msg.value_int = (int)OutletTimeOnRule;
        out_msg.value_float = in_float;

        sendMessage(&(queue[APP_Q]), out_msg);
        break;

      case 4:
        printf("Enter the turn off time: ");
        fflush(stdout);
        fflush(stdin);
        scanf("%f", &in_float);

        out_msg.signal = (int)updateRule;
        out_msg.value_int = (int)OutletTimeOffRule;
        out_msg.value_float = in_float;

        sendMessage(&(queue[APP_Q]), out_msg);
        break;

      case 5:
        printf("Enter the Light Intensity threshold: ");
        fflush(stdout);
        fflush(stdin);
        scanf("%f", &in_float);

        out_msg.signal = (int)updateRule;
        out_msg.value_int = (int)LightIntensityTHRule;
        out_msg.value_float = in_float;

        sendMessage(&(queue[APP_Q]), out_msg);
        break;

      case 6:
        printf("Enter the turn on duration: ");
        scanf("%f", &in_float);

        out_msg.signal = (int)updateRule;
        out_msg.value_int = (int)LightDurationOnRule;
        out_msg.value_float = in_float;

        sendMessage(&(queue[APP_Q]), out_msg);
        break;

      default:
        printf("Not a valid option\n");
        break;
      }
      break;

    // Make movement
    case 2:
      out_msg.signal = movementDetected;
      sendMessage(&(queue[CONTROLLER_Q]), out_msg);
      break;

    // Send temperature read
    case 3:
      printf("Enter a temperature in Celsius: ");
      fflush(stdout);
      fflush(stdin);
      scanf("%f", &in_float);
      out_msg.signal = temperature;
      out_msg.value_float = in_float;
      sendMessage(&(queue[CONTROLLER_Q]), out_msg);
      break;

    // +1 to hours
    case 4:
      addTime();
      break;

    default:
      break;
    }
  }
  return NULL;
}

int main(void) {
  pthread_t env_tid;        /* Environment tid */
  pthread_t controller_tid; /* Controller tid */
  pthread_t app_tid;        /* App tid */
  pthread_t cloud_tid;      /* Cloud tid */
  pthread_t light_tid;      /* Lights tid */
  pthread_t temp_tid;       /* Temperature tid */
  pthread_t watt_tid;       /* Wattmeter tid */

  /* Create queues and initialise data */
  initialiseQueues();
  initiliseData();

  pthread_create(&env_tid, NULL, pUser, NULL);
  pthread_create(&controller_tid, NULL, pController, NULL);
  pthread_create(&app_tid, NULL, pApp, NULL);
  pthread_create(&cloud_tid, NULL, pCloud, NULL);
  /*
  pthread_create ( &hw_tid, NULL, pHardware, NULL );
  pthread_create ( &cntrllr_tid, NULL, pController, NULL );
  */

  pthread_join(env_tid, NULL);
  pthread_join(controller_tid, NULL);
  pthread_join(app_tid, NULL);
  pthread_join(cloud_tid, NULL);

  /* Destroy queues */

  destroyQueues();
  destroyData();

  return 0;
}
