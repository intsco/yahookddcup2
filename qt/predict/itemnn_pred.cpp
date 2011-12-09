#include "itemnn_pred.h"

QHash<QPair<int, int>, float> items_neighbors;

struct next_less {
    bool operator()(QPair<int, float> const &a, QPair<int, float> const &b) const {
        return a.second > b.second;
    }
};

void itemnn_pred::study(RsHash train, bool verbose) {
    if (verbose) printf("Studing item NN ...\n");

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
    if (verbose) printf("OK\n");

    // calculating weights
    if (verbose) printf("Calculating item to item weights ...\n");
    QList<int> items = items_users_set.keys();
    qSort(items);
    int items_n = items.count();
    int n = 0;
    while(n < items_n) {

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
        int k = 0, n = 20;
        if (i_neighb.size() < 20) n = i_neighb.size();
        while(k < n) {
            tmp_i_neighb.append(i_neighb[k]);
            k++;
        }
        i_neighb = tmp_i_neighb;

        // append to the items_neighbors hash
        k = 0;
        while(k < n) {
            QPair<int, int> pair;
            pair.first = i;
            pair.second = i_neighb[k].first;
            items_neighbors.insert(pair, i_neighb[k].second);
            k++;
        }

        printf("%3.3f %% complited\r", float(n)/items_n);
        n++;
    }
    if (verbose) printf("OK\n");

    if (verbose) printf("OK\n");
    /*QSetIterator<int> iter(items_users_set[3091]);
    while (iter.hasNext()){
        printf("%d\t", iter.next());
    }*/
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
















