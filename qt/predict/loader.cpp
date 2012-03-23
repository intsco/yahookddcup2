#include "loader.h"

RsHash load_set(QString fileName, int set) {
    printf("Start loading set from %s ...", qPrintable(fileName));

    RsHash users_rs;

    QFile binfile(fileName + ".bin");
    if (!binfile.exists()) {
        QFile file(fileName + ".txt");
        file.open(QFile::ReadOnly);
        QTextStream in(&file);

        int u = 0, i = 0;
        float r = 0;
        QStringList list;
        while (!in.atEnd()) {
            QString line = in.readLine();
            if (line.simplified().length() > 0)
            {
                if (line.contains('|')) {
                    list = line.split('|');
                    u = list.at(0).toInt();
                    QHash<int, float> rs;
                    users_rs.insert(u, rs);
                }
                else {
                    list = line.split('\t');
                    i = list.at(0).toInt();
                    if (set == VALID or set == TEST)
                        r = 0;
                    else
                        r = list.at(1).toFloat();
                    users_rs[u].insert(i, r);
                }
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
