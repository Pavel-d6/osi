#include <stdio.h>
#include "contract.h"

int main(void) {
    char line[1024];
    while (fgets(line, sizeof line, stdin)) {
        int cmd;
        if (sscanf(line, "%d", &cmd) != 1) continue;

        if (cmd == 1) {
            float a, b, e;
            if (sscanf(line, "%*d %f %f %f", &a, &b, &e) == 3)
                printf("%f\n", SinIntegral(a, b, e));
            else printf("Нужно: 1 A B e\n");
        } else if (cmd == 2) {
            float a, b;
            if (sscanf(line, "%*d %f %f", &a, &b) == 2)
                printf("%f\n", Square(a, b));
            else printf("Нужно: 2 A B\n");
        } else {
           printf("есть только 1 и 2\n");
        }
    }
    return 0;
}