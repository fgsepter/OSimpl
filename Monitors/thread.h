/*
 * thread.h -- public interface to thread library
 *
 * This file should be included by the thread library and by application
 * programs that use the thread library.
 */
#ifndef _THREAD_H
#define _THREAD_H

static const unsigned int STACK_SIZE=262144;  // size of each thread's stack

typedef void (*thread_startfunc_t) (void *);

class thread {
public:
    thread(thread_startfunc_t, void *); // create a new thread
    ~thread();

    void join();                        // wait for this thread to finish

    class impl;                         // defined by the thread library
    impl* impl_ptr;                     // used by the thread library

    /*
     * Disable the copy constructor and copy assignment operator.
     */
    thread(const thread&) = delete;
    thread& operator=(const thread&) = delete;

    /*
     * Move constructor and move assignment operator.
     */
    thread(thread&&);
    thread& operator=(thread&&);
};

#include "cpu.h"
#include "mutex.h"
#include "cv.h"
#include "semaphore.h"

#endif /* _THREAD_H */
