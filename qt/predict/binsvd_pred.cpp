#include "binsvd_pred.h"

using namespace std;

QHash<int, QVector<float> > user_factors;
QHash<int, QVector<float> > item_factors;

QHash<int, QList<int> > load_item_users(QHash<int, QList<int> > user_items)
{
    QHash<int, QList<int> > item_users;
    QHash<int, QList<int> >::const_iterator it;
    for(it = user_items.constBegin(); it != user_items.constEnd(); ++it)
    {
        int u = it.key();
        QList<int> items = it.value();
        QList<int>::const_iterator it2;
        for(it2 = items.constBegin(); it2 != items.constEnd(); ++it2)
        {
            int i = *it2;
            if(!item_users.contains(i))
            {
                QList<int> users;
                item_users.insert(i, users);
            }
            item_users[i].append(u);
        }
    }

    return item_users;
}

QHash<int, QList<int> > load_user_negatives(QString file_name)
{
    QHash<int, QList<int> > user_negatives;

    printf("Start loading set from %s ... ", qPrintable(file_name));

    QFile binfile(file_name + ".bin");
    if (!binfile.exists())
    {
        QFile file(file_name + ".txt");
        file.open(QFile::ReadOnly);
        QTextStream in(&file);

        int u = 0, i = 0;
        QStringList list;
        while (!in.atEnd())
        {
            QString line = in.readLine();
//            printf(qPrintable(line+'\n'));
            if (line.contains('|'))
            {
                list = line.split('|');
                u = list.at(0).toInt();
                QList<int> items;
                user_negatives.insert(u, items);
            }
            else
            {
                list = line.split('\t');
                i = list.at(0).toInt();
                user_negatives[u].append(i);
            }
        }
        file.close();

        // serializationRsHash
        binfile.open(QFile::WriteOnly);
        QDataStream bin(&binfile);
        bin << user_negatives;
        binfile.close();
    }
    else
    {
        // deserialization
        binfile.open(QFile::ReadOnly);
        QDataStream bin(&binfile);
        bin >> user_negatives;
        binfile.close();
    }
    printf("OK\n");
    return user_negatives;
}

QHash<int, QList<int> > load_user_positives(RsHash train)
{
    QHash<int, QList<int> > positives;

    RsHash::const_iterator it;
    for(it = train.constBegin(); it != train.constEnd(); ++it)
    {
        positives[it.key()] = QList<int>();
        QHash<int, float>::const_iterator it2;
        for(it2 = it.value().constBegin(); it2 != it.value().constEnd(); ++it2)
        {
            if (it2.value() >= 80) {
                positives[it.key()].append(it2.key());
            }
        }
    }

    return positives;
}

void create_factors(QHash<int, QVector<float> > &factors, RsHash train, int fact_n, bool u_flag)
{
    QTime time = QTime::currentTime();
    qsrand((uint)time.msec());

    // fill all user ids
    QList<int> elements;
    if (u_flag)
    {
        elements = train.keys();
    }
    // fill all item ids
    else
    {
        QHash<int, int> items;
        RsHash::const_iterator it;
        for(it = train.constBegin(); it != train.constEnd(); ++it)
        {
            QHash<int, float>::const_iterator it2;
            for(it2 = it.value().constBegin(); it2 != it.value().constEnd(); ++it2)
            {
                if (!items.contains(it2.key()))
                {
                    items[it2.key()] = 0;
                }
            }
        }
        elements = items.keys();
    }

    // fill factors with small randoms
    QList<int>::const_iterator it;
    for(it = elements.constBegin(); it != elements.constEnd(); ++it)
    {
        QVector<float> vect(fact_n);
        factors[*it] = vect;
        for(int fi = 0; fi < fact_n; fi++)
        {
            float rand_v = (float)(qrand() % 10) / 1000 + 0.0005;
            factors[*it][fi] = rand_v;
        }
        if (factors[*it].count() < fact_n)
            printf("%d  ", factors[*it].count());
    }
}

