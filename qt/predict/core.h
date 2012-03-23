#ifndef MAIN_H
#define MAIN_H

#include <algorithm>
#include <stdlib.h>
//#include <windows.h>
#include <omp.h>
#include <cmath>
#include <sstream>

#include <QtGlobal>

#include <qwaitcondition.h>
#include <QtCore/QCoreApplication>
#include <QHash>
#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QVector>
#include <QPair>
#include <QTime>
#include <QSet>
#include <QSetIterator>
#include <QtAlgorithms>

#define TRAIN 0
#define VALID 1
#define TEST 2

typedef QHash<int, float> UserRs;
typedef QHash<int, QHash<int, float> > RsHash;
typedef QHashIterator<int, QHash<int, float> > RsHashIter;
typedef QMutableHashIterator<int, QHash<int, float> > RsHashIterMut;
typedef QHash<int, QVector<int> > TaxHash;

struct next_less {
    bool operator()(QPair<int, float> const &a, QPair<int, float> const &b) const {
        return a.second > b.second;
    }
};

#endif // MAIN_H
