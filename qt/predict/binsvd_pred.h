#ifndef BINSVD_PRED_H
#define BINSVD_PRED_H

#include "core.h"
#include "estimate.h"

namespace binsvd_pred 
{
    //RsHash study_and_predict(RsHash train, RsHash valid, QString train_neg_fn, QString valid_fn,
    //                                  QList<float> p, bool verbose);
    double study(RsHash train, RsHash valid, QString train_neg_fn, QString valid_fn, QList<float> p,
               bool verbose);
    RsHash predict(RsHash valid, bool verbose);
}

#endif // BINSVD_PRED_H
