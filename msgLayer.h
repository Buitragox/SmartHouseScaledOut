#ifndef MSG_LAYER_H
#define MSG_LAYER_H

/*******************************************************************************
 *
 *  phtrdsMsgLyr.h - Service routines for concurrent access to a circular buffer
 *                   modelling a message queue
 *
 *   Notes:          Users should modify the pMLusrConf.h file
 *
 *******************************************************************************/

#include "conf.h"
#include <pthread.h>
#include <semaphore.h>

/***( Manifest constants )************************************************/

#define LOCAL 0 /* semaphores will not be shared with other processes */

/***( User-defined types )************************************************/

typedef enum { FALSE, TRUE } boolean;

typedef struct {
  int hour;
  pthread_mutex_t env_lock; /* mutex for accesing environment data */
} env_data_t;

typedef struct {
  float max_temp;
  float min_temp;
  float desired_temp;
  float consump_th;
  float light_intensity_th;
  int idx;
  float valuesPerDevice[NUM_WATT_S];
} ctrl_data_t;

typedef struct {
  int time_outlet_on;
  int time_outlet_off;
  int duration_light_on;
  int time_make_report;
  pthread_mutex_t time_lock; /* mutex for accesing time data */
} time_data_t;

typedef struct /* message queue structure */
{
  msg_t buffer[BUFSIZE];       /* circular buffer */
  int bufin;                   /* nxt free slot ndx */
  int bufout;                  /* nxt msg slot */
  pthread_mutex_t buffer_lock; /* mutex for buffer */
  sem_t items;                 /* semaphore for items */
  sem_t slots;                 /* semaphore for slots */
} msgq_t;

/***( External variables )************************************************/

extern msgq_t main_q[NUM_QUEUES]; /* declare queue as an array of
                              message queues */

extern msgq_t light_q[NUM_LIGHT_S]; /* declare queue as an array of
                              message queues */

extern msgq_t timer_q[NUM_LIGHT_S]; /* declare queue as an array of
                              message queues */

extern msgq_t watt_q[NUM_WATT_S]; /* declare queue as an array of
                              message queues */

extern env_data_t env_data;

extern ctrl_data_t ctrl_data;

extern time_data_t time_data;

/***( Function prototypes )***********************************************/

void initiliseData(void);
void destroyData(void);
void initialiseQueues(msgq_t *queueArray, int size);
void destroyQueues(msgq_t *queueArray, int size);
void sendMessage(msgq_t *queue_ptr, msg_t msg);
msg_t receiveMessage(msgq_t *queue_ptr);

/* functions for env_data_t */
int getTime(void);
void setTime(int time);
void addTime(void);

/* functions for time_data_t */
int getTimeOutletOn(void);
void setTimeOutletOn(int time);
int getTimeOutletOff(void);
void setTimeOutletOff(int time);
int getTimeMakeReport(void);
void setTimeMakeReport(int time);
int getDurationLightOn(void);
void setDurationLightOn(int time);

#endif