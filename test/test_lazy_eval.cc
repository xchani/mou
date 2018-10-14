#include <cstdio>
#include "../include/lazy_eval.h"

using namespace mou;
using namespace mou::expr;

struct maximum {
    inline static float Map(float a, float b) {
        return a > b ? a : b;
    }
};

struct minimum {
    inline static float Map(float a, float b) {
        return a > b ? b : a;
    }
};

const int n = 3;

int main(void) {
    float sa[n] = {1, 2, 3};
    float sb[n] = {2, 3, 4};
    float sc[n] = {3, 4, 5};
    Vec<float> A(sa, n), B(sb, n), C(sc, n);

    A = F<div>(B, F<maximum>(C, B));

    for (int i = 0; i < n; ++i) {
        printf("%d:%f == %f / max(%f, %f)\n",
                i, A.dptr[i], B.dptr[i], C.dptr[i], B.dptr[i]);
    }

    A = F<div>(B, F<minimum>(C, B));

    for (int i = 0; i < n; ++i) {
        printf("%d:%f == %f / max(%f, %f)\n",
                i, A.dptr[i], B.dptr[i], C.dptr[i], B.dptr[i]);
    }

    return 0;
}
