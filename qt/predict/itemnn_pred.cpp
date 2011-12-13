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

#pragma omp parallel for
            for(int n = 0; n < items_n; n++) {

                // calculate weights
                int i = items[n];
                QSet<int> iset = items_users_set[i];
                QVector<QPair<int, float> > i_neighb;
                int n2 = n + 1;
                while (n2 < items_n) {
                    int j = items[n2];
                    QSet<int> jset = items_users_set[j];
                    float ij_intersect = iset.intersect(jset).size();
                    if (ij_intersect > 0) {
                        float w = ij_intersect / iset.unite(jset).size();
                        //printf("%d  %d   %2.4f\n", i, j, w);
                        if (w > 0.5) {
                            QPair<int, float> pair;
                            pair.first = j;
                            pair.second = w;
                            i_neighb.append(pair);
                        }
                    }
                    n2++;
                }

                // sort and shrink to N neighbors
                std::sort(i_neighb.begin(), i_neighb.end(), next_less()); // sort in descending order by weights
                QVector<QPair<int, float> > tmp_i_neighb;
                int k = 0, neighb_n = 20;
                if (i_neighb.size() < 20) neighb_n = i_neighb.size();
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
                    i2i_weights.insert(pair, i_neighb[k].second);
                    k++;
                }
#pragma omp critical
                {
                    if (n % 100 == 0) printf("%d items processed  %f %% complited\r", n, float(n)/items_n*100);
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

    if (verbose) printf("OK\n");
}

void itemnn_pred::predict(RsHash train, RsHash &valid, float params, bool verbose) {
    if (verbose) printf("Predicting (item nn)... \n");

    // iterate through users (valid)
    RsHashIterMut vit(valid);
    int users_n = valid.size(), n = 0;
    while (vit.hasNext()) {
        vit.next();
        n++;
        int u = vit.key();
        QHash<int, float> urs = train[u];
        // iterate through user ratings
        QMutableHashIterator<int, float> iit(vit.value());
        while (iit.hasNext()) {
            iit.next();
            int i = iit.key();
            /*QSet<int> iset = items_users_set[i];

            // compute rating as weighted sum of neighbors
            float r = 0;
            QHashIterator<int, float> it(urs);
            while(it.hasNext()) {
                it.next();
                int ti = it.key();
                float tr = it.value();
                QSet<int> tiset = items_users_set[ti];

                r += w * (1 + tr);
            }

            iit.value() = r;*/
            //printf("%d %d %f\n", u, i, valid[u][i]);
        }
        printf("%3.3f %% complited\r", float(n)/users_n);
    }
    if (verbose) printf("OK\n");
}
















