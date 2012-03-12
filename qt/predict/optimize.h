#include "core.h"
#include "estimate.h"
#include "content_pred.h"
#include "itemnn_pred.h"
#include "binsvd_pred.h"

#ifndef OPTIMIZE_H
#define OPTIMIZE_H

void optimize_bf(RsHash train, RsHash &valid, QString valid_file,
                 RsHash (*get_pred)(RsHash, RsHash &, float));
//void optimize_gsect(RsHash train, RsHash &valid, QString valid_file,
//                    RsHash (*get_pred)(RsHash, RsHash &, float p) );
void optimize_gsect(RsHash train, RsHash valid, QString train_neg_fn, QString valid_fn,
                    double study(RsHash, RsHash, QString, QString, QList<float>, bool) );
void optimize_gd(RsHash train, RsHash valid, QString valid_fn, QString train_neg_fn);

#endif // OPTIMIZE_H
