#include "dungeon_info.h"
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <semaphore.h>

/* Map shared memory object */
struct Dungeon *dptr;

bool isUpper(char c){
  if ((65 <= c) && (c <= 90))
    return true;
  return false;
}

// Handle signal
void sig_handler(int signum){
  //decode Caesar cypher
  int shift = (int)dptr->barrier.spell[0] % 26; // get the shift using % 26
  int length = sizeof(dptr->wizard.spell) / sizeof(dptr->wizard.spell[0]); // get length of array
  for (int i=0; i < length; i++){ // handle every char in the array
    if (!isalpha(dptr->barrier.spell[i+1])) // if not a letter, leave it as is
      dptr->wizard.spell[i] = dptr->barrier.spell[i+1];
    else if (isUpper(dptr->barrier.spell[i+1]) && ((dptr->barrier.spell[i+1] - shift) < 65))
      dptr->wizard.spell[i] = dptr->barrier.spell[i+1] + 26 - shift;
    else if (!isUpper(dptr->barrier.spell[i+1]) && ((dptr->barrier.spell[i+1] - shift) < 97))
      dptr->wizard.spell[i] = dptr->barrier.spell[i+1] + 26 - shift;
    else
      dptr->wizard.spell[i] = dptr->barrier.spell[i+1] - shift;
  }
}

// Handle the semaphore
void sem_handler(int signum){
  sem_t* lever2 = sem_open(dungeon_lever_two, 0);
  sem_wait(lever2);
  sleep(TIME_TREASURE_AVAILABLE); // wait for the rouge to get the treasure
  sem_post(lever2);
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
    exit(-1); // Register signal handler and checking if it gets error
    
  struct sigaction sem;
  sem.sa_handler = &sem_handler;

  if(sigaction(SIGUSR2, &sem, NULL) == -1) // look for signal
    exit(-1); // Register signal handler and checking if it gets error

  while(1){} // Infinite loop so program doesn't end until told to
}