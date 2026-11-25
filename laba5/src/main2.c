#include <stdio.h>
#include <dlfcn.h>

#define EXT ".dylib"


typedef float (*sin_fn)(float, float, float);
typedef float (*sq_fn)(float, float);

static void  *handle = NULL;
static sin_fn sin_integral = NULL;
static sq_fn  square = NULL;

static int load(const char *path) {
    void *h = dlopen(path, RTLD_LAZY);
    if (!h) { fprintf(stderr, "%s\n", dlerror()); return -1; }

    sin_fn f = (sin_fn)dlsym(h, "SinIntegral");
    sq_fn  g = (sq_fn)dlsym(h, "Square");
    if (!f || !g) {
        fprintf(stderr, "%s\n", dlerror());
        dlclose(h);
        return -1;
    }
    if (handle) dlclose(handle);
    handle = h; sin_integral = f; square = g;
    return 0;
}

int main(void) {
    const char *paths[2] = { "./libimpl1" EXT, "./libimpl2" EXT };
    int cur = 0;
    if (load(paths[cur]) != 0) return 1;
    printf("Загружена реализация %d\n", cur + 1);

    char line[256];
    while (fgets(line, sizeof line, stdin)) {
        int cmd;
        if (sscanf(line, "%d", &cmd) != 1) continue;

        if (cmd == 0) {
            int next = cur ^ 1;
            if (load(paths[next]) == 0) {
                cur = next;
                printf("Переключено на реализацию %d\n", cur + 1);
            }
        } else if (cmd == 1) {
            float a, b, e;
            if (sscanf(line, "%*d %f %f %f", &a, &b, &e) == 3)
                printf("%f\n", sin_integral(a, b, e));
            else puts("Нужно: 1 A B e");
        } else if (cmd == 2) {
            float a, b;
            if (sscanf(line, "%*d %f %f", &a, &b) == 2)
                printf("%f\n", square(a, b));
            else puts("Нужно: 2 A B");
        } else {
            puts("Неизвестная команда");
        }
    }
    if (handle) dlclose(handle);
    return 0;
}