#include "mdmath.h"

int abs(int x){
	return (x < 0) ? -x : x;
}

float absf(float x){
	return (x < 0.0f) ? -x : x;
}

int min(int a, int b){
	return (a < b) ? a : b;
}

int max(int a, int b){
	return (a > b) ? a : b;
}

int power10(int x){
    int r = 1;
    while (x-- > 0) r *= 10;
    return r;
}

int power(int x, int y){
	int result = 1;
	
	if (y < 0){
		x = 1 / x;
		y = -y;
	}
	
	for (int i; i < y; i++){
		result *= x;
	}
	
	return result;
}

float squaret(float x){
    if (x == 0.0f || x < 0.0f) {
        return 0.0f;
    }

    float guess = x;
    float prev_guess;
    const float epsilon = 1e-7f;

    do {
        prev_guess = guess;
        guess = (guess + x / guess) * 0.5f;
    } while (absf(guess - prev_guess) > epsilon);

    return guess;
}

float fmodf(float x, float y){
    if (y == 0.0f) {
        return 0.0f;
    }
    float n = (float)(int)(x / y);
    return x - n * y;
}

float sinf(float x){
    x = fmodf(x, 2.0f * PI);
    if (x > PI) x -= 2.0f * PI;
    else if (x < -PI) x += 2.0f * PI;

    float result = x;
    float term = x;
    int n = 1;

    while (absf(term) > 1e-7f) {
        term = -term * x * x / ((2 * n) * (2 * n + 1));
        result += term;
        n++;
    }

    return result;
}

float cosf(float x){
    x = fmodf(x, 2.0f * PI);
    if (x > PI) x -= 2.0f * PI;
    else if (x < -PI) x += 2.0f * PI;

    float result = 1.0f;
    float term = 1.0f;
    int n = 1;

    while (absf(term) > 1e-7f) {
        term = -term * x * x / ((2 * n - 1) * (2 * n));
        result += term;
        n++;
    }

    return result;
}

float tanf(float x){
    x = fmodf(x, 2.0f * PI);
    if (x > PI) x -= 2.0f * PI;
    else if (x < -PI) x += 2.0f * PI;

    float cos_x = cosf(x);
    if (absf(cos_x) < 1e-7f) {
        return (sinf(x) > 0) ? INF : -INF;
    }
    return sinf(x) / cos_x;
}