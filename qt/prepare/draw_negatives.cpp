#include "draw_negatives.h"

struct prev_more {
    bool operator()(QPair<int, int> const &a, QPair<int, int> const &b) const {
        return a.second > b.second;
    }
};

QVector< QPair<int, int> > get_items_numbs(RsHash train)
{
    printf("getting item high rate probabilities...");

    UserRs items_hi_r_prob;
    int hi_rs_sum = 0;

    // collect items and number of hi ratings
    for (RsHash::const_iterator it = train.constBegin(); it != train.constEnd(); ++it)
    {
        UserRs urs = it.value();
        for (UserRs::const_iterator it2 = urs.constBegin(); it2 != urs.constEnd(); ++it2)
        {
            int i = it2.key();
            double r = it2.value();
            if (r >= 80)
            {
                if(!items_hi_r_prob.contains(i))
                    items_hi_r_prob[i] = 1;
                else
                    items_hi_r_prob[i] += 1;
                hi_rs_sum += 1;
            }
        }
    }

    // compute item numbers
//    for(UserRs::iterator it = items_hi_r_prob.begin(); it != items_hi_r_prob.end(); ++it)
//    {
//        double p = it.value();
//        int i = it.key();
//        items_hi_r_prob[i] = p;
//    }

    // covert QHash with item numbs to QList with item numbs
//    double probs_sum = 0;
    QList<int> items = items_hi_r_prob.keys();
    QVector< QPair<int, int> > items_numbs;
    do
    {
        int i = items[rand() % items.count()];
        int numb = qRound(items_hi_r_prob[i]);
        QPair<int, int> item_numb(i, numb);
        items_numbs.append(item_numb);
        items.removeOne(i);
    } while(items.count() > 0);

//    QVector< QPair<int, int> > items_numbs;
//    for(UserRs::const_iterator it = items_hi_r_prob.constBegin(); it != items_hi_r_prob.constEnd(); ++it)
//    {
//        int i = it.key();
//        int numb = qRound(it.value());
//        QPair<int, int> item_numb(i, numb);
//        items_numbs.append(item_numb);
//    }

    // sort items by numbs
    std::sort(items_numbs.begin(), items_numbs.end(), prev_more());

    printf("OK\n");
    return items_numbs;
}

QHash<int, int> get_user_hi_rated(RsHash train)
{
    QHash<int, int> u_hi_rs;
    // user hi rated items number
    for (RsHash::const_iterator it = train.constBegin(); it != train.constEnd(); ++it)
    {
        int u = it.key();
        u_hi_rs.insert(u, 0);
        UserRs urs = it.value();
        for (UserRs::const_iterator it2 = urs.constBegin(); it2 != urs.constEnd(); ++it2)
        {
            if (it2.value() >= 80)
                u_hi_rs[u] += 1;
        }
    }
    return u_hi_rs;
}

bool has_rated(UserRs urs, int neg_item, int neg_item_album, int neg_item_artist)
{
    if (urs.contains(neg_item))/* or urs.contains(neg_item_album) or urs.contains(neg_item_artist))*/
        return true;

    return false;
}

UserRs get_user_rand_negatives(UserRs urs, QVector< QPair<int, int> > items_numbs, TaxHash tracks)
{
    UserRs user_neg;

    QVector<int> emptyTax;
    emptyTax.append(-1);
    emptyTax.append(-1);

    QVector< QPair<int, int> >::iterator it;
    for(it = items_numbs.begin(); it != items_numbs.end(); ++it)
    {
        QPair<int, int> pair = *it;
        for (int j = 0; j < pair.second; j++)
        {

        }
    }

//    int items_numb = items_prob_sums.count();
//    int hi_r_items_numb = get_hi_rated_items(urs);
//    int n = items_prob_sums.count();

//    int neg_item_vars_numb = 0;
//    while (user_neg.count() < hi_r_items_numb)
//    {
//        int neg_item = -1;

//        if (neg_item_vars_numb > hi_r_items_numb * 10)
//        {
//            // simple random selection of negative item
//            float x = qrand() % n;
//            neg_item = items_prob_sums[x].first;
//        }
//        else
//        {
//            // binary search of a random negative item
//            double hi = items_numb, low = 0;
//            double x = ((float)rand()) / RAND_MAX;
//            while (low < hi)
//            {
//                int mid = (low + hi) / 2;
//                float midval = items_prob_sums[mid].second, prevmidval = 0;
//                if (mid > 0)
//                    prevmidval = items_prob_sums[mid-1].second;
//                else
//                    prevmidval = 0;

//                if (midval < x)
//                    low = mid+1;
//                else if (prevmidval > x)
//                    hi = mid;
//                else
//                {
//                    neg_item = items_prob_sums[mid].first;
//                    break;
//                }
//            }
//        }

//        // checking if neg item is suitable
//        int neg_alb = tracks.value(neg_item, emptyTax)[0];
//        int neg_art = tracks.value(neg_item, emptyTax)[1];
//        if (!has_rated(urs, neg_item, neg_alb, neg_art)
//                and !user_neg.contains(neg_item)
//                and neg_item >= 0)
//            user_neg[neg_item] = 0.0;

//        neg_item_vars_numb += 1;
//    }

    return user_neg;
}

