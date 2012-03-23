#ifndef ITEMNN_PRED_H
#define ITEMNN_PRED_H

#include "core.h"
#include "estimate.h"

namespace itemnn_pred {
    double study(RsHash train, RsHash valid, QString valid_fn, bool verbose);
    RsHash predict(RsHash train, RsHash valid, float p, bool verbose);
    RsHash get_predictions(RsHash train, RsHash &valid, float p);
}

#endif // ITEMNN_PRED_H
