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

double study(QList<RsHash> pred_list, QString valid_fn, QList<float> p, bool verbose)
{
    QList<double> coeffs;
    coeffs << 0.5 << 1 - coeffs[0];

    // start predictions blending
    RsHash blend_predict;
    for (RsHash::const_iterator it = pred_list[0].constBegin(); it != pred_list[0].constEnd(); ++it)
    {
        int u = it.key();

        QHash<int, float> blend_rs;
        QHash<int, float>::const_iterator it2;
        for (it2 = it.value().constBegin(); it2 != it.value().constEnd(); it2++)
        {
            blend_rs.insert(it2.value(), 0);
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

    return estimate(blend_predict, valid_fn, "../../blend_pred", true);
}
