#ifndef CONTENT_PRED_H
#define CONTENT_PRED_H

#include "core.h"
#include "estimate.h"

namespace content_pred {
    RsHash predict(RsHash train, RsHash valid, TaxHash tracks, float param, bool verbose);
    double study(RsHash, RsHash, TaxHash, QString, QString, QList<float>, bool);
}

#endif // CONTENT_PRED_H
