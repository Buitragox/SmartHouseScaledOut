#include "appFunctions.h"
#include "cloudFunctions.h"
#include "ctrlFunctions.h"
#include "msgLayer.h"
#include <stdio.h>
#include <stdlib.h>

/* Waits for the specified time to trigger daytime bound events */
void *pClockTrigger(void *arg) {
  int off_time, on_time, report_time, now;
  boolean event;
  msg_t out_msg;

  printf("\t--- ClockTrigger init\n");
  fflush(stdout);

  while (TRUE) {
    event = FALSE;
    now = getTime();
    off_time = getTimeOutletOff();
    on_time = getTimeOutletOn();
    report_time = getTimeMakeReport();

    // Trigger event to turn OFF outlets
    if (now == off_time) {
      out_msg.signal = timerOffOL;
      sendMessage(&(queue[CONTROLLER_Q]), out_msg);
      printf("\t--- ClockTrigger sent signal: timerOffOL TO "
             "Controller\n");
      sleep(1); // waste time
      event = TRUE;
    }
    // Trigger event to turn ON outlets
    else if (now == on_time) {
      out_msg.signal = timerOnOL;
      sendMessage(&(queue[CONTROLLER_Q]), out_msg);
      printf("\t--- ClockTrigger sent signal: timerOnOL TO "
             "Controller\n");
      sleep(1); // waste time
      event = TRUE;
    }
    // Trigger event to make report
    if (now == report_time) {
      out_msg.signal = makeReport;
      sendMessage(&(queue[CLOUD_Q]), out_msg);
      printf("\t--- ClockTrigger sent signal: makeReport TO "
             "Cloud\n");
      event = TRUE;
    }

    // If an event happened, add time
    if (event) {
      addTime(); // add +1 to time to avoid infinite loop
    }

    sleep(1);
  }
}

void *pLightSensor(void *arg) {
  int luxLevels[24] = {
      10,  // 12:00 AM (Midnight)
      10,  // 1:00 AM
      10,  // 2:00 AM
      10,  // 3:00 AM
      10,  // 4:00 AM
      10,  // 5:00 AM
      50,  // 6:00 AM
      100, // 7:00 AM
      200, // 8:00 AM
      300, // 9:00 AM
      500, // 10:00 AM
      700, // 11:00 AM
      800, // 12:00 PM (Noon)
      700, // 1:00 PM
      500, // 2:00 PM
      300, // 3:00 PM
      200, // 4:00 PM
      100, // 5:00 PM
      50,  // 6:00 PM
      30,  // 7:00 PM
      20,  // 8:00 PM
      10,  // 9:00 PM
      10,  // 10:00 PM
      10   // 11:00 PM
  };
  LIGHT_STATES state = IdleL;
  msg_t out_msg;
  msg_t in_msg;
  int now;
  printf("\t--- Light Sensor init\n");
  fflush(stdout);

  while (TRUE) {
    in_msg = receiveMessage(&(queue[LIGHT_Q]));
    switch (state) {
    case IdleL:
      if (in_msg.signal == intensityRequest) {
        now = getTime();
        out_msg.signal = lightIntensity;
        out_msg.value_float = luxLevels[now];
        sendMessage(&(queue[CONTROLLER_Q]), out_msg);
        printf("\t--- LightSensor sent signal: lightIntensity(%f) TO "
               "Controller\n",
               out_msg.value_float);
      }
      break;

    default:
      break;
    }
  }
}

void *pWattmeter(void *arg) {
  WATT_STATES state = IdleW;
  msg_t out_msg;
  msg_t in_msg;
  int watts;
  int MAX_WATT = 60;
  int MIN_WATT = 5;
  srand(time(NULL));

  printf("\t--- Wattmeter init\n");
  fflush(stdout);

  while (TRUE) {
    in_msg = receiveMessage(&(queue[WATT_Q]));
    switch (state) {
    case IdleW:
      watts = MIN_WATT + (rand() % (MAX_WATT - MIN_WATT));
      if (in_msg.signal == consumptionRequest) {
        out_msg.signal = consumption;
        out_msg.value_int = 0; // wattmeter id
        out_msg.value_float = watts;
        sendMessage(&(queue[CONTROLLER_Q]), out_msg);
        printf("\t--- Wattmeter sent signal: consumption(%d, %f) TO "
               "Controller\n",
               out_msg.value_int, out_msg.value_float);
      } //
      else if (in_msg.signal == reportConsumptionRequest) {
        out_msg.signal = reportConsumption;
        out_msg.value_int = 0; // wattmeter id
        out_msg.value_float = watts;
        sendMessage(&(queue[CONTROLLER_Q]), out_msg);
        printf("\t--- Wattmeter sent signal: reportConsumption(%d, %f) TO "
               "Controller\n",
               out_msg.value_int, out_msg.value_float);
      }
      break;

    default:
      break;
    }
  }
}

