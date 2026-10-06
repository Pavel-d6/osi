#include <stdlib.h>  
#include <string.h>   
#include <errno.h>    
#include <unistd.h>  


static void perror_hand(const char *msg){
    const char* e = strerror(errno);
    write(2, msg, strlen(msg));
    write(2, ": ", 2);
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


static void print_num(long long v){
    char out[32];
    int i = sizeof(out);
    unsigned long long u = v < 0 ? -(unsigned long long)v : v;

    out[--i] = '\n';

    do{
        out[--i] = u%10 + '0';
        u = u / 10;
    }while (u > 0);

    if (v < 0){out[--i] = '-';}

    write_full(1, out + i , sizeof(out) - i);
}


int main(void){
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
    if (in_num) sum += neg ? -cur: cur;
    if (dirty) print_num(sum);
    return 0;
}