void save_negatives(RsHash train_neg, QString train_neg_fn)
{
    printf("saving negatives...");

    QFile file(train_neg_fn + ".txt");
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&file);

    for (RsHash::const_iterator it = train_neg.constBegin(); it != train_neg.constEnd(); ++it)
    {
        out << QString::number(it.key()) << '|' << QString::number(it.value().count()) << '\n';
        UserRs urs = it.value();
        for (UserRs::const_iterator it2 = urs.constBegin(); it2 != urs.constEnd(); ++it2)
        {
            out << QString::number(it2.key()) << '\t' << QString::number(it2.value()) << '\n';
        }
    }

    file.close();
    printf("OK\n");
}

void draw_negatives(QString train_fn, QString tracks_fn, QString train_neg_fn)
{
    printf("drawing negatives...\n");
//    qsrand(QTime::currentTime().msec());
    rand();

    RsHash train = load_set(train_fn, TRAIN);
    QList<int> users = train.keys();
    QHash<int, int> users_hi_rs_numb = get_user_hi_rated(train);
    RsHash train_neg;
    for (int j = 0; j < users.count(); j++)
    {
        QHash<int, double> u_rs;
        train_neg.insert(users[j], u_rs);
    }

    TaxHash tracks = load_tracks(tracks_fn);
    QVector< QPair<int, int> > items_numbs = get_items_numbs(train);


    QVector<int> emptyTax;
    emptyTax.append(-1);
    emptyTax.append(-1);

    int k = 0, items_n = items_numbs.count();
    QVector< QPair<int, int> >::iterator it;
    for(it = items_numbs.begin(); it != items_numbs.end(); ++it)
    {
        QPair<int, int> pair = *it;
        int neg_item = pair.first;
        int neg_alb = tracks.value(neg_item, emptyTax)[0];
        int neg_art = tracks.value(neg_item, emptyTax)[1];
        for (int j = 0; j < pair.second; j++)
        {
            int f = 0;
            while(true)
            {
                f += 1;
                if (f > 10000)
                    break;

                int u = users[rand() % users.count()];
                int u_hi_rs_numb = users_hi_rs_numb[u];
                QHash<int, double> u_neg_set = train_neg[u];

                if (!has_rated(train[u], neg_item, neg_alb, neg_art)
                        and !u_neg_set.contains(neg_item)
                        and u_hi_rs_numb > u_neg_set.count())
                {
//                    printf("\n%d  %d\n", u_hi_rs_numb, train_neg[u].count());
                    train_neg[u][neg_item] = 0;
                    break;
                }
                if (u_hi_rs_numb == u_neg_set.count())
                    users.removeOne(u);
            }
            if (f > 10000) break;
        }
        k += 1;
        if (k % 10 == 0)
            printf("%d items and %2.2f %% processed\r", k, ((float)k) / items_n * 100);
    }
    // appending additional items
    printf("\nappending additional items to %d users...\n", users.count());
    while (users.count() > 0)
    {
        int neg_item = items_numbs[rand() % items_numbs.count()].first;
        int neg_alb = -1;//tracks.value(neg_item, emptyTax)[0];
        int neg_art = -1;//tracks.value(neg_item, emptyTax)[1];
        int u = users[rand() % users.count()];
        if (!has_rated(train[u], neg_item, neg_alb, neg_art)
                and !train_neg[u].contains(neg_item)
                and users_hi_rs_numb[u] > train_neg[u].count())
        {
            train_neg[u][neg_item] = 0;
        }
        if (users_hi_rs_numb[u] == train_neg[u].count())
            users.removeOne(u);
    }

    printf("\n");

//    int j = 0;
//    RsHash train_neg;
//    for (RsHash::const_iterator it = train.constBegin(); it != train.constEnd(); ++it)
//    {
//        int u = it.key();
//        UserRs neg_rs = get_user_rand_negatives(it.value(), items_prob_sums, tracks);
//        train_neg[u] = neg_rs;


//        j += 1;
//        if (j % 1 == 0)
//            printf("%d users and %2.2f %% processed\r", j, ((float)j) / n * 100);
//    }
//    printf("\n");

    save_negatives(train_neg, train_neg_fn);

    printf("finished\n");
}

