float dot_product(QVector<float> u_f, QVector<float> i_f, int fact_n)
{
    float dot_prod = 0;
    for(int i = 0; i < fact_n; i++)
    {
        dot_prod += u_f[i] * i_f[i];
    }
    return dot_prod;
}


void save_factors();
RsHash binsvd_pred::predict(RsHash valid, bool verbose);

int steps = 300, fact_n = 10;
float alfa = 0.1, lambda = 0.1;

void binsvd_pred::study(RsHash train, RsHash valid, QString train_neg_fn, QString valid_fn, bool verbose)
{
    if (verbose) printf("Start binsvd studying...\n");

    double min_err = 1, last_err[4] = {1, 1, 1, 1};
    int min_err_step = 1;

    stringstream ss;
    ss<<"../../user_factors_st="<<steps<<"_fn="<<fact_n<<"_a="<<alfa<<"_l="<<lambda;
    QString file_name = QString::fromStdString(ss.str());
    QFile binfile(file_name + ".bin");
    if (!binfile.exists())
    {
        // create and fill data structures
        QHash<int, QList<int> > user_negatives = load_user_negatives(train_neg_fn);
        QHash<int, QList<int> > user_positives = load_user_positives(train);
        QHash<int, QList<int> > item_negatives = load_item_users(user_negatives);
        QHash<int, QList<int> > item_positives = load_item_users(user_positives);
        create_factors(user_factors, train, fact_n, 1);
        create_factors(item_factors, train, fact_n, 0);

        int un = user_positives.count();
        // by step
        for (int st = 1; st <= steps; st++)
        {
            if (verbose) printf("%d step: ", st);
            int j = 0;

            // update USER factors
            QList<int> users = user_positives.keys();
#pragma omp parallel for
            for(int ui = 0; ui < un; ++ui)
            {
                int u = users[ui];
                QList<int> u_pos = user_positives[u];
                QList<int> u_neg = user_negatives[u];

                // by negative and positive items
                for(int r = -1; r <= 1; r += 2)
                {
                    QList<int> items_list;
                    if (r == -1) items_list = u_neg;
                    else items_list = u_pos;

                    QList<int>::const_iterator it2;
                    for(it2 = items_list.constBegin(); it2 != items_list.constEnd(); ++it2)
                    {
                        int i = *it2;

                        if (user_factors[u].count() < fact_n || item_factors[i].count() < fact_n)
                        {
                            printf("%d  %d,  %d  %d\n", u, user_factors[u].count(), i, item_factors[i].count());
                        }

                        float pr = dot_product(user_factors[u], item_factors[i], fact_n);
                        float err = r - pr;

                        // by factor
                        for(int fi = 0; fi < fact_n; fi++)
                        {

                            float user_f = user_factors[u].value(fi);
                            float item_f = item_factors[i].value(fi); // <- some problems!
                            user_factors[u][fi] = user_f + alfa * (err * item_f - lambda * user_f);                            
                            //item_factors[i][fi] = item_f + alfa * (err * user_f - lambda * item_f);
                        }
                    }
                }
//#pragma omp critical
//                {
//                j++;
//                if (verbose && j % 100 == 0)
//                    printf("%d users and %2.2f %% processed\r", j, (float)j / un * 100);
//                }
            }

            // update ITEM factors
//            printf("\n");
            j = 0;
            QList<int> items = item_negatives.keys().toSet().unite(item_positives.keys().toSet()).toList();
            int in = item_positives.count();
#pragma omp parallel for
            for(int ii = 0; ii < in; ++ii)
            {
                int i = items[ii];
                QList<int> i_pos = item_positives.value(i, QList<int>());
                QList<int> i_neg = item_negatives.value(i, QList<int>());

                // by negative and positive users
                for(int r = -1; r <= 1; r += 2)
                {
                    QList<int> users_list;
                    if (r == -1) users_list = i_neg;
                    else users_list = i_pos;

                    QList<int>::const_iterator it2;
                    for(it2 = users_list.constBegin(); it2 != users_list.constEnd(); ++it2)
                    {
                        int u = *it2;

                        if (user_factors[u].count() < fact_n || item_factors[i].count() < fact_n)
                        {
                            printf("%d  %d,  %d  %d\n", u, user_factors[u].count(), i, item_factors[i].count());
                        }

                        float pr = dot_product(user_factors[u], item_factors[i], fact_n);
                        float err = r - pr;

                        // by factor
                        for(int fi = 0; fi < fact_n; fi++)
                        {
                            float user_f = 0;
                            float item_f = 0;
                            user_f = user_factors[u].value(fi);
                            item_f = item_factors[i].value(fi);
                            //user_factors[u][fi] = user_f + alfa * (err * item_f - lambda * user_f);
                            item_factors[i][fi] = item_f + alfa/st * (err * user_f - lambda * item_f);
                        }
                    }
                }
//#pragma omp critical
//                {
//                j++;
//                if (verbose && j % 100 == 0)
//                    printf("%d items and %2.2f %% processed\r", j, (float)j / in * 100);
//                }
            }

            double err = estimate(predict(valid, false), valid_fn, false);

            if (err < min_err)
            {
                min_err = err;
                min_err_step = st;
            }
            if (verbose) printf("error %2.3f\n", err*100);

            last_err[3] = last_err[2];
            last_err[2] = last_err[1];
            last_err[1] = last_err[0];
            last_err[0] = err;
            if (st > 30 and ((abs(last_err[1] - last_err[0]) < 0.0001 and abs(last_err[2] - last_err[1]) < 0.0001 and abs(last_err[3] - last_err[2]) < 0.0001)
                             or last_err[0] - last_err[3] > 0.01))
                break;
        }
        //save_factors();
    }
    else
    {
        binfile.open(QFile::ReadOnly);
        QDataStream bin(&binfile);
        bin >> user_factors;
        binfile.close();

        QFile binfile2(file_name.replace("user", "item") + ".bin");
        binfile2.open(QFile::ReadOnly);
        QDataStream bin2(&binfile2);
        bin2 >> item_factors;
        binfile2.close();
    }
    if (verbose) printf("ok (min error %2.2f on %d step)\n", min_err*100, min_err_step);
}

