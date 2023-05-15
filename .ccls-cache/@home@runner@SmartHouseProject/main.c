#include "msgLayer.h"
#include "stateFunctions.h"
#include <stdio.h>
#include <stdlib.h>

void *pController(void *arg) {
  CONTROLLER_STATES state = IdleC;
  msg_t in_msg;  /* input message */
  

  while (TRUE) {
    in_msg = receiveMessage(&(queue[CONTROLLER_Q]));
    switch (state) {
    case IdleC:
      //state = ctrlIdle(&in_msg);
      break;

    case WaitConsumption:
      //state = ctrlWaitConsumption(&in_msg);
      break;

    case HighConsumption:
     // state = ctrlHighConsumption(&in_msg);
      break;

    case WaitIntensity:
     //state = ctrlWaitIntensity(&in_msg);
      break;

    case WaitReportC:
     // state = ctrlWaitReport(&in_msg);
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
    printf("What rule do you want to change?\n");
    printf("1. Temperature\n");
    printf("2. Consumption\n");
    printf("3. Outlet set time on\n");
    printf("4. Outlet set time off\n");
    printf("5. Light Intensity\n");
    printf("6. Light Duration\n");
    printf("7. Quit\n");
    printf("Select one option: ");
    scanf("%d\n", &opt);

    switch (opt) {
      case 1:
        printf("Enter the maximum value of the temperature: ");
        scanf("%f\n", &in_float);

        out_msg.signal = (int) updateRule;
        out_msg.value_int = (int) MaxTempRule;
        out_msg.value_float = in_float;

        sendMessage(&(queue[APP_Q]), out_msg);
  
        printf("Enter the minimun value of the temperature: ");
        scanf("%f\n", &in_float);

        out_msg.signal = (int) updateRule;
        out_msg.value_int = (int) MinTempRule;
        out_msg.value_float = in_float;

        sendMessage(&(queue[APP_Q]), out_msg);
        break;
  
      case 2:
        printf("Enter the consumption threshold: ");
        scanf("%f\n", &in_float);

        out_msg.signal = (int) updateRule;
        out_msg.value_int = (int) ConsumptionTHRule;
        out_msg.value_float = in_float;
        
        sendMessage(&(queue[APP_Q]), out_msg);
        break;
  
      case 3:
        printf("Enter the turn on time: ");
        scanf("%f\n", &in_float);

        out_msg.signal = (int) updateRule;
        out_msg.value_int = (int) OutletTimeOnRule;
        out_msg.value_float = in_float;
        
        sendMessage(&(queue[APP_Q]), out_msg);
        break;

      case 4:
        printf("Enter the turn off time: ");
        scanf("%f\n", &in_float);

        out_msg.signal = (int) updateRule;
        out_msg.value_int = (int) OutletTimeOffRule;
        out_msg.value_float = in_float;
        
        sendMessage(&(queue[APP_Q]), out_msg);
        break;
  
      case 5:
        printf("Enter the Light Intensity threshold: ");
        scanf("%f\n", &in_float);

        out_msg.signal = (int) updateRule;
        out_msg.value_int = (int) LightIntensityTHRule;
        out_msg.value_float = in_float;
        
        sendMessage(&(queue[APP_Q]), out_msg);
        break;
  
      case 6:
        printf("Enter the turn on duration: ");
        scanf("%f\n", &in_float);

        out_msg.signal = (int) updateRule;
        out_msg.value_int = (int) LightDurationOnRule;
        out_msg.value_float = in_float;
        
        sendMessage(&(queue[APP_Q]), out_msg);
        break;
  
      case 7:
        exit ( 0 );
        break;
  
      default:
        break;
    }
  }
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

  printf("%d\n", (int) updateRule);
  /*
  pthread_create ( &customr_tid, NULL, pCustomer, NULL );
  pthread_create ( &hw_tid, NULL, pHardware, NULL );
  pthread_create ( &cntrllr_tid, NULL, pController, NULL );
  */

  /* Destroy queues */
  destroyQueues();
  destroyData();

  return 0;
}
