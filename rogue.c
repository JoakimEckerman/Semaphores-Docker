#include "dungeon_info.h"
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <semaphore.h>

// Map shared memory object
struct Dungeon *dptr;

void sig_handler(int signum){
  // Guess the float num
  dptr->rogue.pick = 50.0;
  float max = MAX_PICK_ANGLE;
  float min = 0.0;
  float guess = max/2.0f; // guess half of max.
  dptr->rogue.pick = guess;
  float range = max - min; // range = the numbers guess could be
  usleep(TIME_BETWEEN_ROGUE_TICKS); // wait for update (TIME_BETWEEN_ROGUE_TICKS) 
  while(dptr->trap.locked){ // while not solved.
    switch(dptr->trap.direction){
      case 'u': // guess too low, increase guess
        min = guess + LOCK_THRESHOLD; // set current guess + 1 as min value
        range = max - min; // change range accordingly
        guess += range/2.0f; // increase guess by half the range
        break;
      case 'd': // guess too high, decrease guess
        max = guess - LOCK_THRESHOLD; // set current guess - 1 as max value
        range = max - min; // change range accordingly
        guess -= range/2.0f; // decrease guess by half the range
        break;
      case '-': // guess is correct
        break;
    }
    dptr->rogue.pick = guess;
    // wait for update (TIME_BETWEEN_ROGUE_TICKS) 
    usleep(TIME_BETWEEN_ROGUE_TICKS);
  }
}

// Handle the semaphore
void sem_handler(int signum){
  for(int i = 0; i < 4; i++){ // loop for all indexes in treasure
    if(dptr->treasure[i] != '\0') // if treasure isn't \0 copy it to spoils
      dptr->spoils[i] = dptr->treasure[i];
    else // treasure is \0
      i--; // decrement i so we can try again for the same index
  }
}

int main(int argc, char *argv[]){
  // Open shared memory
  int fd = shm_open(dungeon_shm_name, O_EXCL | O_RDWR, S_IRUSR | S_IWUSR);
  if (fd == -1)
    exit(-1);

  // Map ptr to shared memory
  dptr = mmap(NULL, sizeof(struct Dungeon), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (dptr == MAP_FAILED)
    exit(-1);

  struct sigaction sa;
  sa.sa_handler = &sig_handler;

  if(sigaction(SIGUSR1, &sa, NULL) == -1) // look for signal 
    // Register signal handler and checking if it gets error
    exit(-1);

  // Semaphore work in pregress
  struct sigaction sem;
  sem.sa_handler = &sem_handler;

  if(sigaction(SIGUSR2, &sem, NULL) == -1) // look for signal
    // Register signal handler and checking if it gets error
    exit(-1);

  while(1){} // Infinite loop so program doesn't end until told to
}