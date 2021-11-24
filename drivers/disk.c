#include <stdint.h>
#include "disk.h"
#include "../libc/mem.h"
#include "../libc/string.h"
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fts.h>

// err.h contains various nonstandard BSD extensions, but they are
// very handy.
#include <err.h>

#include <pthread.h>
// Has to detect a disk
// Needs to detect how many bytes of storage it has
// Needs to detect other low level information about the disk

void routine() {

}

void detectdisk() {

}

void readdiskbytes() {

}

void lowlevelread() {

}

int main() {
    int num_threads = 1;

    // creates threads
    pthread_t *threads = calloc(num_threads, sizeof(pthread_t));
  for (int i = 0; i < num_threads; i++) {
    if (pthread_create(&threads[i], NULL, &worker, &ht) != 0) {
      err(1, "pthread_create() failed");
    }
  }
  
    //destroys threads
  for (int i = 0; i < num_threads; i++) {
    if (pthread_join(threads[i], NULL) != 0) {
      err(1, "pthread_join() failed");
    }
  }

  return 0;
}

