#include "binsvd_pred.h"

QHash<int, QVector<float> > user_factors;
QHash<int, QVector<float> > item_factors;

QHash<int, QVector<int> > load_item_users(QHash<int, QVector<int> > user_items)
{
    QHash<int, QVector<int> > item_users;
    QHash<int, QVector<int> >::const_iterator it;
    for(it = user_items.constBegin(); it != user_items.constEnd(); ++it)
    {
        int u = it.key();
        QVector<int> items = it.value();
        QVector<int>::const_iterator it2;
        for(it2 = items.constBegin(); it2 != items.constEnd(); ++it2)
        {
            int i = *it2;
            if(!item_users.contains(i))
            {
                QVector<int> users;
                item_users.insert(i, users);
            }
            item_users[i].append(u);
        }
    }

    return item_users;
}

QHash<int, QVector<int> > load_user_negatives(QString file_name, bool verbose)
{
    QHash<int, QVector<int> > user_negatives;

    if (verbose) printf("Start loading set from %s ... ", qPrintable(file_name));

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
                QVector<int> items;
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
    if (verbose) printf("OK\n");
    return user_negatives;
}

QHash<int, QVector<int> > load_user_positives(RsHash train)
{
    QHash<int, QVector<int> > positives;

    RsHash::const_iterator it;
    for(it = train.constBegin(); it != train.constEnd(); ++it)
    {
        positives[it.key()] = QVector<int>();
        QHash<int, float>::const_iterator it2;
        for(it2 = it.value().constBegin(); it2 != it.value().constEnd(); ++it2)
        {
            if (it2.value() >= 80)
            {
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
    QVector<int> elements;
    if (u_flag)
    {
        elements = train.keys().toVector();
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
        elements = items.keys().toVector();
    }

    // fill factors with small randoms
    QVector<int>::const_iterator it;
    for(it = elements.constBegin(); it != elements.constEnd(); ++it)
    {
        QVector<float> vect(fact_n);
        factors[*it] = vect;
        for(int fi = 0; fi < fact_n; fi++)
        {
            float rand_v = (float)(qrand() % 10) / 100000 + 0.000005;
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

void check_user_negatives(RsHash, UserRs urs, QVector<int> u_pos, QVector<int> u_neg);

int steps = 200, fact_n = 60;
float alfa = 0.01, lambda = 0.01;

double binsvd_pred::study(RsHash train, RsHash valid, QString train_neg_fn, QString valid_fn,
                        QList<float> p, bool verbose)
{
    if (verbose) printf("Start binsvd studying...\n");

    if (!p.empty())
    {
        fact_n = p[0];
        alfa = p[1];
        lambda = p[2];
    }
    double min_err = 100, last_err[4] = {100,100,100,100};
    int min_err_step = 1;

    std::stringstream ss;
    ss<<"../../user_factors_st="<<steps<<"_fn="<<fact_n<<"_a="<<alfa<<"_l="<<lambda;
    QString file_name = QString::fromStdString(ss.str());
    QFile binfile(file_name + ".bin");
    if (!binfile.exists())
    {
        // create and fill data structures
        QHash<int, QVector<int> > user_negatives = load_user_negatives(train_neg_fn, verbose);
        QHash<int, QVector<int> > user_positives = load_user_positives(train);
        create_factors(user_factors, train, fact_n, 1);
        create_factors(item_factors, train, fact_n, 0);

        int un = user_positives.count();
        // by steps
        for (int st = 1; st <= steps; st++)
        {
            if (verbose) printf("%d step: ", st);

            // by users
            QVector<int> users = user_positives.keys().toVector();
#pragma omp parallel for
            for(int ui = 0; ui < un; ++ui)
            {
                int u = users[ui];
                QVector<int> u_pos = user_positives[u];
                QVector<int> u_neg = user_negatives[u];
                if (u_pos.count() != u_neg.count())
                    printf("bugs!\n");

                // by negative and positive items
                for(int r = -1; r <= 1; r += 2)
                {
                    QVector<int> items_list;
                    if (r == -1) items_list = u_neg;
                    else items_list = u_pos;
                    if (u_neg.count() != u_pos.count()) printf("bugs!\n");

                    if (u_neg.count() != u_pos.count()) printf("bugs!\n");
                    //check_user_negatives(train, train[u], u_pos, u_neg);

                    QVector<int>::const_iterator it2;
                    for(it2 = items_list.constBegin(); it2 != items_list.constEnd(); ++it2)
                    {
                        int i = *it2;

                        QVector<float> i_factors = item_factors[i];
                        QVector<float> u_factors = user_factors[u];

                        float pr = dot_product(u_factors, i_factors, fact_n);
                        float err = r - pr;
			

                        // by factor
                        for(int fi = 0; fi < fact_n; fi++)
                        {
                            float user_f = u_factors.value(fi);
                            float item_f = i_factors.value(fi);
                            user_factors[u][fi] = user_f + alfa * (err * item_f - lambda * user_f);
                        }
#pragma omp critical
                        {
                        for(int fi = 0; fi < fact_n; fi++)
                        {

                            float user_f = u_factors.value(fi);
                            float item_f = i_factors.value(fi);
                            item_factors[i][fi] = item_f + alfa * (err * user_f - lambda * item_f);
                        }
                        }
                    }
                }
            }

            double err = estimate(predict(valid, false), valid_fn, false);

            if (err < min_err)
            {
                min_err = err;
                min_err_step = st;
            }
            if (verbose) printf("error %2.3f\n", err);

            last_err[3] = last_err[2];
            last_err[2] = last_err[1];
            last_err[1] = last_err[0];
            last_err[0] = err;
            if (st > 30 and (last_err[0] - last_err[3] > 0.01) ) break;
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
    if (verbose) printf("ok (min error %2.2f on %d step)\n", min_err, min_err_step);
    return min_err;
}

void save_factors() {
    printf("Saving factors to files... ");

    std::stringstream ss;
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

// tmp
UserRs items_hi_r_numb;
void check_user_negatives(RsHash train, UserRs urs, QVector<int> u_pos, QVector<int> u_neg) {
    if (items_hi_r_numb.count() == 0) {
        // collect items and number of hi ratings
        for (RsHash::const_iterator it = train.constBegin(); it != train.constEnd(); ++it)
        {
            UserRs urs = it.value();
            for (UserRs::const_iterator it2 = urs.constBegin(); it2 != urs.constEnd(); ++it2)
            {
                int i = it2.key();
                float r = it2.value();
                if (r >= 80)
                {
                    if(!items_hi_r_numb.contains(i))
                        items_hi_r_numb[i] = 1;
                    else
                        items_hi_r_numb[i] += 1;
                }
            }
        }
    }

    QFile file("../../temp/user_pos_and_neg.csv");
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&file);

    for (int j = 0; j < u_pos.count(); j++)
    {
        out<<u_pos[j]<<";"<<urs[u_pos[j]]<<";"<<u_neg[j]<<";"<<items_hi_r_numb[u_neg[j]]<<"\n";
    }

    file.close();
}

















