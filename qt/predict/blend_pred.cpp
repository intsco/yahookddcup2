#include "blend_pred.h"

QVector< QPair<int, float> > get_ranks(QHash<int, float> rs_hash)
{
    QVector< QPair<int, float> > rs;
    QHash<int, float>::const_iterator rs_it;
    for (rs_it = rs_hash.constBegin(); rs_it != rs_hash.constEnd(); rs_it++)
    {
        rs.append( QPair<int, float>(rs_it.key(), rs_it.value()) );
    }
    std::sort(rs.begin(), rs.end(), next_less());

    for (int j = 0; j < 6; j++)
    {
        rs[j].second = 6 - j;
    }

    return rs;
}

double blend_pred::study(QList<RsHash> pred_list, QString valid_fn, QList<float> p, bool verbose)
{
    printf("Start predictions blending of %d models...\n", pred_list.count());

    float err = 100, minerr = 100;
    QList<float> coeffs;
    coeffs << 0 << 0;
    QList<float> min_coeffs;
    min_coeffs << 0 << 0;

    for (int m = 1; m <= 100; m+=1)
    {
        coeffs[0] = (float)m / 100;
        coeffs[1] = 1 - coeffs[0];

        // start predictions blending
        RsHash blend_predict;
        QList<int> keys = pred_list[0].keys();
        for (int j = 0; j < keys.count(); j++)
        {
            int u = keys[j];
            QHash<int, float> rs = pred_list[0][u];

            QHash<int, float> blend_rs;
            QHash<int, float>::const_iterator it2;
            for (it2 = rs.constBegin(); it2 != rs.constEnd(); it2++)
            {
                blend_rs.insert(it2.key(), 0);
            }

            for (int j = 0; j < pred_list.count(); j++)
            {
                QVector< QPair<int, float> > ranks = get_ranks(pred_list[j][u]);
                for (int k = 0; k < 6; k++)
                {
                    blend_rs[ranks[k].first] += coeffs[j] * ranks[k].second;
                }
            }
            blend_predict.insert(u, blend_rs);
        }

        // estimate blending results
        err = estimate(blend_predict, valid_fn, "", false);
        if (err < minerr)
        {
            minerr = err;
            min_coeffs = coeffs;
        }
        printf("err=%2.4f, c1=%2.4f, c2=%2.4f\n", err, coeffs[0], coeffs[1]);
    }

    printf("min err=%2.4f, c1=%2.4f, c2=%2.4f\n", minerr, min_coeffs[0], min_coeffs[1]);

    return minerr;
}









