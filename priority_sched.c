// Scheduler implementation for CS343

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "context.h"
#include "event.h"
#include "job.h"
#include "jobqueue.h"
#include "scheduler.h"

// Enable debugging for this scheduler? 1=True
// Be sure to rename this for each scheduler
#define DEBUG_PRIORITY_SCHED 1

#if DEBUG_PRIORITY_SCHED
#define DEBUG(fmt, args...) DEBUG_PRINT("priority_sched: " fmt, ##args)
#else
#define DEBUG(fmt, args...)
#endif
#define ERROR(fmt, args...) ERROR_PRINT("priority_sched: " fmt, ##args)
#define INFO(fmt, args...)  INFO_PRINT("priority_sched: " fmt, ##args)


// Struct definition that holds state for this scheduler
typedef struct sched_state {
  sim_sched_t* sim;
  bool busy;
  sim_job_t* curr_job_running;
  sim_event_t* curr_job_ending_event;
  double curr_job_start_time;
} sched_state_t;


// Initialization for this scheduler
static int init(void* state, sim_context_t* context) {
  sched_state_t* s = (sched_state_t*)state;

  // initially, nothing is scheduled
  s->busy = false;

  return 0;
}

int priority_comparer(sim_job_t* lhs, sim_job_t* rhs){

  // if lhs has a smaller priority, return -1
  if (lhs -> static_priority > rhs -> static_priority){
    return -1;
  }

  // if they both have the same priority and lhs arrived first, return -1
  else if (lhs->static_priority == rhs -> static_priority && lhs -> arrival_time < rhs -> arrival_time){
    return -1;
  }
  else if (lhs->static_priority == rhs -> static_priority && lhs -> arrival_time == rhs -> arrival_time && lhs -> remaining_size < rhs -> remaining_size){
    return -1;
  }
  // this is when rhs is smaller and had an earlier start time
  else{
    return 1;
  }

};


// Function called when an aperiodic job arrives
static sim_sched_acceptance_t aperiodic_job_arrival(void*          state,
                                                    sim_context_t* context,
                                                    double         current_time,
                                                    sim_job_t*     job) {

  sched_state_t* s = (sched_state_t*)state;

  DEBUG("Time[%lf] ARRIVAL, job %lu size %lf\n", current_time, job->id, job->size);

  // add the job to the queue of jobs at the end
   sim_job_queue_enqueue_in_order(&context-> aperiodic_queue,

                                    job,

                                    priority_comparer);

  // only start a new job if there is not one already running
  if (!s->busy) {
    DEBUG("starting new job %lu because we are idle\n", job->id);

    // create an event for when this job is done
    sim_event_t* event = sim_event_create(current_time + job->remaining_size,
                                          context,
                                          SIM_EVENT_JOB_DONE,
                                          job);

                                          
    s -> curr_job_start_time = current_time;

    s -> curr_job_running = job;

    s -> curr_job_ending_event = event;
    if (!event) {
      ERROR("failed to allocate event\n");
      return SIM_SCHED_REJECT;
    }

    // post the event
    sim_event_queue_post(&context->event_queue, event);
    s->busy = true;
  }
else{
    // get the priority of the current job running
    double curr_job_priority= s->curr_job_running->static_priority;
    double curr_job_size = s->curr_job_running->remaining_size; 
    double curr_job_remaining_size = curr_job_size - (current_time - s->curr_job_start_time); 

    // check to see if new job has higher priority
    if (job -> static_priority > curr_job_priority){
      DEBUG("Stopping current job %lu and switching to job %lu\n", s->curr_job_running->id, job->id);
      // if so
      // adjust the size of the job that was running
    sim_job_set_remaining_size(s->curr_job_running, curr_job_remaining_size);
      // remove the event of the current job running ending
      sim_event_queue_delete(&context->event_queue, s->curr_job_ending_event);

      // create an event for when the replacement job is done
      sim_event_t* event = sim_event_create(current_time + job->remaining_size,
                                            context,
                                            SIM_EVENT_JOB_DONE,
                                            job);

      s -> curr_job_running = job;
      s -> curr_job_ending_event = event;

      // post the event
      sim_event_queue_post(&context->event_queue, event);

      // update state variables to account for changing active job
      s -> curr_job_start_time = current_time;
      s -> curr_job_running = job;
    }
  }
  return SIM_SCHED_ACCEPT;
}


// Function called when a job is finished
static void job_done(void*          state,
                     sim_context_t* context,
                     double         current_time,
                     sim_job_t*     job) {

  sched_state_t* s = (sched_state_t*)state;

  DEBUG("TIME[%lf] DONE, job %lu\n", current_time, job->id);

  // remove the job from the job queue
  sim_job_queue_remove(&context->aperiodic_queue, job);
  s->busy = false;

  // mark the job as completed
  if (sim_job_complete(context, job)) {
    ERROR("failed to complete job\n");
    return;
  }

  // check if there is a next job at the front of the queue
  sim_job_t* next = sim_job_queue_peek(&context->aperiodic_queue);

  // if there is no job, we're done here
  if (!next) {
    DEBUG("no more jobs in queue\n");
    return;
  }

  // there is a job, so let's schedule it
  DEBUG("%lf switching to job %lu\n", current_time, next->id);
  sim_event_t* event = sim_event_create(current_time + next->remaining_size,
                                        context,
                                        SIM_EVENT_JOB_DONE,
                                        next);

  s -> curr_job_running = next;
  s -> curr_job_ending_event = event;
  if (!event) {
    ERROR("failed to allocate event\n");
    return;
  }

  // post the event
  sim_event_queue_post(&context->event_queue, event);
  s->busy = true;
}


// Function called when a timeslice expires
static void timer_interrupt(void*          state,
                            sim_context_t* context,
                            double         current_time) {
  // nothing to do in this scheduler
  DEBUG("ignoring timer interrupt\n");
}


/* Scheduler configuration */

// Map of the generic scheduler operations into specific function calls in this scheduler
// Each of these lines should be a function pointer to a function in this file
static sim_sched_ops_t ops = {
  .init = init,

  // Only aperiodic jobs will occur in this lab
  .periodic_job_arrival  = NULL,
  .sporadic_job_arrival  = NULL,
  .aperiodic_job_arrival = aperiodic_job_arrival,

  // job status calls
  .job_done        = job_done,
  .timer_interrupt = timer_interrupt,
};

// Register this scheduler with the simulation
// All functions with the `constructor` attribute run _before_ `main()` is called
// Note that the name of this function MUST be unique
__attribute__((constructor)) void priority_sched_init() {
  sched_state_t* my_state = malloc(sizeof(sched_state_t));
  if (!my_state) {
    ERROR("cannot allocate scheduler state\n");
    return;
  }
  memset(my_state, 0, sizeof(sched_state_t));

  // IMPORTANT: the string here is the name of this scheduler and MUST match the expected name
  my_state->sim = sim_sched_register("priority_sched", my_state, &ops);
}
