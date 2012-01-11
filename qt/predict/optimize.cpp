#include "optimize.h"

int rrand(int from, int to) {
    return from + (qrand() % (to-from));
}

// brute-force optimization
void optimize_bf(RsHash train, RsHash &valid, QString valid_file,
                 RsHash (*get_pred)(RsHash, RsHash &, float)) {
    printf("Brute-force optimizing...\n");

    qsrand(2011);
    float err = 0.5, errThr = 0.09, minerr = 0.5;

    float alfa = 3, alfa_h = 4, best_alfa = 0;
    float step = 0.05;
    int n = 0; //nThr = 200;
    do {
        alfa += step;
        n++;
        //content_pred::predict(train, valid, tracks, alfa, false);
        err = estimate(get_pred(train, valid, alfa), valid_file, false);
        printf("n = %d  alfa = %1.4f   err = %3.4f\n", n, alfa, err*100);
        if (err < minerr) {
            minerr = err;
            best_alfa = alfa;
        }
    }
    while (err > errThr && alfa <= alfa_h);

    printf("OK. Best result: alfa = %1.4f  err = %3.4f\n", best_alfa, minerr*100);
}

// the golden section optimization
void optimize_gsect(RsHash train, RsHash &valid, QString valid_file,
                    RsHash (*get_pred)(RsHash, RsHash &, float) ) {
    printf("Gold section optimizing...\n");

    float err = 0.5, errThr = 0.09, minerr = 0.5;
    float gama = 0, gama_l = 0, gama_h = 5, best_gama = 0;

    float eps = 0.001, fi = (1 + sqrt(5)) / 2, a = gama_l, b = gama_h;

    float x1 = b - (b - a) / fi, x2 = a + (b - a) / fi;
    float err1 = estimate(get_pred(train, valid, x1), valid_file, false),
        err2 = estimate(get_pred(train, valid, x2), valid_file, false);
    while (err > errThr && (b-a) > eps) {
        if (err1 > err2) {
            a = x1;
            x1 = x2;
            err1 = err2;
            x2 = a + (b - a) / fi;
            err2 = estimate(get_pred(train, valid, x2), valid_file, false);
            err = err2;
            gama = x2;
        }
        else {
            b = x2;
            x2 = x1;
            err2 = err1;
            x1 = b - (b - a) / fi;
            err1 = estimate(get_pred(train, valid, x1), valid_file, false);
            err = err1;
            gama = x1;
        }

        printf("gama = %1.4f   err = %3.4f\n", gama, err*100);
        if (err < minerr) {
            minerr = err;
            best_gama = gama;
        }
    }

    printf("OK. Best result: gama = %1.4f  err = %3.4f\n", best_gama, minerr*100);
}
















