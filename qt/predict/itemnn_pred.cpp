#include "itemnn_pred.h"

QHash<QPair<int, int>, float> i2i_weights;

struct next_less {
    bool operator()(QPair<int, float> const &a, QPair<int, float> const &b) const {
        return a.second > b.second;
    }
};

void itemnn_pred::study(RsHash train, bool verbose) {
    if (verbose) printf("Studing item NN ...\n");

    QFile binfile("../../i2i_weights.bin");
    if (!binfile.exists()) {

        if (verbose) printf("Preparing items users sets ...\n");
        // prepare items users sets
        QHash<int, QSet<int> > items_users_set;
        RsHashIter it(train);
        while(it.hasNext()) {
            it.next();
            int u = it.key();
            QHashIterator<int, float> iit(it.value());
            while(iit.hasNext()) {
                iit.next();
                int i = iit.key();
                if (!items_users_set.contains(i)) {
                    QSet<int> users;
                    users.insert(u);
                    items_users_set.insert(i, users);
                }
                else {
                    items_users_set[i].insert(u);
                }
            }
        }

        // calculating weights
        if (verbose) printf("Calculating item to item weights ...\n");
        QList<int> items = items_users_set.keys();
        qSort(items);
        int items_n = items.count();

        int items_processed = 0;
#pragma omp parallel for schedule(dynamic, 100)
        for(int n = 0; n < items_n; n++) {

            // calculate weights
            int i = items[n];
            QSet<int> iset = items_users_set[i];
            QVector<QPair<int, float> > i_neighb;
            for(int n2 = n + 1; n2 < items_n; n2++) {
                int j = items[n2];
                QSet<int> jset = items_users_set[j];
                QSet<int> itersect_set(iset);
                itersect_set.intersect(jset);
                if (itersect_set.size() > 0) {
                    QSet<int> unite_set(iset);
                    unite_set.unite(jset);
                    float w = (float)itersect_set.size() / unite_set.size();
                    if (w > 0.01) {
                        QPair<int, float> pair;
                        pair.first = j;
                        pair.second = w;
                        i_neighb.append(pair);
                        //printf("i=%d j=%d w=%1.4f\n", i, pair.first, pair.second);
                    }
                }
            }

            // sort and shrink to N neighbors
            std::sort(i_neighb.begin(), i_neighb.end(), next_less()); // sort in descending order by weights
            QVector<QPair<int, float> > tmp_i_neighb;
            int k = 0, neighb_n = 50;
            if (i_neighb.size() < neighb_n) neighb_n = i_neighb.size();
            while(k < neighb_n) {
                tmp_i_neighb.append(i_neighb[k]);
                k++;
            }
            i_neighb = tmp_i_neighb;

            // append to the items_neighbors hash
            k = 0;
            while(k < neighb_n) {
                QPair<int, int> pair;
                pair.first = i;
                pair.second = i_neighb[k].first;
#pragma omp critical
                {
                    i2i_weights.insert(pair, i_neighb[k].second);
                }
                k++;
            }
#pragma omp critical
            {
                items_processed++;
                if (items_processed % 100 == 0)
                    printf("%d items processed  %f %% complited i2i.size=%d\r", items_processed, float(items_processed)/items_n*100, i2i_weights.size());
            }
        }

        // serialization item to item weights
        printf("Saving to bin file...\n");
        binfile.open(QFile::WriteOnly);
        QDataStream bin(&binfile);
        bin << i2i_weights;
        binfile.close();
    }
    else {
        // deserialization item to item weights
        printf("Loading from bin file...\n");
        binfile.open(QFile::ReadOnly);
        QDataStream bin(&binfile);
        bin >> i2i_weights;
        binfile.close();
    }

    /*QHashIterator<QPair<int, int>, float> iter(i2i_weights);
    while(iter.hasNext()) {
        iter.next();
        printf("%d  %d  %f\n", iter.key().first, iter.key().second, iter.value());
    }*/

    if (verbose) printf("OK %d items * N neighbours loaded\n", i2i_weights.size());
}

void itemnn_pred::predict(RsHash train, RsHash &valid, float p, bool verbose) {
    if (verbose) printf("Predicting (item nn)... \n");

    RsHash new_valid(valid);

    // iterate through users (valid)
    RsHashIter uit(valid);
    int users_n = valid.size(), n = 0;
    while (uit.hasNext()) {
        uit.next();
        n++;
        int u = uit.key();
        QHash<int, float> u_rs = train[u]; // user's ratings
        // iterate through user ratings
        QHashIterator<int, float> iit(uit.value());
        while (iit.hasNext()) {
            iit.next();
            int i = iit.key();

            // compute rating as weighted sum of neighbours
            float r = 0;
            QHash<int, float>::const_iterator it;
            for(it = u_rs.constBegin(); it != u_rs.constEnd(); ++it) {
                int j = it.key();
                QPair<int, int> pair;
                pair.first = i;
                pair.second = j;
                float w = i2i_weights.value(pair, 0);
                if (w > 0)
                    r += w * pow(1 + it.value(), p);
            }
            new_valid[u][i] = r;
        }
        if (verbose) printf("%3.3f %% complited\r", float(n) / users_n * 100);
    }
    valid = new_valid;

    if (verbose) printf("\nOK\n");
}

RsHash itemnn_pred::get_predictions(RsHash train, RsHash &valid, float p) {
    itemnn_pred::predict(train, valid, p, false);
    return valid;
}












