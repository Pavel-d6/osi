#ifndef COMMON_H
#define COMMON_H

#define SHM_NAME  "/lab4_shm"
#define SEM_FULL  "/lab4_full"
#define SEM_EMPTY "/lab4_empty"

#define DATA_SIZE 8192

struct shared {
    int  state;            
    int  len;              
    char data[DATA_SIZE];
};

#endif