#include "dungeon_info.h"
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <semaphore.h>

// Global ptr for shared memory object
struct Dungeon *dptr;

int main(int argc, char *argv[]){
    /* Create shared memory object, set its size, and map it */
    int fd = shm_open(dungeon_shm_name, O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR); // create
    ftruncate(fd, 5*sizeof(struct Dungeon)); // set size
    dptr = mmap(NULL, sizeof(struct Dungeon), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0); // map

    pid_t game = getpid(); // Get the main game pid

    // Fork and get process pid for each class, if 0 (child process), exec the process
    pid_t barbarian = fork();
    if(barbarian == 0)
        execv("./barbarian", argv);

    pid_t wizard = fork();
    if(wizard == 0)
        execv("./wizard", argv);

    pid_t rogue = fork();
    if(rogue == 0)
        execv("./rogue", argv);
    
    // Open semaphores for both levers to be used by barbarian and wizard
    sem_t * lever1 = sem_open(dungeon_lever_one, O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR, 0);
    sem_t * lever2 = sem_open(dungeon_lever_two, O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR, 0);

    sleep(1); // wait for other processes to set up
    if(getpid() == game) // if original process (not fork)
        RunDungeon(wizard, rogue, barbarian); // run the dungeon
    // kill the other processes that are looping infinitely
    kill(barbarian, SIGKILL);
    kill(wizard, SIGKILL);
    kill(rogue, SIGKILL);
}