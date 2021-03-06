#include "estimate.h"

struct prev_less {
    bool operator()(QPair<int, float> const &a, QPair<int, float> const &b) const {
        return a.second < b.second;
    }
};
struct prev_more {
    bool operator()(QPair<int, float> const &a, QPair<int, float> const &b) const {
        return a.second > b.second;
    }
};

QVector<QPair<int, float> > get_sorted_items(QHash<int, float> rs, bool desc) {
    QVector<QPair<int, float> > items_rs;
    QHashIterator<int, float> it(rs);
    while (it.hasNext()) {
        it.next();
        QPair<int, float> item_r;
        item_r.first = it.key();
        item_r.second = it.value();
        items_rs.append(item_r);
    }

    if (desc)
        std::sort(items_rs.begin(), items_rs.end(), prev_more());
    else
        std::sort(items_rs.begin(), items_rs.end(), prev_less());

    return items_rs;
}

void save_predictions(RsHash valid, QString fn)
{
    printf("Saving predictions to %s.txt file... ", qPrintable(fn));

    QFile file(fn + ".txt");
    file.open(QFile::WriteOnly);
    QTextStream st(&file);

    for (RsHash::const_iterator it = valid.begin(); it != valid.end(); it++)
    {
        int u = it.key();
        st << u << "|6\n";

        QHash<int, float> u_rs = it.value();
        QVector< QPair<int, float> > sort_rs = get_sorted_items(u_rs, false);
        QVector< QPair<int, float> >::const_iterator it2;
        int rank = 1;
        for (it2 = sort_rs.begin(); it2 != sort_rs.end(); it2++)
        {
            int i = (*it2).first;
            float r = rank;
            st << i << "\t" << r << "\n";
            rank++;
        }
    }
    file.close();
    printf("ok\n");
}

double estimate(RsHash valid, QString valid_fn, QString save_pred_fn, bool verbose)
{
    if (verbose) printf("Estimating prediction results...\n");

    // save predictions
    if (save_pred_fn.length() > 0) save_predictions(valid, save_pred_fn);

    // iterate through users
    RsHashIterMut it(valid);
    while (it.hasNext())
    {
        it.next();
        /*QHash<int, float>::const_iterator iter;
        for (iter = it.value().begin(); iter != it.value().end(); ++iter)
            printf("%d  %f\n", iter.key(), iter.value());*/

        // classifing predictions into 2 groups
        QVector<QPair<int, float> > sort_items = get_sorted_items(it.value(), false);
        for (int j = 0; j < 6; j++)
        {
            int i = sort_items[j].first;
            if (j < 3)
                it.value().insert(i, -1);
            else
                it.value().insert(i, 1);
        }
    }

    double err = 0;
    int n = 0, u = 0, i = 0;
    float real_r = 0, pred_r = 0;

    // comparing prediction results and real data from the file
    QFile file(valid_fn + ".txt");
    file.open(QFile::ReadOnly);
    QTextStream in(&file);
    while (!in.atEnd())
    {
        QString line = in.readLine();
        QStringList list;
        if (line.contains("|"))
        {
            list = line.split("|");
            u = list[0].toInt();
        }
        else
        {
            list = line.split("\t");
            i = list[0].toInt();
            real_r = list[1].toFloat();
            pred_r = valid[u][i];
            if (real_r != -1) real_r = 1;
            if (pred_r != -1) pred_r = 1;
            if (real_r != pred_r)
            {
                err += 1;
            }
            n += 1;
        }
    }
    err /= n;
    err *= 100;
    if (verbose) printf("Error rate: %3.3f, n=%d\n", err, n);
    file.close();
    return err;
}












