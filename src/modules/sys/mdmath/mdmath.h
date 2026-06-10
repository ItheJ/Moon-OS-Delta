#ifndef MDMATH_H
#define MDMATH_H

#define PI 3.141592f
#define TO_DEGRESS PI/180

#define INF 0x7f7fffff

int abs(int x);
float absf(float x);

int min(int a, int b);
int max(int a, int b);
int power10(int x);
int power(int x, int y);

float fmodf(float x, float y);

float squaret(float x);
float sinf(float x);
float cosf(float x);
float tanf(float x);

#endif