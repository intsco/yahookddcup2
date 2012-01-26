#include "core.h"

#ifndef ITEMNN_PRED_H
#define ITEMNN_PRED_H

namespace itemnn_pred {
    void study(RsHash train, bool verbose);
    void predict(RsHash train, RsHash &valid, float p, bool verbose);
    RsHash get_predictions(RsHash train, RsHash &valid, float p);
}

#endif // ITEMNN_PRED_H
