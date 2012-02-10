#include "optimize.h"

int rrand(int from, int to) {
    return from + (qrand() % (to-from));
}

// brute-force optimization (1-Dim)
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

// the golden section optimization (1-Dim)
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

void optimize_gd(RsHash train, RsHash valid, TaxHash tracks, QString valid_fn, QString train_neg_fn)
{
    printf("Gradient descent optimizing...\n");

    QList<float> p;
    p.append(10);
    p.append(0.01);
    p.append(0.01);
    double py = binsvd_pred::study(train, valid, tracks, train_neg_fn, valid_fn, p, false);
    printf("p1=%3.4f, p2=%3.4f, p3=%3.4f, err = %3.4f\n", p[0], p[1], p[2], py);

    QList<float> pp;
    pp = p;
    p[0] = 20;
    p[1] = 0.001;
    p[2] = 0.1;
    double y = binsvd_pred::study(train, valid, tracks, train_neg_fn, valid_fn, p, false);
    printf("p1=%3.4f, p2=%3.4f, p3=%3.4f, err = %3.4f\n", p[0], p[1], p[2], y);

    float nu = 0.01; // study speed
    while(abs(y - py) > 0.0001)
    {
        for(int i = 0; i < p.count(); ++i)
        {
            float dp = p[i] - pp[i];
            pp[i] = p[i];
            p[i] = p[i] - nu * (y - py) / dp; // moving along gradient
            if (i == 0) p[i] = floor(p[i]);
        }
        py = y;
        y = binsvd_pred::study(train, valid, tracks, train_neg_fn, valid_fn, p, false);
        printf("p1=%3.4f, p2=%3.4f, p3=%3.4f, err = %3.4f\n", p[0], p[1], p[2], y);
    }

    printf("OK. Best result: p1=%3.4f, p2=%3.4f, p3=%3.4f, err = %3.4f\n", p[0], p[1], p[2], y);
}














