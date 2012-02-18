#ifndef LOADER_H
#define LOADER_H

#include "core.h"

RsHash load_set(QString fileName, int set);
TaxHash load_tracks(QString tracks_fn);

#endif // LOADER_H
