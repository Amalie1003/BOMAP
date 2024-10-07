#ifndef _PRECOMPUTE_H_
#define _PRECOMPUTE_H_

#include <vector>
#include <cmath>
#include <limits.h>
#include <iostream>
#include <stdlib.h>
#include <stdint.h>

double f1(double x, double y, int L);
double f(double y, int N, int L);
double fx(double y, int N, int L); 
void gradientDescent(double& x, double alpha, double minError, int N, int L);
std::vector<int> param_bODS(int N, int L);

#endif
