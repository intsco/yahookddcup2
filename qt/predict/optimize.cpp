#include "optimize.h"

int rrand(int from, int to) {
    return from + (qrand() % (to-from));
}

// brute-force optimization
void optimize(RsHash train, RsHash &valid, QString valid_file, TaxHash tracks) {
    printf("Optimizing ...\n");

    qsrand(2011);
    float err = 0.5, errThr = 0.112, minerr = 0.5;
    int alfa = 0, alfa_h = 100, best_alfa = 0;
    int n = 0, nThr = 200;
    do {
        alfa++;
        n++;
        content_pred::predict(train, valid, tracks, alfa, false);
        err = estimate(valid, valid_file, false);
        printf("n = %d  alfa = %d   err = %3.4f\n", n, alfa, err*100);
        if (err < minerr) {
            minerr = err;
            best_alfa = alfa;
        }
    }
    while (err > errThr && alfa <= alfa_h);

    printf("OK. Best result: alfa = %d  err = %3.4f\n", best_alfa, minerr*100);
}