void *pController(void *arg) {
  CONTROLLER_STATES state = IdleC;
  msg_t in_msg; /* input message */

  printf("\t--- Controller init\n");
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

  return NULL;
}

void *pApp(void *arg) {
  APP_STATES state = IdleA;
  msg_t in_msg; /* input message */

  printf("\t--- App init\n");
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

    default:
      break;
    }
  }

  return NULL;
}

void *pCloud(void *arg) {
  CLOUD_STATES state = IdleCl;
  msg_t in_msg; /* input message */

  printf("\t--- Cloud init\n");
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
    printf("5. Set time\n");
    fflush(stdout);
    fflush(stdin);
    scanf("%d", &opt);

    switch (opt) {
    // CHANGE RULE
    case 1:
      printf("What rule do you want to change?\n");
      printf("1. Max Temperature\n");
      printf("2. Min Temperature\n");
      printf("3. Consumption\n");
      printf("4. Outlet set time on\n");
      printf("5. Outlet set time off\n");
      printf("6. Light Intensity\n");
      printf("7. Light Duration\n");
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
        break;

      case 2:
        printf("Enter the minimun value of the temperature: ");
        fflush(stdout);
        fflush(stdin);
        scanf("%f", &in_float);

        out_msg.signal = (int)updateRule;
        out_msg.value_int = (int)MinTempRule;
        out_msg.value_float = in_float;

        sendMessage(&(queue[APP_Q]), out_msg);
        break;

      case 3:
        printf("Enter the consumption threshold: ");
        fflush(stdout);
        fflush(stdin);
        scanf("%f", &in_float);

        out_msg.signal = (int)updateRule;
        out_msg.value_int = (int)ConsumptionTHRule;
        out_msg.value_float = in_float;

        sendMessage(&(queue[APP_Q]), out_msg);
        break;

      case 4:
        printf("Enter the turn on time: ");
        fflush(stdout);
        fflush(stdin);
        scanf("%f", &in_float);

        out_msg.signal = (int)updateRule;
        out_msg.value_int = (int)OutletTimeOnRule;
        out_msg.value_float = in_float;

        sendMessage(&(queue[APP_Q]), out_msg);
        break;

      case 5:
        printf("Enter the turn off time: ");
        fflush(stdout);
        fflush(stdin);
        scanf("%f", &in_float);

        out_msg.signal = (int)updateRule;
        out_msg.value_int = (int)OutletTimeOffRule;
        out_msg.value_float = in_float;

        sendMessage(&(queue[APP_Q]), out_msg);
        break;

      case 6:
        printf("Enter the Light Intensity threshold: ");
        fflush(stdout);
        fflush(stdin);
        scanf("%f", &in_float);

        out_msg.signal = (int)updateRule;
        out_msg.value_int = (int)LightIntensityTHRule;
        out_msg.value_float = in_float;

        sendMessage(&(queue[APP_Q]), out_msg);
        break;

      case 7:
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

    // Manually set time
    case 5:
      printf("Enter a time of day in hours (0-23): ");
      fflush(stdout);
      fflush(stdin);
      scanf("%d", &in_int);
      setTime(in_int);
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
  pthread_t clock_tid;
  pthread_t timer_tid;

  /* Create queues and initialise data */
  initialiseQueues();
  initiliseData();

  pthread_create(&env_tid, NULL, pUser, NULL);
  pthread_create(&controller_tid, NULL, pController, NULL);
  pthread_create(&app_tid, NULL, pApp, NULL);
  pthread_create(&cloud_tid, NULL, pCloud, NULL);
  pthread_create(&light_tid, NULL, pLightSensor, NULL);
  pthread_create(&watt_tid, NULL, pWattmeter, NULL);
  pthread_create(&timer_tid, NULL, pTimerLight, NULL);
  sleep(1); // give time for main processes to initialize
  pthread_create(&clock_tid, NULL, pClockTrigger, NULL);

  pthread_join(env_tid, NULL);
  pthread_join(controller_tid, NULL);
  pthread_join(app_tid, NULL);
  pthread_join(cloud_tid, NULL);
  pthread_join(light_tid, NULL);
  pthread_join(watt_tid, NULL);
  pthread_join(clock_tid, NULL);
  pthread_join(timer_tid, NULL);

  /* Destroy queues */

  destroyQueues();
  destroyData();

  return 0;
}
