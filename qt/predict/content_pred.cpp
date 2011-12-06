#include "content_pred.h"

void get_alb_art_avg_r(QHash<int, float> u_rs, TaxHash tracks, int alb, int art, float &avg_alb_r, float &avg_art_r) {
    int art_r_n = 0, alb_r_n = 0;

    QHashIterator<int, float> it(u_rs);
    while(it.hasNext()) {
        it.next();
        int i = it.key();
        int r = it.value();
        if (tracks.contains(i)) {
            if(tracks.value(i).at(0) == alb && alb != -1) {
                avg_alb_r += r;
                alb_r_n++;
            }
            if(tracks.value(i).at(1) == art && art != -1) {
                avg_art_r += r;
                art_r_n++;
            }
        }
    }
    if (alb_r_n > 0)
        avg_alb_r /= alb_r_n;
    if (art_r_n > 0)
        avg_art_r /= art_r_n;
}

void predict(RsHash train, RsHash &valid, TaxHash tracks, float param, bool verbose) {
    if (verbose) printf("Predicting users ratings... ");

    // track without taxonomy info default rating
    float default_r = param;

    // iterate through users (valid)
    RsHashIterMut it(valid);
    while (it.hasNext()) {
        it.next();
        int u = it.key();
        // iterate through user ratings
        QMutableHashIterator<int, float> it2(it.value());
        while (it2.hasNext()) {
            int art = 0, alb = 0, art_r = 0, alb_r =0, i = 0;
            it2.next();

            i = it2.key();
            art = tracks.value(i).at(1);
            alb = tracks.value(i).at(0);
            if (train.value(u).contains(art)) {
                art_r = train.value(u).value(art);
            }
            if (train.value(u).contains(alb)) {
                alb_r = train.value(u).value(alb);
            }

            float alb_avr_r = 0, art_avr_r = 0;
            if (alb == -1 && art == -1) {
                alb_avr_r = art_avr_r = default_r;
            }
            else {
                get_alb_art_avg_r(train.value(u), tracks, alb, art, alb_avr_r, art_avr_r);
            }

            it2.value() = art_r + alb_r + alb_avr_r + art_avr_r;
            //printf("%d %d %f\n", u, i, valid[u][i]);
        }
    }
    if (verbose) printf("OK\n");
}












