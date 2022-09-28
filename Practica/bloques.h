//Pau Bonet Alcover, Joan Martorell Ferriol, Zhuo Han Yang
#include <stdio.h>  //printf(), fprintf(), stderr, stdout, stdin
#include <fcntl.h> //O_WRONLY, O_CREAT, O_TRUNC
#include <sys/stat.h> //S_IRUSR, S_IWUSR
#include <stdlib.h>  //exit(), EXIT_SUCCESS, EXIT_FAILURE, atoi()
#include <unistd.h> // SEEK_SET, read(), write(), open(), close(), lseek()
#include <errno.h>  //errno
#include <string.h> // strerror()
#include "semaforo_mutex_posix.h"
 
#define BLOCKSIZE 1024 // bytes
//DEBUG para cada nivel
#define DEBUG1 0 //nivel 1
#define DEBUG2 0 //nivel 2 
#define DEBUG3 0 //nivel 3
#define DEBUG4 0 //nivel 4
#define DEBUG5 0 //nivel 5
#define DEBUG6 0 //nivel 6
#define DEBUG7 0 //nivel 7
#define DEBUG8 0 //nivel 8
#define DEBUG9 0 //nivel 9
#define DEBUG10 0 //nivel 10
#define DEBUG11 0 //nivel 11
#define DEBUG12 0 //nivel 12
#define DEBUG13 0 //nivel 13
 
void mi_waitSem();
void mi_signalSem();
int bmount(const char *camino);
int bumount();
int bwrite(unsigned int nbloque, const void *buf);
int bread(unsigned int nbloque, void *buf);


