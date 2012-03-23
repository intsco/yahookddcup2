#ifndef BLEND_PRED_H
#define BLEND_PRED_H

#include "core.h"
#include "estimate.h"

namespace blend_pred {
    double study(QList<RsHash> pred_list, QString valid_fn, QList<float> p, bool verbose);
    RsHash predict(RsHash valid, bool verbose);
}

#endif // BLEND_PRED_H
