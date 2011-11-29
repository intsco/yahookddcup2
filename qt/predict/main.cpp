#include "main.h"

RsHash load_trainset(QString fileName, int set) {
    printf("Start loading set from %s ...", qPrintable(fileName));

    RsHash users_rs = new QHash<int, QHash<int, float>* >();

    QFile file(fileName);
    file.open(QFile::ReadOnly);
    QTextStream in(&file);

    int u = 0, i = 0, r = 0;
    QStringList list;
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (line.contains('|')) {
            list = line.split('|');
            u = list.at(0).toInt();
            QHash<int, float> *rs = new QHash<int, float>();
            users_rs->insert(u, rs);
        }
        else {
            list = line.split('\t');
            i = list.at(0).toInt();
            if (set == VALID)
                r = 0;
            else
                r = list.at(1).toInt();
            (*users_rs)[u]->insert(i, r);
        }

//        printf(qPrintable(line+'\n'));
    }

    file.close();
    printf("OK\n");
    return users_rs;
}

TaxHash load_tracks(QString tracks_fn) {
    printf("Start loading tracks ...\n");

    TaxHash tracks = new QHash<int, QVector<int>* >();
    QFile file(tracks_fn);
    file.open(QFile::ReadOnly);
    QTextStream in(&file);
    while (!in.atEnd()) {
        QStringList list = in.readLine().split("|");
        QVector<int>* track_tax = new QVector<int>();
        for (int j = 1; j < list.size(); j++) {
            int el = 0;
            if (list[j] == "None")
                el = -1;
            else
                el = list[j].toInt();
            track_tax->append(el);
        }
        tracks->insert(list[0].toInt(), track_tax);
    }
    file.close();
    printf("OK\n");
    return tracks;
}

void release_set(RsHash users_rs) {
    // deleting train set
    printf("deleting set... ");
    RsHashIter i2(*users_rs);
    while (i2.hasNext()) {
        i2.next();
        //printf("deleting %d user %d rs...", i2.key(), i2.value()->size());
        delete i2.value();
        //printf("Ok");
    }
    delete users_rs;
    printf("OK\n");
}

void release_tracks(TaxHash tracks) {
    // deleting tracks
    printf("deleting tracks... ");
    QHashIterator<int, QVector<int>* > i2(*tracks);
    while (i2.hasNext()) {
        i2.next();
        //printf("deleting %d user %d rs...", i2.key(), i2.value()->size());
        delete i2.value();
        //printf("Ok");
    }
    delete tracks;
    printf("OK\n");
}

struct prev {
    bool operator()(QPair<int, float> const &a, QPair<int, float> const &b) const {
        return a.second < b.second;
    }
};

QVector<QPair<int, float> > get_sorted_items(QHash<int, float> rs) {
    QVector<QPair<int, float> > items_rs;
    QHashIterator<int, float> it(rs);
    while (it.hasNext()) {
        it.next();
        QPair<int, float> item_r;
        item_r.first = it.key();
        item_r.second = it.value();
        items_rs.append(item_r);
    }

    std::sort(items_rs.begin(), items_rs.end(), prev());

    return items_rs;
}

double estimate(RsHash valid, QString valid_fn) {
    printf("Estimating prediction results...\n");

    // iterate through users
    RsHashIter it(*valid);
    while (it.hasNext()) {
        it.next();
        // classifing predictions into 2 groups
        QVector<QPair<int, float> > sort_items = get_sorted_items(*(it.value()));
        for (int j = 0; j < 3; j++) {
            int i = sort_items[j].first;
            it.value()->insert(i, -1);
        }
    }

    double err = 0;
    int n = 0, u = 0, i = 0, real_r = 0, pred_r = 0;

    // comparing prediction results and real data from the file
    QFile file(valid_fn);
    file.open(QFile::ReadOnly);
    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList list;
        if (line.contains("|")) {
            list = line.split("|");
            u = list[0].toInt();
        }
        else {
            list = line.split("\t");
            i = list[0].toInt();
            real_r = list[1].toInt();
            pred_r = (*(*valid)[u])[i];
            if (real_r >= 0) real_r = 1;
            if (pred_r >= 0) pred_r = 1;
            if (real_r != pred_r) {
                err += 1;
            }
            n += 1;
        }
    }
    err /= n;
    printf("Error rate: %3.3f, n=%d\n", err*100, n);
    file.close();
    return err;
}

float* get_alb_art_avg_r(QHash<int, float> u_rs, TaxHash tracks, int alb, int art) {
    float avg_art_r = 0, avg_alb_r = 0;
    int art_r_n = 0, alb_r_n = 0;

    QHashIterator<int, float> it(u_rs);
    while(it.hasNext()) {
        it.next();
        int i = it.key();
        int r = it.value();
        if (tracks->contains(i)) { // TODO: use not only tracks ratings
            if(tracks->value(i)->at(0) == alb) {
                avg_alb_r += r;
                alb_r_n++;
            }
            if(tracks->value(i)->at(1) == art) {
                avg_art_r += r;
                art_r_n++;
            }
        }
    }
    if (alb_r_n > 0)
        avg_alb_r /= alb_r_n;
    if (art_r_n > 0)
        avg_art_r /= art_r_n;

    float* avg_r = new float[2];
    avg_r[0] = avg_alb_r;
    avg_r[1] = avg_art_r;
    return avg_r;
}

void predict(RsHash train, RsHash valid, TaxHash tracks) {
    printf("Predicting users ratings... ");
    // iterate through users (valid)
    RsHashIter it(*valid);
    while (it.hasNext()) {
        it.next();
        int u = it.key();
        // iterate through user ratings
        QHashIterator<int, float> it2(*(it.value()));
        while (it2.hasNext()) {
            int art = 0, alb = 0, art_r = 0, alb_r =0, i = 0;
            it2.next();

            i = it2.key();
            art = tracks->value(i)->at(1);
            alb = tracks->value(i)->at(0);
            if (train->value(u)->contains(art)) {
                art_r = train->value(u)->value(art);
            }
            if (train->value(u)->contains(alb)) {
                alb_r = train->value(u)->value(alb);
            }

            float *alb_art_avr_r = get_alb_art_avg_r(*(train->value(u)), tracks, alb, art);

            valid->value(u)->insert(i, art_r + alb_r + alb_art_avr_r[0] + alb_art_avr_r[1]);
        }
    }
    printf("OK\n");
}

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);
    QCoreApplication::setApplicationName("YahooKDDCup Prediction");

    printf("Start prediction\n");

    QString train_file = "../../../train_sample.txt";
    QString valid_file = "../../../valid_sample.txt";
    QString tracks_file = "../../../_trackData.txt";

    RsHash train = load_trainset(train_file, TRAIN);
    RsHash valid = load_trainset(valid_file, VALID);
    TaxHash tracks = load_tracks(tracks_file);

    predict(train, valid, tracks);

    estimate(valid, valid_file);

    release_set(train);
    release_set(valid);
    release_tracks(tracks);

    printf("Prediction finished\n");
    return a.exec();
}














