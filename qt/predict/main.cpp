#include "main.h"

RsHash load_set(QString fileName, int set) {
    printf("Start loading set from %s ...", qPrintable(fileName));

    RsHash users_rs;

    QFile binfile(fileName + ".bin");
    if (!binfile.exists()) {
        QFile file(fileName + ".txt");
        file.open(QFile::ReadOnly);
        QTextStream in(&file);

        int u = 0, i = 0, r = 0;
        QStringList list;
        while (!in.atEnd()) {
            QString line = in.readLine();
//            printf(qPrintable(line+'\n'));
            if (line.contains('|')) {
                list = line.split('|');
                u = list.at(0).toInt();
                QHash<int, float> rs;
                users_rs.insert(u, rs);
            }
            else {
                list = line.split('\t');
                i = list.at(0).toInt();
                if (set == VALID)
                    r = 0;
                else
                    r = list.at(1).toInt();
                users_rs[u].insert(i, r);
            }
        }
        file.close();

        // serialization
        binfile.open(QFile::WriteOnly);
        QDataStream bin(&binfile);
        bin << users_rs;
        binfile.close();
    }
    else {
        // deserialization
        binfile.open(QFile::ReadOnly);
        QDataStream bin(&binfile);
        bin >> users_rs;
        binfile.close();
    }

    printf("OK\n");
    return users_rs;
}

TaxHash load_tracks(QString tracks_fn) {
    printf("Start loading tracks ... ");

    TaxHash tracks;
    QFile file(tracks_fn);
    file.open(QFile::ReadOnly);
    QTextStream in(&file);
    while (!in.atEnd()) {
        QStringList list = in.readLine().split("|");
        QVector<int> track_tax;
        for (int j = 1; j < list.size(); j++) {
            int el = 0;
            if (list[j] == "None")
                el = -1;
            else
                el = list[j].toInt();
            track_tax.append(el);
        }
        tracks.insert(list[0].toInt(), track_tax);
    }
    file.close();
    printf("OK\n");
    return tracks;
}


int main(int argc, char* argv[])
{
    //QCoreApplication a(argc, argv);
    //QCoreApplication::setApplicationName("YahooKDDCup2 Prediction");

    //Sleep(3000);

    printf("Start\n");
    QTime myTimer;
    myTimer.start();
    setbuf(stdout, NULL);

    QString postfix = "";
    if (QString::fromAscii(argv[1]) == "sample")
        postfix = "_sample";
    QString train_file = "../../train";
    train_file.append(postfix);
    QString valid_fn = "../../valid";
    valid_fn.append(postfix);
    QString tracks_file = "../../_trackData.txt";
    QString negatives_train_file = "../../train_negatives";
    negatives_train_file.append(postfix);

    RsHash train = load_set(train_file, TRAIN);
    RsHash valid = load_set(valid_fn, VALID);
//    TaxHash tracks = load_tracks(tracks_file);

//    predict(train, valid, tracks);
//    estimate(valid, valid_fn);

//    check(train, "../i2i_weights");
//    itemnn_pred::study(train, true);
//    itemnn_pred::predict(train, valid, 1, true);
//    estimate(valid, valid_fnrr, true);

//    binsvd_pred::study(train, valid, negatives_train_file, valid_fn, QList<float>(), true);
//    estimate(binsvd_pred::predict(valid, true), valid_fn, true);
    optimize_gd(train, valid, valid_fn, negatives_train_file);

//    optimize_gsect(train, valid, valid_fn, itemnn_pred::get_predictions);
//    optimize_bf(train, valid, valid_fn, itemnn_pred::get_predictions);

    printf("Finished. Exec time: %3.2f sec\n", (float)myTimer.elapsed() / 1000);
    //return a.exec();
    exit(1);
}















