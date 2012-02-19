#include "draw_negatives.h"

QVector< QPair<int, float> > get_items_prob_sums(RsHash train)
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
            float r = it2.value();
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

    // compute item high rate probabilities
    for(UserRs::iterator it = items_hi_r_prob.begin(); it != items_hi_r_prob.end(); ++it)
    {
        float p = it.value() / hi_rs_sum;
        int i = it.key();
        items_hi_r_prob[i] = p;
    }

    // covert QHash with probs to QList with prob sums
    double probs_sum = 0;
    QVector< QPair<int, float> > items_prob_sums;
    for(UserRs::const_iterator it = items_hi_r_prob.constBegin(); it != items_hi_r_prob.constEnd(); ++it)
    {
        int i = it.key();
        probs_sum += it.value();
        QPair<int, float> item_sum_pair(i, probs_sum);
        items_prob_sums.append(item_sum_pair);
    }

    printf("OK\n");
    return items_prob_sums;
}

int get_hi_rated_items(UserRs urs)
{
    int n = 0;
    for(UserRs::const_iterator it = urs.constBegin(); it != urs.constEnd(); ++it)
    {
        if (it.value() >= 80)
            n += 1;
    }
    return n;
}

bool has_rated(UserRs urs, int neg_item, int neg_item_album, int neg_item_artist)
{
    if (urs.contains(neg_item) /*or urs.contains(neg_item_album) or urs.contains(neg_item_artist)*/)
        return true;

    return false;
}

//bool contain_neg_item(UserRs user_neg, int neg_item, int hi_r_items_numb)
//{
//    if (hi_r_items_numb >= 10000)
//        return false;
//    else
//        return user_neg.contains(neg_item);
//}

//UserRs get_muchrs_user_rand_neg(QVector< QPair<int, float> > items_prob_sums, int hi_r_items_numb)
//{
//    UserRs user_neg;
//    qsrand(QTime::currentTime().msec());
//    int n = items_prob_sums.count();

//    while (user_neg.count() < hi_r_items_numb)
//    {
//        int x = qrand() % n;
//        int neg_item = items_prob_sums[x].first;
//        user_neg[neg_item] = 0.0;
//    }
//    return user_neg;
//}

UserRs get_user_rand_negatives(UserRs urs, QVector< QPair<int, float> > items_prob_sums, TaxHash tracks)
{
    UserRs user_neg;

    QVector<int> emptyTax;
    emptyTax.append(-1);
    emptyTax.append(-1);

    qsrand(QTime::currentTime().msec());

    int items_numb = items_prob_sums.count();
    int hi_r_items_numb = get_hi_rated_items(urs);
    int n = items_prob_sums.count();

//    if (hi_r_items_numb >= 10000)
//        user_neg = get_muchrs_user_rand_neg(items_prob_sums, hi_r_items_numb);
//    else
    int neg_item_vars_numb = 0;
    while (user_neg.count() < hi_r_items_numb)
    {
        int neg_item = -1;

        if (neg_item_vars_numb > hi_r_items_numb * 10)
        {
            // simple random selection of negative item
            float x = qrand() % n;
            neg_item = items_prob_sums[x].first;
        }
        else
        {
            // binary search of a random negative item
            float hi = items_numb, low = 0;
            float x = ((float)qrand()) / RAND_MAX;
            while (low < hi)
            {
                int mid = (low + hi) / 2;
                float midval = items_prob_sums[mid].second, prevmidval = 0;
                if (mid > 0)
                    prevmidval = items_prob_sums[mid-1].second;
                else
                    prevmidval = 0;

                if (midval < x)
                    low = mid+1;
                else if (prevmidval > x)
                    hi = mid;
                else
                {
                    neg_item = items_prob_sums[mid].first;
                    break;
                }
            }
        }

        // checking if neg item is suitable
        int neg_alb = tracks.value(neg_item, emptyTax)[0];
        int neg_art = tracks.value(neg_item, emptyTax)[1];
        if (!has_rated(urs, neg_item, neg_alb, neg_art)
                and !user_neg.contains(neg_item)
                and neg_item >= 0)
            user_neg[neg_item] = 0.0;

        neg_item_vars_numb += 1;
    }

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

    RsHash train = load_set(train_fn, TRAIN);
    TaxHash tracks = load_tracks(tracks_fn);
    QVector< QPair<int, float> > items_prob_sums = get_items_prob_sums(train);

    int n = train.count(), j = 0;
    RsHash train_neg;
    for (RsHash::const_iterator it = train.constBegin(); it != train.constEnd(); ++it)
    {
        int u = it.key();
        UserRs neg_rs = get_user_rand_negatives(it.value(), items_prob_sums, tracks);
        train_neg[u] = neg_rs;
//        printf("%d  %d\n", u, j);
        j += 1;
        if (j % 1 == 0)
            printf("%d users and %2.2f %% processed\r", j, ((float)j) / n * 100);
    }
    printf("\n");

    save_negatives(train_neg, train_neg_fn);

    printf("finished\n");
}

