void save_factors() {
    printf("Saving factors to files... ");

    stringstream ss;
    ss<<"../../user_factors_st="<<steps<<"_fn="<<fact_n<<"_a="<<alfa<<"_l="<<lambda;
    QString file_name = QString::fromStdString(ss.str());

    QFile binfile(file_name + ".bin");
    binfile.open(QFile::WriteOnly);
    QDataStream bin(&binfile);
    bin << user_factors;
    binfile.close();

    file_name = file_name.replace("user", "item");

    QFile binfile2(file_name + ".bin");
    binfile2.open(QFile::WriteOnly);
    QDataStream bin2(&binfile2);
    bin2 << item_factors;
    binfile2.close();

    printf("ok\n");
}

RsHash binsvd_pred::predict(RsHash valid, bool verbose)
{
    if (verbose) printf("Binsvd predicting...\n");

    RsHash new_valid(valid);

    // iterate through users (valid)
    RsHashIter uit(valid);
    int users_n = valid.size(), n = 0;
    while (uit.hasNext()) {
        uit.next();
        n++;
        int u = uit.key();
        // iterate through user valid ratings
        QHashIterator<int, float> iit(uit.value());
        while (iit.hasNext()) {
            iit.next();
            int i = iit.key();

            // compute "rating" as dot product of factor vectors
            float r = 0;
            QVector<float> v;
            if (!item_factors.contains(i))
            {
                for(int j = 0; j < user_factors[u].count(); j++)
                {
                    v.append(1);
                }
                r = dot_product(user_factors[u], v, user_factors[u].count());
            }
            else
                r = dot_product(user_factors[u], item_factors[i], user_factors[u].count());
            new_valid[u][i] = r;
        }
        if (verbose) printf("%3.3f %% complited\r", float(n) / users_n * 100);
    }
    valid = new_valid;

    if (verbose) printf("ok\n");

    return valid;
}

















