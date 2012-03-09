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

typedef QHash<int, double> UserRs;
typedef QHash<int, QHash<int, double> > RsHash;
typedef QHashIterator<int, QHash<int, double> > RsHashIter;
typedef QMutableHashIterator<int, QHash<int, double> > RsHashIterMut;
typedef QHash<int, QVector<int> > TaxHash;

#endif // MAIN_H
