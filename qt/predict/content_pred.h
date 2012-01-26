#include "core.h"

#ifndef CONTENT_PRED_H
#define CONTENT_PRED_H

namespace content_pred {
    void predict(RsHash train, RsHash &valid, TaxHash tracks, float param, bool verbose);
}

#endif // CONTENT_PRED_H
