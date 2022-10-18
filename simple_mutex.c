#define _GNU_SOURCE
#include "simple_mutex.h"
#include <linux/futex.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <stdint.h>
#include <stdatomic.h>
#include <errno.h>
#include <sys/wait.h>
#include <stdarg.h>
#include <stdlib.h>


#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE); \
                               } while (0)


static int futex(int *uaddr, int futex_op, uint32_t val,
	         const struct timespec *timeout, uint32_t *uaddr2, uint32_t val3){
  return syscall(SYS_futex, uaddr, futex_op, val,timeout, uaddr2, val3);
}

static void futex_wait(int *futexp){
  long s;

  /* atomic_compare_exchange_strong(ptr, oldval, newval)
              atomically performs the equivalent of:

                  if (*ptr == *oldval)
                      *ptr = newval;

              It returns true if the test yielded true and *ptr was updated. */

  while (1) {

               /* Is the futex available? */
               const int one = 1;
               if (atomic_compare_exchange_strong(futexp, &one, 0))
                   break;      /* Yes */

               /* Futex is not available; wait. */

               s = futex(futexp, FUTEX_WAIT, 0, NULL, NULL, 0);
               if (s == -1 && errno != EAGAIN)
                   errExit("futex-FUTEX_WAIT");
           }
       }

static void
       futex_wake(int *futexp)
       {
           long s;

           /* atomic_compare_exchange_strong() was described
              in comments above. */

           const int zero = 0;
           if (atomic_compare_exchange_strong(futexp, &zero, 1)) {
               s = futex(futexp, FUTEX_WAKE, 1, NULL, NULL, 0);
               if (s  == -1)
                   errExit("futex-FUTEX_WAKE");
           }
       }

void mutex_lock (int *mutex) {
  //printf("/////////////// %d\n", *mutex);
  int v;
  /* Bit 31 was clear, we got the mutex (the fastpath) */
  int preVal = __sync_lock_test_and_set (mutex, 0x80000000);
  if ((preVal & 0x80000000) == 0)
    return;
  //atomic_increment (mutex);
  //printf("before  %d\n", *mutex);
  __sync_fetch_and_add (mutex,1);
  //printf("----------- %d\n", *mutex);
  while (1) {
    //printf("****** %d\n", *mutex);
    int preVal = __sync_lock_test_and_set (mutex, 0x80000000);
    if ((preVal & 0x80000000) == 0){
      __sync_fetch_and_sub (mutex,1);
      return;
    }
    /* We have to waitFirst make sure the futex value we are monitoring is truly negative (locked). */
    v = *mutex;
    if (v >= 0)
      continue;
    futex_wait (mutex/*, v*/);
    //sys_futex(mutex_lock(mutex), FUTEX_WAIT, v, NULL);
    //__sync_fetch_and_or
    //__sync_fetch_and_add
    //__sync_fetch_and_or
    //__sync_fetch_and_sub
    //syscall(sys_futex);
    //__sync_add_and_fetch
    //perror("error calling futex");
    /*if(function){
      perror
      }*/
    
  }
}

void mutex_unlock (int *mutex) {
  /* Adding 0x80000000 to counter results in 0 if and 23 only if there are not other interested threads */
  if (__sync_fetch_and_add (mutex, 0x80000000))
    return;

  /* There are other threads waiting for this mutex, 28 wake one of them up. */
  futex_wake (mutex);
}
