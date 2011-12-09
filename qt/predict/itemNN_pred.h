#include "core.h"

#ifndef ITEMNN_PRED_H
#define ITEMNN_PRED_H
namespace itemnn_pred {
    void study(RsHash train, bool verbose);
    void predict(RsHash train, RsHash &valid, float param, bool verbose);
}

#endif // ITEMNN_PRED_H
