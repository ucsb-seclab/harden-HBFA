/** @file
  Signal handling.

  Copyright (c) 2020-2021, Intel Corporation. All rights reserved.<BR>
  
  SPDX-License-Identifier: BSD-2-Clause-Patent

  Obtained from pdclib project licensed under CC0.

**/

#ifndef _PDCLIB_SIGNAL_H
#define _PDCLIB_SIGNAL_H _PDCLIB_SIGNAL_H

#include "pdclib/_PDCLIB_internal.h"
#include <sys/types.h>

/* Signals ------------------------------------------------------------------ */

/* A word on signals, to the people using PDCLib in their OS projects.

   The definitions of the C standard leave about everything that *could* be
   useful to be "implementation defined". Without additional, non-standard
   arrangements, it is not possible to turn them into a useful tool.

   This example implementation chose to "not generate any of these signals,
   except as a result of explicit calls to the raise function", which is
   allowed by the standard but of course does nothing for the usefulness of
   <signal.h>.

   A useful signal handling would:
   1) make signal() a system call that registers the signal handler with the OS
   2) make raise() a system call triggering an OS signal to the running process
   3) make provisions that further signals of the same type are blocked until
      the signal handler returns (optional for SIGILL)
*/

/* These are the values used by Linux. */

/* Abnormal termination / abort() */
#define SIGABRT 6
/* Arithmetic exception / division by zero / overflow */
#define SIGFPE  8
/* Illegal instruction */
#define SIGILL  4
/* Interactive attention signal */
#define SIGINT  2
/* Invalid memory access */
#define SIGSEGV 11
/* Termination request */
#define SIGTERM 15

/* The following should be defined to pointer values that could NEVER point to
   a valid signal handler function. (They are used as special arguments to
   signal().) Again, these are the values used by Linux.
*/
#define SIG_DFL (void (*)( int ))0
#define SIG_ERR (void (*)( int ))-1
#define SIG_IGN (void (*)( int ))1

typedef _PDCLIB_sig_atomic_t sig_atomic_t;

/* Installs a signal handler "func" for the given signal.
   A signal handler is a function that takes an integer as argument (the signal
   number) and returns void.

   Note that a signal handler can do very little else than:
   1) assign a value to a static object of type "volatile sig_atomic_t",
   2) call signal() with the value of sig equal to the signal received,
   3) call _Exit(),
   4) call abort().
   Virtually everything else is undefind.

   The signal() function returns the previous installed signal handler, which
   at program start may be SIG_DFL or SIG_ILL. (This implementation uses
   SIG_DFL for all handlers.) If the request cannot be honored, SIG_ERR is
   returned and errno is set to an unspecified positive value.
*/
_PDCLIB_PUBLIC void ( *signal( int sig, void ( *func )( int ) ) )( int );

/* Raises the given signal (executing the registered signal handler with the
   given signal number as parameter).
   This implementation does not prevent further signals of the same time from
   occuring, but executes signal( sig, SIG_DFL ) before entering the signal
   handler (i.e., a second signal before the signal handler re-registers itself
   or SIG_IGN will end the program).
   Returns zero if successful, nonzero otherwise. */
_PDCLIB_PUBLIC int raise( int sig );

#define SIGBUS          10
#define SA_SIGINFO      0x0040

union sigval {
  int           sival_int;
  void          *sival_ptr;
};

typedef struct {
  int           si_signo;
  int           si_errno;
  int           si_code;
  union sigval  si_value;
  pid_t         si_pid;
  uid_t         si_uid;
  void          *si_addr;
  int           si_status;
  long          si_band;
  int           si_trapno;
  int           si_timerid;
  int           si_overrun;
  int           si_mqd;
} siginfo_t;

typedef UINT32 sigset_t;

struct sigaction {
  //   void    (*sa_handler)(int);
  void (*sa_sigaction)(int, siginfo_t*, void*);
  int       sa_flags;
  sigset_t  sa_mask;
};

/* The sigemptyset() function initialises the signal set pointed to by set,
   such that all signals defined in this document are excluded.
*/
int sigemptyset(sigset_t *mask);

/* Examine and change a signal action */
int sigaction(int signum, const struct sigaction *act, struct sigaction *oldact);

/* Send a signal to a process or a group of processes */
int kill(pid_t pid, int sig);

/* Extension hook for downstream projects that want to have non-standard
   extensions to standard headers.
*/
#ifdef _PDCLIB_EXTEND_SIGNAL_H
#include _PDCLIB_EXTEND_SIGNAL_H
#endif

#endif
