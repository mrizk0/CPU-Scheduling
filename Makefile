#
# Queueing/Scheduling Lab
#
# Copyright (c) 2022 Peter Dinda, Branden Ghena
#
# Original Queuesim tool is Copyright (c) 2005 Peter Dinda
#
#

## Configurations

# Build defines
CC = gcc
CFLAGS = -g -Wall -Wno-unused-variable -MMD -I ./ -I support/
LDFLAGS = 
BUILDDIR ?= _build/

#
# EDIT HERE
# You will add your schedulers to the following list
#
# List of scheduler source files
SCHED_SOURCES = \
	fifo_sched.c \
	sjf_sched.c \
	srpt_sched.c \
	priority_sched.c \
	rr_sched.c \
	stride_sched.c \

# List of library source files
CORE_LIB_SOURCES = \
	job.c 	\
	jobqueue.c \
	event.c	\
	eventqueue.c \
	scheduler.c \
	context.c \

# List of executable source files
EXEC_SOURCES = \
	queuesim.c

# Figure out which files we need to make
SOURCES = $(SCHED_SOURCES) $(CORE_LIB_SOURCES) $(EXEC_SOURCES)
CSOURCES = $(filter %.c,$(SOURCES))
OBJS = $(addprefix $(BUILDDIR), $(CSOURCES:.c=.o))
DEPS = $(addprefix $(BUILDDIR), $(CSOURCES:.c=.d))

# Directories make searches for prerequisites and targets
VPATH = ./ support/


## Rules

# Default make rule
.PHONY: all
all: $(OBJS) queuesim

# Make build directory
$(BUILDDIR):
	$(Q)mkdir -p $@

# Make object files
$(BUILDDIR)%.o: %.c | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Make the queuesim executable
queuesim: $(OBJS)
	$(CC) $(LDFLAGS) $^ -lm -o $@

# Clean rule
.PHONY: clean
clean:
	@rm -rf $(BUILDDIR)
	@rm -f queuesim

# Dependencies
# Include dependency rules for picking up header changes (by convention at bottom of makefile)
-include $(DEPS)
