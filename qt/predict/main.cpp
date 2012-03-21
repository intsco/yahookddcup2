#include "core.h"
#include "content_pred.h"
#include "itemnn_pred.h"
#include "optimize.h"
#include "binsvd_pred.h"
#include "loader.h"

int main(int argc, char* argv[])
{
    //QCoreApplication a(argc, argv);
    //QCoreApplication::setApplicationName("YahooKDDCup2 Prediction");

    //Sleep(3000);

    printf("Start\n");
    QTime myTimer;
    myTimer.start();
    setbuf(stdout, NULL);

    QString postfix = "";
    if (QString::fromAscii(argv[2]) == "sample") postfix = "_sample";
    QString train_file = "../../train";
    train_file.append(postfix);
    QString valid_fn = "../../" + QString::fromAscii(argv[1]);
    valid_fn.append(postfix);
    QString negatives_train_file = "../../train_negatives";
    negatives_train_file.append(postfix);
    QString tracks_file = "../../_trackData.txt";

    RsHash train = load_set(train_file, TRAIN);
    RsHash valid = load_set(valid_fn, VALID);
    TaxHash tracks = load_tracks(tracks_file);

    if (QString::fromAscii(argv[3]) == "study")
    {
//        estimate(content_pred::predict(train, valid, tracks, 22,  true), valid_fn, true);

//	  check(train, "../i2i_weights");
//        itemnn_pred::study(train, true);
//        itemnn_pred::predict(train, valid, 1, true);
//        estimate(valid, valid_fn, true);

        binsvd_pred::study(train, valid, negatives_train_file, valid_fn, QList<float>(), true);
        estimate(binsvd_pred::predict(valid, true, true), valid_fn, true);
    }
    else if (QString::fromAscii(argv[3]) == "opt")
    {
//	optimize_gsect(train, valid, negatives_train_file, valid_fn, binsvd_pred::study);
	optimize_gsect(train, valid, tracks, negatives_train_file, valid_fn, content_pred::study);
    

//    optimize_gsect(train, valid, valid_fn, itemnn_pred::get_predictions);
//    optimize_bf(train, valid, valid_fn, itemnn_pred::get_predictions);
    }

    printf("Finished. Exec time: %3.2f sec\n", (float)myTimer.elapsed() / 1000);
    //return a.exec();
    exit(0);
}















