#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "common.h"

static sem_t *sem_full_p = SEM_FAILED;

void perror_hand(const char *msg){
    const char* e = strerror(errno);
    write(2, msg, strlen(msg));
    write(2, ": ", 2);
    write(2, e, strlen(e));
    write(2, "\n", 1);
    exit(1);
}

void write_full(int fd, const char* buf, size_t n){
    while(n>0){
        ssize_t w = write(fd, buf, n);
        if (w == -1){
            if (errno == EINTR){continue;}
            perror_hand("write");
        }
        buf += w;
        n -= w;
    }
}

void on_sigchld(int sig){
    (void)sig;
    int saved = errno;
    if (sem_full_p != SEM_FAILED) sem_post(sem_full_p);
    errno = saved;
}

void cleanup(void){
    shm_unlink(SHM_NAME);
    sem_unlink(SEM_FULL);
    sem_unlink(SEM_EMPTY);
}

int main(void){
    char fileName[256];
    size_t len = 0;
    char c;

    while(1){
        ssize_t r = read(0, &c, 1);
        if (r == -1){
            if(errno == EINTR) continue;
            perror_hand("error read name file");
        }
        if ( r == 0 || c == '\n') break;
        if (len < sizeof(fileName) - 1) fileName[len++] = c;
    }
    fileName[len] = '\0';
    if (len == 0){
        write_full(2, "empty file name\n", 16);
        exit(1);
    }

    int fd = open(fileName, O_RDONLY);
    if (fd == -1) perror_hand("error open file");

    cleanup();

    int sfd = shm_open(SHM_NAME, O_CREAT | O_EXCL | O_RDWR, 0600);
    if (sfd == -1) perror_hand("shm_open");
    if (ftruncate(sfd, sizeof(struct shared)) == -1){ cleanup(); perror_hand("ftruncate"); }

    struct shared *sh = mmap(NULL, sizeof(struct shared), PROT_READ | PROT_WRITE,
                             MAP_SHARED, sfd, 0);
    if (sh == MAP_FAILED){ cleanup(); perror_hand("mmap"); }
    close(sfd);
    sh->state = 0;
    sh->len = 0;

    sem_full_p = sem_open(SEM_FULL, O_CREAT | O_EXCL, 0600, 0);
    if (sem_full_p == SEM_FAILED){ cleanup(); perror_hand("sem_open full"); }
    sem_t *sem_empty_p = sem_open(SEM_EMPTY, O_CREAT | O_EXCL, 0600, 1);
    if (sem_empty_p == SEM_FAILED){ cleanup(); perror_hand("sem_open empty"); }

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = on_sigchld;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_NOCLDSTOP;
    if (sigaction(SIGCHLD, &sa, NULL) == -1){ cleanup(); perror_hand("sigaction"); }

    pid_t pid = fork();
    if (pid == -1){ cleanup(); perror_hand("error fork operation"); }

    if (pid == 0){
        if (dup2(fd, 0) == -1) perror_hand("dup2 error stdin");
        close(fd);
        execl("./child", "child", NULL);
        perror_hand("error execl");
    }

    close(fd);

    int child_failed = 0;
    while (1){
        if (sem_wait(sem_full_p) == -1){
            if (errno == EINTR) continue;
            cleanup();
            perror_hand("sem_wait full");
        }
        if (sh->state != 1){          
            child_failed = 1;
            break;
        }
        int n = sh->len;
        if (n == 0) break;           
        write_full(1, sh->data, n);
        sh->state = 0;
        if (sem_post(sem_empty_p) == -1){ cleanup(); perror_hand("sem_post empty"); }
    }

    int status;
    while (waitpid(pid, &status, 0) == -1){
        if (errno == EINTR) continue;
        cleanup();
        perror_hand("waitpid");
    }

    munmap(sh, sizeof(struct shared));
    sem_close(sem_full_p);
    sem_close(sem_empty_p);
    cleanup();

    if (child_failed || !WIFEXITED(status) || WEXITSTATUS(status) != 0){
        write_full(2, "child terminated with error\n", 28);
        return 1;
    }
    return 0;
}