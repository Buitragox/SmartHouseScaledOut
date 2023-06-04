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
      sendMessage(&(main_q[CONTROLLER_Q]), out_msg);
      printf("\t--- ClockTrigger sent signal: timerOffOL TO "
             "Controller\n");
      sleep(1); // waste time
      event = TRUE;
    }
    // Trigger event to turn ON outlets
    else if (now == on_time) {
      out_msg.signal = timerOnOL;
      sendMessage(&(main_q[CONTROLLER_Q]), out_msg);
      printf("\t--- ClockTrigger sent signal: timerOnOL TO "
             "Controller\n");
      sleep(1); // waste time
      event = TRUE;
    }
    // Trigger event to make report
    if (now == report_time) {
      out_msg.signal = makeReport;
      sendMessage(&(main_q[CLOUD_Q]), out_msg);
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
  int pid = *((int *)arg);

  // TODO: VERIFY LUX LEVELS
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
  printf("\t--- Light Sensor[%d] init \n", pid);
  fflush(stdout);

  while (TRUE) {
    in_msg = receiveMessage(&(light_q[pid]));
    switch (state) {
    case IdleL:
      if (in_msg.signal == intensityRequest) {
        now = getTime();
        out_msg.signal = lightIntensity;
        out_msg.value_int = pid; // Sensor sends its own id
        out_msg.value_float = luxLevels[now];
        sendMessage(&(main_q[CONTROLLER_Q]), out_msg);
        printf("\t--- LightSensor[%d] sent signal: lightIntensity(%f) TO "
               "Controller\n",
               pid, out_msg.value_float);
      }
      break;
    default:
      break;
    }
  }
}

