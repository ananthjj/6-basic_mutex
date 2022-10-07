Assignment 6
============

**Due: October 17, 2022 11:59 PM US Central Time**. Push to GitHub to submit the assignment.

In this assignment you are going to implement a simple but fully functional mutex based on the code in Chapter 28 (Figure 28.10) of the book `Operating Systems: Three Easy Pieces`. You have to implement the functions in `simple_mutex.c` to:

* lock a mutex with the `mutex_lock` function,
* unlock a mutex with a `mutex_unlock` function.

You will need the manual page for the `futex` system call, as well as the manual page for `syscall` because there is no glibc wrapper for the futex system call.

A test harness is already written for you in `main.c`. A correct execution of the test program looks something like this:

```
daniel@daniel-vb:~/work/assignment-6/build$ ./main
Starting test 1 of 10
Test 1 passed!
Starting test 2 of 10
Test 2 passed!
Starting test 3 of 10
Test 3 passed!
Starting test 4 of 10
Test 4 passed!
Starting test 5 of 10
Test 5 passed!
Starting test 6 of 10
Test 6 passed!
Starting test 7 of 10
Test 7 passed!
Starting test 8 of 10
Test 8 passed!
Starting test 9 of 10
Test 9 passed!
Starting test 10 of 10
Test 10 passed!
```

## Tasks

* Implement the `mutex_lock` function. Use atomic builtins (or inline assembly) and bitwise operations where needed. Use the `futex` system call to put the calling process on a wait queue if necessary.
* Implement the `mutex_unlock` function.  Use atomic builting (or inline assembly) where needed. Use the `futex` system call to wake waiting threads only if there is at least one waiter.

## Evaluation

Your assignment will be graded according to the following criteria:

- **80 points** for a correct implementation of `mutex_lock`.
- **20 points** for a correct implementation of `mutex_unlock`.
- **10 points** for a correct implementation of the MCS spinlock described below.

## Bonus -- MCS spinlock

Futex-based locks are useful for applications with longer critical sections, where yielding the processor to allow other threads/processes is worth the extra cost of context switching. However, for short critical sections, the cost of context switching may be higher than simply spinning and waiting. A class of algorithms called spinlocks solve these concurrency problems without invoking the operating system and simply using atomic primitives.

For bonus points, you can implement the famous MCS spinlock, named after the authors Mellor-Crummey and Scott. Pseudocode for the algorithm is given in their [seminal paper](https://www.cs.rochester.edu/u/scott/papers/1991_TOCS_synch.pdf). There is also a nice pictorial depiction of the lock on [LWN](https://lwn.net/Articles/590243/).

This algorithm is conceptually quite simple. Each waiting thread waits by spinning on its own variable. When a lock-holder releases the lock, it toggles that local-spin variable for the next thread in a linked-list-based queue. This way, every lock and unlock operation does not cause lock state to *bounce* across all of the core-local caches in a multicore architecture. But managing this concurrent queue of waiting threads requires careful synchronization using atomic primitives.
