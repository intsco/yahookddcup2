#include "core.h"
#include "estimate.h"
#include "content_pred.h"

#ifndef OPTIMIZE_H
#define OPTIMIZE_H

void optimize(RsHash train, RsHash &valid, QString valid_file, TaxHash tracks);

#endif // OPTIMIZE_H
