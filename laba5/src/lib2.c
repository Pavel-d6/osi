#include "contract.h"

#define pi 3.14159265358979323846

static double MySin(double x) {
    const double tpi = 2 * pi;

    long long k = (long long)(x / tpi);
    x -= k * tpi;
    if (x >  pi) x -= tpi;
    if (x < -pi) x += tpi;

    if (x >  pi / 2) x =  pi - x;
    if (x < -pi / 2) x = -pi - x;

    double x2 = x * x;
    double term = x, sum = x;
    for (int i = 1; i < 12; i++) {
        term *= -x2 / ((2 * i) * (2 * i + 1));
        sum += term;
    }
    return sum;
}

float SinIntegral(float A, float B, float e) {
    if (e <= 0 || B <= A) return 0;
    int n = (int)((B - A) / e);
    double sum = 0;
    for (int i = 0; i < n; i++) {
        double x0 = A + i * (double)e;
        sum += (MySin(x0) + MySin(x0 + e)) / 2 * e;
    }
    double x = A + n * (double)e;
    if (x < B) sum += (MySin(x) + MySin(B)) / 2 * (B - x);
    return (float)sum;
}

float Square(float A, float B) {
    return A * B / 2;
}