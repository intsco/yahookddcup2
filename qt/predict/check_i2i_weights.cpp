#include "core.h"

void check(RsHash train, QString i2i_file) {
    QHash<QPair<int, int>, float> i2i_weights;

    printf("Loading from bin file... ");
    QFile binfile("../../i2i_weights.bin");
    binfile.open(QFile::ReadOnly);
    QDataStream bin(&binfile);
    bin >> i2i_weights;
    binfile.close();
    printf("i2i_size = %d\n", i2i_weights.count());

    printf("Choosing test items...\n");
    int n = 10;
    QHash< QPair<int, int>, float>  items_pairs;
    QHash<int, QList<int> > items_users;
    for (int k = 0; k < n; k++) {
        QPair<int, int> pair;
        do {
            pair.first = qrand() % (280000 + 1);
            pair.second = qrand() % (280000 + 1);
            //printf("%d  %d  %d\n", pair.first, pair.second, i2i_weights.contains(pair));
        } while (!i2i_weights.contains(pair));
        items_pairs[pair] = i2i_weights[pair];
        items_users[pair.first] = QList<int>();
        items_users[pair.second] = QList<int>();
    }

    printf("Getting test items user sets...\n");
    QHash<int, QHash<int, float> >::const_iterator it;
    for(it = train.constBegin(); it != train.constEnd(); ++it) {
        int u = it.key();
        QHash<int, float>::const_iterator it2 = it.value().constBegin();
        while(it2 != it.value().constEnd()) {
            ++it2;
            int i = it2.key();
            if (items_users.contains(i)) {
                items_users[i].append(u);
            }
        }
    }

    printf("Writing test data to file...\n");
    QFile file("../../check_i2i_weights.csv");
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&file);

    QHashIterator< QPair<int, int>, float> iter(items_pairs);
    while(iter.hasNext()) {
        iter.next();
        out << iter.key().first << ";" << iter.key().second << ";" << items_pairs[iter.key()] << "\n";

        QList<int> i_users = items_users[iter.key().first];
        QListIterator<int> iter2(i_users);
        while(iter2.hasNext()) {
            int u = iter2.next();
            out << u << ';';
        }
        out << '\n';

        QList<int> j_users = items_users[iter.key().second];
        QListIterator<int> iter3(j_users);
        while(iter3.hasNext()) {
            int u = iter3.next();
            out << u << ';';
        }
        out << "\n\n";
    }

    file.close();
}










