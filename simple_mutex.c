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

//Sources used: https://pages.cs.wisc.edu/~remzi/OSTEP/threads-locks.pdf, https://gcc.gnu.org/onlinedocs/gcc/_005f_005fsync-Builtins.html, https://man7.org/linux/man-pages/man2/futex.2.html


#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE); \
                               } while (0)


static int futex(int *uaddr, int futex_op, int val,
	         const struct timespec *timeout, int *uaddr2, int val3){
  return syscall(SYS_futex, uaddr, futex_op, val,timeout, uaddr2, val3);
}

static void futex_wait(int *futexp){
  long s;

  while (1) {
               /* Is the futex available? */
               const int one = 1;
               if (__sync_bool_compare_and_swap(futexp, &one, 0)){
                   break;     
	       }

               s = futex(futexp, FUTEX_WAIT, 0, NULL, NULL, 0);
               if (s == -1 && errno != EAGAIN){
                   errExit("futex-FUTEX_WAIT");
	       }
           }
       }

static void
       futex_wake(int *futexp)
       {
           long s;

           const int zero = 0;
           if (__sync_bool_compare_and_swap(futexp, &zero, 1)) {
               s = futex(futexp, FUTEX_WAKE, 1, NULL, NULL, 0);
               if (s  == -1)
                   errExit("futex-FUTEX_WAKE");
           }
       }

void mutex_lock (int *mutex) {
  int v;
  /* Bit 31 was clear, we got the mutex (the fastpath) */
  int preVal = __sync_lock_test_and_set(mutex, 0x80000000);
  __sync_lock_release(mutex);
  if (!(__sync_and_and_fetch (&preVal, 0x80000000)))
    return;
  __sync_fetch_and_add (mutex,1);
  while (1) {
    int preVal = __sync_lock_test_and_set(mutex, 0x80000000);
    __sync_lock_release(mutex);
    if (!(__sync_and_and_fetch (&preVal, 0x80000000))){
      __sync_fetch_and_sub (mutex,1);
      return;
    }
    /* We have to waitFirst make sure the futex value we are monitoring is truly negative (locked). */
    v = *mutex;
    if (v >= 0)
      continue;
    futex_wait (mutex);   
  }
}

void mutex_unlock (int *mutex) {
  /* Adding 0x80000000 to counter results in 0 if and 23 only if there are not other interested threads */
  if (__sync_add_and_fetch (mutex, 0x80000000))
    return;

  /* There are other threads waiting for this mutex, 28 wake one of them up. */
  futex_wake (mutex);
}

struct qnode{
  struct qnode* next;
  int locked;
};

void acquire_lock(struct qnode** L, struct qnode* I){
  I->next = NULL;
  struct qnode* predecessor = __sync_lock_test_and_set(L, I);
  do{
    if (predecessor != NULL){
      I->locked = 1;
      predecessor->next = I;
    }
  } while (I->locked);
}

void release_lock(struct qnode** L, struct qnode* I){
  while (I->next == NULL){
    if(__sync_bool_compare_and_swap(L, I, NULL))
       return;
    I->next->locked = 0;
  }
}
  