void *pWattmeter(void *arg) {
  int pid = *((int *)arg);
  WATT_STATES state = IdleW;
  msg_t out_msg;
  msg_t in_msg;
  int watts;
  int MAX_WATT = 60;
  int MIN_WATT = 5;

  srand(time(NULL));

  printf("\t--- Wattmeter[%d] init \n", pid);
  fflush(stdout);

  while (TRUE) {
    in_msg = receiveMessage(&(watt_q[pid]));
    switch (state) {
    case IdleW:

      watts = MIN_WATT + (rand() % (MAX_WATT - MIN_WATT));

      if (in_msg.signal == consumptionRequest) {
        out_msg.signal = consumption;
        out_msg.value_int = pid; // wattmeter id
        out_msg.value_float = watts;
        sendMessage(&(main_q[CONTROLLER_Q]), out_msg);
        printf("\t--- Wattmeter[%d] sent signal: consumption(%d, %f) TO "
               "Controller\n",
               pid, out_msg.value_int, out_msg.value_float);
      } //
      else if (in_msg.signal == reportConsumptionRequest) {
        out_msg.signal = reportConsumption;
        out_msg.value_int = pid; // wattmeter id
        out_msg.value_float = watts;
        sendMessage(&(main_q[CONTROLLER_Q]), out_msg);
        printf("\t--- Wattmeter[%d] sent signal: reportConsumption(%d, %f) TO "
               "Controller\n",
               pid, out_msg.value_int, out_msg.value_float);
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
    in_msg = receiveMessage(&(main_q[CONTROLLER_Q]));
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
    in_msg = receiveMessage(&(main_q[APP_Q]));
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
    in_msg = receiveMessage(&(main_q[CLOUD_Q]));
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

    printf("> %d\n", opt);
    fflush(stdout);

    // printf("> %d\n", opt);
    // fflush(stdout);

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
      printf("> %d\n", opt);
      fflush(stdout);

      switch (opt) {
      case 1:
        printf("Enter the maximum value of the temperature: ");
        fflush(stdout);
        fflush(stdin);
        scanf("%f", &in_float);
        printf("> %f\n", in_float);
        fflush(stdout);

        out_msg.signal = (int)updateRule;
        out_msg.value_int = (int)MaxTempRule;
        out_msg.value_float = in_float;

        sendMessage(&(main_q[APP_Q]), out_msg);
        break;

      case 2:
        printf("Enter the minimun value of the temperature: ");
        fflush(stdout);
        fflush(stdin);
        scanf("%f", &in_float);
        printf("> %f\n", in_float);
        fflush(stdout);

        out_msg.signal = (int)updateRule;
        out_msg.value_int = (int)MinTempRule;
        out_msg.value_float = in_float;

        sendMessage(&(main_q[APP_Q]), out_msg);
        break;

      case 3:
        printf("Enter the consumption threshold: ");
        fflush(stdout);
        fflush(stdin);
        scanf("%f", &in_float);
        printf("> %f\n", in_float);
        fflush(stdout);

        out_msg.signal = (int)updateRule;
        out_msg.value_int = (int)ConsumptionTHRule;
        out_msg.value_float = in_float;

        sendMessage(&(main_q[APP_Q]), out_msg);
        break;

      case 4:
        printf("Enter the turn on time: ");
        fflush(stdout);
        fflush(stdin);
        scanf("%f", &in_float);
        printf("> %f\n", in_float);
        fflush(stdout);

        out_msg.signal = (int)updateRule;
        out_msg.value_int = (int)OutletTimeOnRule;
        out_msg.value_float = in_float;

        sendMessage(&(main_q[APP_Q]), out_msg);
        break;

      case 5:
        printf("Enter the turn off time: ");
        fflush(stdout);
        fflush(stdin);
        scanf("%f", &in_float);
        printf("> %f\n", in_float);
        fflush(stdout);

        out_msg.signal = (int)updateRule;
        out_msg.value_int = (int)OutletTimeOffRule;
        out_msg.value_float = in_float;

        sendMessage(&(main_q[APP_Q]), out_msg);
        break;

      case 6:
        printf("Enter the light intensity threshold: ");
        fflush(stdout);
        fflush(stdin);
        scanf("%f", &in_float);
        printf("> %f\n", in_float);
        fflush(stdout);

        out_msg.signal = (int)updateRule;
        out_msg.value_int = (int)LightIntensityTHRule;
        out_msg.value_float = in_float;

        sendMessage(&(main_q[APP_Q]), out_msg);
        break;

      case 7:
        printf("Enter the turn on duration: ");
        scanf("%f", &in_float);
        printf("> %f\n", in_float);
        fflush(stdout);

        out_msg.signal = (int)updateRule;
        out_msg.value_int = (int)LightDurationOnRule;
        out_msg.value_float = in_float;

        sendMessage(&(main_q[APP_Q]), out_msg);
        break;

      default:
        printf("Not a valid option\n");
        break;
      }
      break;

    // Make movement
    case 2:

      printf("\n\tSelect which movement sensor to activate:\n");
      printf("\t1. Deck\n");
      printf("\t2. Kitchen\n");
      printf("\t3. Entry\n");
      printf("\t4. Porch\n");
      printf("\t5. Hall\n");
      printf("\t6. Garage\n");
      fflush(stdout);
      fflush(stdin);
      scanf("%d", &opt);

      printf("> %d\n", opt);
      fflush(stdout);

      if (opt < 1 || opt > NUM_LIGHT_S) {
        printf("Not a valid option\n");
        break;
      }

      out_msg.signal = movementDetected;
      out_msg.value_int = opt - 1;
      sendMessage(&(main_q[CONTROLLER_Q]), out_msg);
      break;

    // Send temperature read
    case 3:
      printf("\n\tSelect which temperature sensor to activate:\n");
      printf("\t1. Living\n");
      printf("\t2. Bed Two\n");
      printf("\t3. Master Suite\n");
      fflush(stdout);
      fflush(stdin);
      scanf("%d", &opt);
      printf("> %d\n", opt);
      fflush(stdout);

      if (opt < 1 || opt > 3) {
        printf("Not a valid option\n");
        break;
      }

      printf("Enter a temperature in Celsius: ");
      fflush(stdout);
      fflush(stdin);
      scanf("%f", &in_float);
      printf("> %f\n", in_float);
      fflush(stdout);
      out_msg.signal = temperature;
      out_msg.value_int = opt - 1;
      out_msg.value_float = in_float;
      sendMessage(&(main_q[CONTROLLER_Q]), out_msg);
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
      printf("> %d\n", in_int);
      fflush(stdout);
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
  pthread_t clock_tid;

  pthread_t tid_light_array[NUM_LIGHT_S];
  pthread_t tid_watt_array[NUM_WATT_S];
  pthread_t tid_timer_array[NUM_LIGHT_S];

  int pid_light_array[NUM_LIGHT_S];
  int pid_watt_array[NUM_WATT_S];
  int pid_timer_array[NUM_LIGHT_S];

  /* Create queues and initialise data */
  initialiseQueues(main_q, NUM_QUEUES);
  initialiseQueues(light_q, NUM_LIGHT_S);
  initialiseQueues(timer_q, NUM_LIGHT_S);
  initialiseQueues(watt_q, NUM_WATT_S);
  initiliseData();

  // PTHREAD CREATES

  pthread_create(&env_tid, NULL, pUser, NULL);
  pthread_create(&controller_tid, NULL, pController, NULL);
  pthread_create(&app_tid, NULL, pApp, NULL);
  pthread_create(&cloud_tid, NULL, pCloud, NULL);

  for (int i = 0; i < NUM_LIGHT_S; i++) {
    pid_light_array[i] = i;
    pid_timer_array[i] = i;
    pthread_create(&(tid_light_array[i]), NULL, pLightSensor,
                   (void *)&pid_light_array[i]);
    pthread_create(&(tid_timer_array[i]), NULL, pTimerLight,
                   (void *)&pid_timer_array[i]);
  }

  for (int i = 0; i < NUM_WATT_S; i++) {
    pid_watt_array[i] = i;
    pthread_create(&(tid_watt_array[i]), NULL, pWattmeter,
                   (void *)&pid_watt_array[i]);
  }

  sleep(1); // give time for main processes to initialize
  pthread_create(&clock_tid, NULL, pClockTrigger, NULL);

  // PTHREAD JOINS

  pthread_join(env_tid, NULL);
  pthread_join(controller_tid, NULL);
  pthread_join(app_tid, NULL);
  pthread_join(cloud_tid, NULL);
  pthread_join(clock_tid, NULL);

  for (int i = 0; i < NUM_LIGHT_S; i++) {
    pthread_join(tid_light_array[i], NULL);
    pthread_join(tid_timer_array[i], NULL);
  }

  for (int i = 0; i < NUM_WATT_S; i++) {
    pthread_join(tid_watt_array[i], NULL);
  }

  /* Destroy queues */
  destroyQueues(main_q, NUM_QUEUES);
  destroyQueues(light_q, NUM_LIGHT_S);
  destroyQueues(timer_q, NUM_LIGHT_S);
  destroyQueues(watt_q, NUM_WATT_S);
  destroyData();

  return 0;
}
