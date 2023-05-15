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
} ctrl_data_t;

typedef struct {
  int time_outlet_on;
  int time_outlet_off;
  int duration_light_on;
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

extern msgq_t queue[NUM_QUEUES]; /* declare queue as an array of
                                      message queues */

extern env_data_t env_data;

extern ctrl_data_t ctrl_data;

extern time_data_t time_data;

/***( Function prototypes )***********************************************/

void initiliseData(void);
void destroyData(void);
void initialiseQueues(void);
void destroyQueues(void);
void sendMessage(msgq_t *queue_ptr, msg_t msg);
msg_t receiveMessage(msgq_t *queue_ptr);

/* functions for env_data_t */
int getTime(void);
void setTime(int time);

/* functions for time_data_t */
int getTimeOutletOn(void);
void setTimeOutletOn(int time);
int getTimeOutletOff(void);
void setTimeOutletOff(int time);
int getDurationLightOn(void);
void setDurationLightOn(int time);

#endif