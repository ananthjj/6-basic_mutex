#define _GNU_SOURCE
#include "simple_mutex.h"
#include <linux/futex.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <stdio.h>

void mutex_lock(int *mutex)
{  
}

void mutex_unlock(int *mutex)
{
}
