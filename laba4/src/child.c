#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/mman.h>

#include "common.h"

static struct shared *sh;
static sem_t *sem_full_p, *sem_empty_p;

static char out_buf[DATA_SIZE];   
static int  out_len = 0;

void perror_hand(const char *msg){
    const char* e = strerror(errno);
    write(2, msg, strlen(msg));
    write(2, ": ", 2);
    write(2, e, strlen(e));
    write(2, "\n", 1);
    exit(1);
}

void send_block(const char *buf, int n){
    while (sem_wait(sem_empty_p) == -1){      
        if (errno == EINTR) continue;
        perror_hand("sem_wait empty");
    }
    memcpy(sh->data, buf, n);
    sh->len = n;
    sh->state = 1;
    if (sem_post(sem_full_p) == -1) perror_hand("sem_post full"); 
}

void flush_out(void){
    if (out_len > 0){
        send_block(out_buf, out_len);
        out_len = 0;
    }
}

void out_write(const char *buf, int n){
    if (out_len + n > DATA_SIZE) flush_out();
    memcpy(out_buf + out_len, buf, n);
    out_len += n;
}

void print_num(long long v){
    char out[32];
    int i = sizeof(out);
    unsigned long long u = v < 0 ? -(unsigned long long)v : (unsigned long long)v;

    out[--i] = '\n';

    do{
        out[--i] = u%10 + '0';
        u = u / 10;
    }while (u > 0);

    if (v < 0){out[--i] = '-';}

    out_write(out + i , sizeof(out) - i);
}


int main(void){
    int sfd = shm_open(SHM_NAME, O_RDWR, 0);
    if (sfd == -1) perror_hand("shm_open");
    sh = mmap(NULL, sizeof(struct shared), PROT_READ | PROT_WRITE, MAP_SHARED, sfd, 0);
    if (sh == MAP_FAILED) perror_hand("mmap");
    close(sfd);

    sem_full_p = sem_open(SEM_FULL, 0);
    if (sem_full_p == SEM_FAILED) perror_hand("sem_open full");
    sem_empty_p = sem_open(SEM_EMPTY, 0);
    if (sem_empty_p == SEM_FAILED) perror_hand("sem_open empty");

    char buf[4096];

    long long sum = 0, cur = 0;
    int in_num = 0, neg = 0, dirty = 0;

    ssize_t n;

    while((n = read(0, buf, sizeof(buf))) != 0){
        if (n == -1){
            if (errno == EINTR) continue;
            perror_hand("read stdin");
        }
        for (ssize_t i = 0; i < n; ++i) {
            char c = buf[i];
            if (c >= '0' && c <= '9') {
                cur = cur * 10 + (c - '0');
                in_num = 1;
                dirty = 1;
            } else if (c == '-' && !in_num) {
                neg = 1;
                dirty = 1;
            } else {
                if (in_num) sum += neg ? -cur : cur;
                in_num = 0; neg = 0; cur = 0;
                if (c == '\n') {
                    if (dirty) print_num(sum);
                    sum = 0; dirty = 0;
                } else if (c != ' ' && c != '\t' && c != '\r') {
                    dirty = 1;
                }
            }
        }
    }
    if (in_num) sum += neg ? -cur : cur;
    if (dirty) print_num(sum);

    flush_out();
    send_block("", 0);      

    munmap(sh, sizeof(struct shared));
    sem_close(sem_full_p);
    sem_close(sem_empty_p);
    return 0;
}