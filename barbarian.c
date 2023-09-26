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

// Global ptr for shared memory object
struct Dungeon *dptr;

void sig_handler(int signum){
  //Copy minion health to barbarian attack
  dptr->barbarian.attack = dptr->enemy.health;
}
// Handle the semaphore
void sem_handler(int signum){
  sem_t * lever1 = sem_open(dungeon_lever_one, 0);
  sem_wait(lever1);
  sleep(TIME_TREASURE_AVAILABLE);
  sem_post(lever1);
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