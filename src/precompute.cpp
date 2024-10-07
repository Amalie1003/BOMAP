#include "precompute.hpp"
// #include <iostream>
// #include <cmath>
// #include <vector>
// #include <stdint.h>

using namespace std;
int C1 = 36;
int C2 = 36;


double f1(double x, double y, int L)
{
    double term1 = C1 * y;
    double term2 = 4 * C1 * y * log2(ceil(pow(y, ((L - 1) * (L - 2)/2)) / pow(4, (L-2))));
    double term3 = 4 * C2 * x * log2(ceil(pow(y, (L-1)) / 4));
    return term1 + term2 + term3;
}
// 目标函数
double f(double y, int N, int L) {
    double shift = 1e-20;
    double term1 = C1 * y;
    // double term2 = 4 * C1 * y * log2(ceil(pow(y, ((L - 1) * (L - 2)/2)) / pow(4, (L-2))) + shift);
    double term2 = 4 * C1 * y * log2(ceil(pow(y, ((L - 1) * (L - 2)/2)) / pow(2, (L-2))) + shift);
    double term3 = 4 * C2 * pow(2, L-1) * N * log2(ceil(pow(y, (L-1)) / 2) + shift) / pow(y, (L-1));
    // double term3 = 4 * C2 * pow(2, L-1) * N * log2(ceil(pow(y, (L-1)) / 4) + shift) / pow(y, (L-1));
    return term1 + term2 + term3;
}

// x偏导数
double fx(double y, int N, int L) {
    double term1 = C1;
    // double term2 = 2 * C1 * (L - 1) * (L - 2) * log2(y) + 2 * C1 * (L - 1) * (L - 2) - 8 * C1 * (L - 2);
    double term2 = 2 * C1 * (L - 1) * (L - 2) * log2(y) + 2 * C1 * (L - 1) * (L - 2) - 4 * C1 * (L - 2);
    // double term3 = (double)4 * pow(2, L-1) * C2 * N * (1 - L) / pow(y, L) * log2(pow(y, (L - 1)) / 4) + (double)4 * pow(2, L-1) * C2 * N / pow(y, L) * (L - 1);
    double term3 = (double)4 * pow(2, L-1) * C2 * N * (1 - L) / pow(y, L) * log2(pow(y, (L - 1)) / 2) + (double)4 * pow(2, L-1) * C2 * N / pow(y, L) * (L - 1);
    return term1 + term2 +term3;
}


// 梯度下降法求解最小值
void gradientDescent(double& x, double alpha, int maxIter, double minError, int N, int L) {
    for (int i = 0; ; i++) {
        if(x < 3) x = 3;
        double dx = fx(x, N, L);
        // std::cout << x << ":" << dx << " ";
        x -= alpha * dx;
        double error = abs(f(x, N, L) - f(x - dx, N, L));
        // std::cout << error << std::endl;
        if (error < minError) break;
    }
}

vector<int> param_bODS(int N, int L)
{
    double y0 = 100;
    double alpha = 0.0001;
    int maxIter = 1000;
    double minError = 1e-6;
    gradientDescent(y0, alpha, maxIter, minError, N, L);

    cout << "Minimum value of f(x,y) is " << f(y0, N, L) << " at p=" << y0 << endl;
    vector<int> ylist(2);
    vector<int> xlist(4);  
    ylist[0] = floor(y0);
    ylist[1] = ceil(y0);
    double x0 = (double)pow(2, L-1) * N / pow(ylist[0], L-1);
    xlist[0] = floor(x0);
    xlist[1] = ceil(x0);
    double x1 = (double)pow(2, L-1) * N / pow(ylist[1], L-1);
    xlist[2] = floor(x1);
    xlist[3] = ceil(x1);

    int min_index;
    double min_val = INT32_MAX;
    for(int i = 0 ; i < 8; i++)
    {
        if(i == 0 || i == 5 || i == 4 || i == 3) continue;
        int y = ylist[i % 2];
        int x = xlist[i / 2];
        if(x * pow(y, L-1) < pow(2, L-1) * N) continue;
        double val = f1(x, y, L);
        if(val <min_val)
        {
            min_val = val;
            min_index = i;
        }
    }
    cout << "B = " << xlist[min_index/2] << ", p=" << ylist[min_index%2] << ", minComn = " << min_val <<endl;
	vector<int> ret(2);
	ret[0] = ylist[min_index%2];
	ret[1] = xlist[min_index/2];
    return ret;
}

int main() {
    
    // cout << fx(3) <<endl;
    // cout << f(2.5) << " " << f(3) << " " << f(3.5) << endl;
    for(int L = 3; L <= 5; L++)
    {
        param_bODS(pow(2, 10), L);
    }
    
    return 0;
}
