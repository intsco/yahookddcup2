#ifndef BINSVD_PRED_H
#define BINSVD_PRED_H

#include "core.h"
#include "estimate.h"

namespace binsvd_pred {
    void study(RsHash train, RsHash valid, QString train_neg_fn, QString valid_fn, bool verbose);
    RsHash predict(RsHash valid, bool verbose);
}

#endif // BINSVD_PRED_H
