#include <QtCore/QCoreApplication>
#include "draw_negatives.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    Sleep(3000);

    printf("Start\n");
    QTime myTimer;
    myTimer.start();
    setbuf(stdout, NULL);

    QString postfix = "";
    if (QString::fromAscii(argv[1]) == "sample") postfix = "_sample";

    QString train_fn = "../../train";
    train_fn.append(postfix);

//    QString valid_fn = "../../valid";
//    valid_fn.append(postfix);

    QString train_neg_fn = "../../train_negatives";
    train_neg_fn.append(postfix);

    QString tracks_fn = "../../_trackData.txt";

    draw_negatives(train_fn, tracks_fn, train_neg_fn);

    printf("Finished. Exec time: %3.2f sec\n", (float)myTimer.elapsed() / 1000);

    return a.exec();
}
