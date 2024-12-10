/*
 *  Queueing/Scheduling Lab
 *
 *  Copyright (c) 2022 Peter Dinda, Branden Ghena
 *
 *  Original Queuesim tool is Copyright (c) 2005 Peter Dinda
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "context.h"
#include "event.h"


// Terminal control sequences for single stepping (paired, start and end)
// Bolds text with a brighter foreground
// See https://en.wikipedia.org/wiki/ANSI_escape_code#SGR_(Select_Graphic_Rendition)_parameters
#define ES "\e[1;92m\e[7m"
#define EE  "\e[0m"
#define EQS "\e[1;93m\e[7m"
#define EQE "\e[0m"
#define AQS "\e[1;94m\e[7m"
#define AQE "\e[0m"
#define RQS "\e[1;95m\e[7m"
#define RQE "\e[0m"
#define DES "\e[1;96m\e[7m"
#define DEE "\e[0m"


int main(int argc, char** argv) {

  // print help output
  if (argc < 3 || argc > 4 ) {
    fprintf(stderr, "queuesim schedspec eventfile [singlestep]\n");
    fprintf(stderr, "available schedulers: \n");
    sim_sched_list(stderr);
    fprintf(stderr, "environment:\n");
    fprintf(stderr, "  QUEUESIM_SEED      => random number seed [def: time(0)]\n");
    fprintf(stderr, "  QUEUESIM_QUANTUM   => scheduling quantum in seconds [def: 0.01]\n");
    exit(-1);
  }

  // get user input
  char* schedspec = argv[1];
  char* eventfile = argv[2];

  // assume singlestepping
  bool singlestep = (argc == 4);

  // check if an environment variable defined a random seed
  if (getenv("QUEUESIM_SEED")) {
    srand(atoi(getenv("QUEUESIM_SEED")));
  } else {
    srand(time(0));
  }

  double quantum = 0.01; // 10 ms
  if (getenv("QUEUESIM_QUANTUM")) {
    quantum = atof(getenv("QUEUESIM_QUANTUM"));
  }

  // setup the simulation
  sim_context_t context;
  if (sim_context_init(&context, schedspec, quantum)) {
    fprintf(stderr, "Unable to initialize simulation context (does %s exist?)\n", schedspec);
    exit(-1);
  }

  if (sim_context_load_events(&context, eventfile)) {
    fprintf(stderr, "Unable to load events from %s\n", eventfile);
    exit(-1);
  }

  if (sim_context_begin(&context)) {
    fprintf(stderr, "Unable to begin\n");
    exit(-1);
  }

  // run the simulation, continues until all events are completed
  uint64_t count = 0;
  while (1) {

    // print starting information if singlestepping
    if (singlestep) {
      fprintf(stderr, ES "==================== Event %lu =========================================\n" EE, count);
      fprintf(stderr, EQS "======Event Queue:\n" EQE);
      sim_event_queue_print(&context.event_queue, stderr);
      fprintf(stderr, RQS "======Realtime Job Queue:\n" RQE);
      sim_job_queue_print(&context.realtime_queue, stderr);
      fprintf(stderr, AQS "======Aperiodic Job Queue:\n" AQE);
      sim_job_queue_print(&context.aperiodic_queue, stderr);
    }

    // get next event, and finish if there is none
    sim_event_t* event = sim_context_get_next_event(&context);
    if (!event) {
      break;
    }

    // print event details if single stepping
    if (singlestep) {
      fprintf(stderr, DES "======Dispatching Event:\n" DEE);
      sim_event_print(event, stderr);
      fprintf(stderr, "\n");
    }

    // actually handle the event
    sim_context_dispatch_event(&context, event);

    // wait for user to hit enter if singlestepping
    if (singlestep) {
      char buf[1024];
      fprintf(stderr, "======Done. Hit Enter to continue");
      fflush(stderr);
      fflush(stdin);
      fgets(buf, 1024, stdin);
    }

    count++;
  }

  // print results
  fprintf(stderr, "%lu events processed\n", count);
  sim_context_print_stats(&context, stdout);

  // clean up and exit
  sim_context_deinit(&context);
  return 0;
}

