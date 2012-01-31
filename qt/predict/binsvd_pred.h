#ifndef BINSVD_PRED_H
#define BINSVD_PRED_H

#include "core.h"

namespace binsvd_pred {
    void study(RsHash train, bool verbose);
    void predict(RsHash train, RsHash &valid, bool verbose);
}

#endif // BINSVD_PRED_H
