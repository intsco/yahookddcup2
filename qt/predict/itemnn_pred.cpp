#include "itemnn_pred.h"

QHash<QPair<int, int>, float> i2i_weights;

struct next_less {
    bool operator()(QPair<int, float> const &a, QPair<int, float> const &b) const {
        return a.second > b.second;
    }
};

void itemnn_pred::study(RsHash train, bool verbose) {
    if (verbose) printf("Studing item NN ...\n");

    QFile binfile("../../tmp_i2i_weights_all.bin");
    if (!binfile.exists()) {

        if (verbose) printf("Preparing items users sets ...\n");
        // prepare items users sets
        QFile items_users_binfile("../../items_users.bin");
        QList< QPair<int, QSet<int> > > items_users_sets;

        QHash<int, QSet<int> > items_users_sets_h;
        if(!items_users_binfile.exists()) {
            RsHashIter it(train);
            while(it.hasNext()) {
                it.next();
                int u = it.key();
                QHashIterator<int, float> iit(it.value());
                while(iit.hasNext()) {
                    iit.next();
                    int i = iit.key();
                    if (!items_users_sets_h.contains(i)) {
                        QSet<int> users;
                        users.insert(u);
                        items_users_sets_h.insert(i, users);
                    }
                    else {
                        items_users_sets_h[i].insert(u);
                    }
                }
            }
            // change data structure (hash -> list)
            QList<int> items = items_users_sets_h.keys();
            qSort(items);
            int item = 0;
            foreach(item, items) {
                QPair<int, QSet<int> > pair(item, items_users_sets_h[item]);
                items_users_sets.append(pair);
            }
            // serialize
            items_users_binfile.open(QFile::WriteOnly);
            QDataStream bin(&items_users_binfile);
            bin << items_users_sets;
            items_users_binfile.close();
        }
        else {
            // deserialize
            items_users_binfile.open(QFile::ReadOnly);
            QDataStream bin(&items_users_binfile);
            bin >> items_users_sets;
            items_users_binfile.close();
        }

        // calculating weights
        if (verbose) printf("Calculating item to item weights ...\n");
        QTime myTimer;
        myTimer.start();

        int items_processed = 0, items_n = items_users_sets.size();

        //QList< QPair<int, QSet<int> > >::const_iter it;
        //for(it = items_users_sets.constBegin(); it < items_users_sets.constEnd(); ++it) {
//#pragma omp parallel for schedule(dynamic, 50)
        for(int n = 0; n < items_n; n++) {

            // calculate weights
            //int i = (*it).first;
            //QSet<int> iset = (*it).second;
            int i = items_users_sets[n].first;
            QSet<int> iset = items_users_sets[n].second;
            QVector<QPair<int, float> > i_neighb;
            //QVector< QPair<int, QSet<int> > >::const_iter it;
            for(int n2 = n + 1; n2 < items_n; n2++) {
                int j = items_users_sets[n2].first;
                QSet<int> jset = items_users_sets[n2].second;
                QSet<int> itersect_set(iset);
                itersect_set.intersect(jset);
                if (itersect_set.size() > 0) {
                    QSet<int> unite_set(iset);
                    unite_set.unite(jset);
                    float w = (float)itersect_set.size() / unite_set.size();
                    if (w > 0.01) {
                        QPair<int, float> pair(j, w);
                        //pair.first = j;
                        //pair.second = w;
                        i_neighb.append(pair);
                        //printf("i=%d j=%d w=%1.4f\n", i, pair.first, pair.second);
                    }
                }
            }

            // sort and shrink to N neighbors
            std::sort(i_neighb.begin(), i_neighb.end(), next_less()); // sort in descending order by weights
            QVector<QPair<int, float> > tmp_i_neighb;
            int k = 0, neighb_n = 1500;
            if (i_neighb.size() < neighb_n) neighb_n = i_neighb.size();
            while(k < neighb_n) {
                tmp_i_neighb.append(i_neighb[k]);
                k++;
            }
            i_neighb = tmp_i_neighb;

            // append to the i2i hash
            k = 0;
            while(k < neighb_n) {
                QPair<int, int> pair(i, i_neighb[k].first);
                //pair.first = i;
                //pair.second = i_neighb[k].first;
//#pragma omp critical
                {
                    i2i_weights.insert(pair, i_neighb[k].second);
                }
                k++;
            }
//#pragma omp critical
            {
                items_processed++;
                if (items_processed % 1 == 0)
                    printf("%d items processed  %f %% complited i2i.size=%d  speed=%5.0f\r",
                           items_processed, float(items_processed)/items_n*100,
                           i2i_weights.size(), (float)(items_processed*1000)/myTimer.elapsed() );

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
    int users_n = valid.size(), n = 0; //neighb_n = 50;
    while (uit.hasNext()) {
        uit.next();
        n++;
        int u = uit.key();
        QHash<int, float> u_rs = train[u]; // user's train ratings
        // iterate through user valid ratings
        QHashIterator<int, float> iit(uit.value());
        while (iit.hasNext()) {
            iit.next();
            int i = iit.key();

            // compute rating as weighted sum of neighbours
            QHash<int, float>::const_iterator it;
            QList< QPair<int, float> > most_sim_neighb;
            for(it = u_rs.constBegin(); it != u_rs.constEnd(); ++it) {
                int j = it.key();
                QPair<int, int> pair(i, j);
                float w = i2i_weights.value(pair, 0);
                QPair<int, float> neighb_w(j, w);
                most_sim_neighb.append(neighb_w);
            }
            float r = 0;
            std::sort(most_sim_neighb.begin(), most_sim_neighb.end(), next_less());
            QListIterator< QPair<int, float> > nit(most_sim_neighb);
            int k = 0;
	    while(nit.hasNext() && k < 50) {
		QPair<int, float> neighb_w = nit.next();
		int j = neighb_w.first;
		float w = neighb_w.second;
		if (w > 0)
		    r += w * pow(1 + u_rs[j], p);
		k++;
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












