#include "core.h"
#include "estimate.h"
#include "content_pred.h"
#include "itemnn_pred.h"

#ifndef OPTIMIZE_H
#define OPTIMIZE_H

void optimize_bf(RsHash train, RsHash &valid, QString valid_file,
                 RsHash (*get_pred)(RsHash, RsHash &, float));
void optimize_gsect(RsHash train, RsHash &valid, QString valid_file,
                    RsHash (*get_pred)(RsHash, RsHash &, float p) );

#endif // OPTIMIZE_H
