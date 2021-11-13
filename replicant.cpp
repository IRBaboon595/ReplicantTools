#include "replicant.h"

replicant::replicant(QObject *parent) : QObject(parent)
{
    timerSync = new QTimer;
    masterDir = new QDir;
    slaveDir = new QDir;
    logFile = new QFile;

    masterCat.clear();
    slaveCat.clear();
    logCat.clear();

    syncTime = 5000;

    timerSync->start(syncTime);
    connect(timerSync, SIGNAL(timeout()), this, SLOT(sync()));
}

void replicant::getArgs(QStringList list)
{
    masterCat = list.value(1);
    slaveCat = list.value(2);
    logCat = list.value(3);
    syncTime = list.value(4).toInt();

    qDebug() << masterCat << slaveCat << logCat << syncTime;
}

void copyPath(QString src, QString dst)
{
    QDir dir(src);
    if (! dir.exists())
        return;

    foreach (QString d, dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        QString dst_path = dst + QDir::separator() + d;
        dir.mkpath(dst_path);
        copyPath(src+ QDir::separator() + d, dst_path);
    }

    foreach (QString f, dir.entryList(QDir::Files)) {
        QFile::copy(src + QDir::separator() + f, dst + QDir::separator() + f);
    }
}

void replicant::sync()
{

}
