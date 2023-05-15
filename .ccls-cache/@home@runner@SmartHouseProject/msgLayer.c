/*******************************************************************************
 *
 *  phtrdsMsgLyr.c - Service routines for concurrent access to a circular buffer
 *                   modelling a message queue
 *
 *   Notes:          Error checking omitted...
 *
 *******************************************************************************/

#include "msgLayer.h"

/***( Global variables )**************************************************/

msgq_t queue[NUM_QUEUES]; /* declare queue as an array of
                              message queues */
env_data_t env_data;

ctrl_data_t ctrl_data;

time_data_t time_data;

/***( Support functions )*************************************************/

void initiliseData(void) {
  // env_data init
  env_data.hour = 0;
  pthread_mutex_init(&(env_data.env_lock), NULL);

  // ctrl_data init
  ctrl_data.min_temp = 22.0;
  ctrl_data.max_temp = 27.0;
  ctrl_data.desired_temp = (ctrl_data.max_temp + ctrl_data.min_temp) / 2;
  ctrl_data.light_intensity_th = 100.0;
  ctrl_data.consump_th = 30.0;

  // time_data init
  time_data.duration_light_on = 5;
  time_data.time_outlet_off = 23; // 11PM
  time_data.time_outlet_on = 6;   // 6AM
  pthread_mutex_init(&(time_data.time_lock), NULL);
}

void destroyData(void) {
  pthread_mutex_destroy(&(env_data.env_lock));
  pthread_mutex_destroy(&(time_data.time_lock));
}

/* initialise array of message queues  */
void initialiseQueues(void) {
  int i;

  for (i = 0; i < NUM_QUEUES; i++) {
    queue[i].bufin = 0;
    queue[i].bufout = 0;
    pthread_mutex_init(&(queue[i].buffer_lock), NULL);
    /* queue [i].buffer_lock = PTHREAD_MUTEX_INITIALIZER; */
    /* Create semaphores */
    sem_init(&(queue[i].items), LOCAL, 0);       /* There are no messages */
    sem_init(&(queue[i].slots), LOCAL, BUFSIZE); /* There are BUFSIZE slots */
  }
}

/* put message msg into queue (circular buffer) pointed to by queue_ptr */
static void PutMsg(msgq_t *queue_ptr, msg_t msg) {
  pthread_mutex_lock(&(queue_ptr->buffer_lock));
  queue_ptr->buffer[queue_ptr->bufin] = msg;
  queue_ptr->bufin = (queue_ptr->bufin + 1) % BUFSIZE;
  pthread_mutex_unlock(&(queue_ptr->buffer_lock));
}

/* return message msg from queue (circular buffer) pointed to by queue_ptr */
static msg_t GetMsg(msgq_t *queue_ptr) {
  msg_t msg;

  pthread_mutex_lock(&(queue_ptr->buffer_lock));
  msg = queue_ptr->buffer[queue_ptr->bufout];
  queue_ptr->bufout = (queue_ptr->bufout + 1) % BUFSIZE;
  pthread_mutex_unlock(&(queue_ptr->buffer_lock));

  return (msg);
}

/* destroy array of message queues  */
void destroyQueues(void) {
  int i;

  for (i = 0; i < NUM_QUEUES; i++) {
    /* Destroy mutex */
    pthread_mutex_destroy(&(queue[i].buffer_lock));
    /* Destroy semaphores */
    sem_destroy(&(queue[i].items));
    sem_destroy(&(queue[i].slots));
  }
}

/* emulate SDL Output operation */
void sendMessage(msgq_t *queue_ptr, msg_t msg) {
  sem_wait(&(queue_ptr->slots));
  PutMsg(queue_ptr, msg);
  sem_post(&(queue_ptr->items));
}

/* emulate SDL Input operation */
msg_t receiveMessage(msgq_t *queue_ptr) {
  msg_t msg;

  sem_wait(&(queue_ptr->items));
  msg = GetMsg(queue_ptr);
  sem_post(&(queue_ptr->slots));

  return (msg);
}

/* functions for env_data_t */
int getTime(void) {
  int time;
  pthread_mutex_lock(&(env_data.env_lock));
  time = env_data.hour;
  pthread_mutex_unlock(&(env_data.env_lock));
  return time;
}

void setTime(int time) {
  pthread_mutex_lock(&(env_data.env_lock));
  env_data.hour = time;
  pthread_mutex_unlock(&(env_data.env_lock));
}

/* functions for time_data_t */
int getTimeOutletOn(void) {
  int time;
  pthread_mutex_lock(&(time_data.time_lock));
  time = time_data.time_outlet_on;
  pthread_mutex_unlock(&(time_data.time_lock));
  return time;
}

void setTimeOutletOn(int time) {
  pthread_mutex_lock(&(time_data.time_lock));
  time_data.time_outlet_on = time;
  pthread_mutex_unlock(&(time_data.time_lock));
}

int getTimeOutletOff(void) {
  int time;
  pthread_mutex_lock(&(time_data.time_lock));
  time = time_data.time_outlet_off;
  pthread_mutex_unlock(&(time_data.time_lock));
  return time;
}

void setTimeOutletOff(int time) {
  pthread_mutex_lock(&(time_data.time_lock));
  time_data.time_outlet_off = time;
  pthread_mutex_unlock(&(time_data.time_lock));
}

int getDurationLightOn(void) {
  int time;
  pthread_mutex_lock(&(time_data.time_lock));
  time = time_data.duration_light_on;
  pthread_mutex_unlock(&(time_data.time_lock));
  return time;
}

void setDurationLightOn(int time) {
  pthread_mutex_lock(&(time_data.time_lock));
  time_data.duration_light_on = time;
  pthread_mutex_unlock(&(time_data.time_lock));
}
