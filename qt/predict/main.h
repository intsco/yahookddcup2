#ifndef MAIN_H
#define MAIN_H

#include <algorithm>
#include <stdlib.h>
#include <Windows.h>

#include <QtCore/QCoreApplication>
#include <QHash>
#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QVector>
#include <QPair>

#define TRAIN 0
#define VALID 1

typedef QHash<int, QHash<int, float> > RsHash;
typedef QHashIterator<int, QHash<int, float> > RsHashIter;
typedef QMutableHashIterator<int, QHash<int, float> > RsHashIterMut;
typedef QHash<int, QVector<int> > TaxHash;

QHash<int, QHash<int, int> > load_set();
QHash<int, QVector<int> > load_tracks();
/*void release_set(QHash<int, QHash<int, int> >);
void release_tracks(QHash<int, QVector<int> >);*/
double estimate(QHash<int, QHash<int, int> > , QString);


#endif // MAIN_H
