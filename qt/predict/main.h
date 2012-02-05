#include "core.h"
#include "content_pred.h"
#include "itemnn_pred.h"
#include "optimize.h"
#include "binsvd_pred.h"

#ifndef MAIN_H
#define MAIN_H

RsHash load_set(QString fileName, int set);
TaxHash load_tracks(QString tracks_fn);

#endif // MAIN_H
