#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>



static void perror_hand(const char *msg){
    const char* e = strerror(errno);
    write(2, msg, strlen(msg));
    write(2, ":",2);
    write(2, e, strlen(e));
    write(2, "\n", 1);
    exit(1);
}


static void write_full(int fd, const char* buf, size_t n){
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

int main(int args, char* ardv[]){
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

    int p[2];
    if (pipe(p) == -1) perror_hand("error create pipe");

    int fd = open(fileName, O_RDONLY);
    if (fd == -1) perror_hand("error open file");

    pid_t pid = fork();
    if (pid == -1) perror_hand("error fork operation");

    if (pid == 0){
        if (dup2(fd, 0) == -1) perror_hand("dup2 error stdin");
        if (dup2(p[1], 1) == -1) perror_hand("dup2 error stdout");
        close(fd), close(p[0]), close(p[1]);
        execl("./child", "child", NULL);
        perror_hand("error execl");
    }

    close(fd);
    close(p[1]);

    char buf[4096];
    ssize_t n;
    while ((n = read(p[0], buf, sizeof(buf))) != 0) {
        if (n == -1) {
            if (errno == EINTR) continue;
            perror_hand("read pipe");
        }
        write_full(1, buf, n);
    }
    close(p[0]);

    int status;
    if (waitpid(pid, &status, 0) == -1) perror_hand("waitpid");
    return 0;

}