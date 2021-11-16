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

    syncTime = 3000;

    timerSync->start(syncTime);
    connect(timerSync, SIGNAL(timeout()), this, SLOT(sync()));
}

void replicant::getArgs(QStringList list)
{
    masterCat = list.value(1);
    slaveCat = list.value(2);
    logCat = list.value(3);
    syncTime = list.value(4).toInt();

    masterDir->setPath(masterCat);
    slaveDir->setPath(slaveCat);
    logFile->setFileName(logCat + "log");

    qDebug() << masterCat << slaveCat << logCat << syncTime;
}

void replicant::copyPath(QString src, QString dst)
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
    QString     tempDirName;
    QString     tempFileName;
    QDir        tempDir;
    QFileInfo   tempFileInfo;
    uint16_t    index = 0;
    //uint16_t*   indexRemTemp;
    //uint16_t*   indexAddTemp;

    timerSync->stop();

    QFileInfoList masterInfo = masterDir->entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries, QDir::Name);
    QFileInfoList slaveInfo = slaveDir->entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries, QDir::Name);

    //indexRemTemp = new uint16_t(slaveInfo.size());
    //indexAddTemp = new uint16_t(masterInfo.size());

    for(int i = 0; i < masterInfo.size(); i++)
    {
        if(slaveDir->exists(masterInfo.value(i).fileName()))
        {
            tempFileInfo.setFile(slaveDir->path() + '/' +masterInfo.value(i).fileName());
            index = slaveInfo.indexOf(tempFileInfo);
            qDebug() << masterInfo.value(i).fileName() << slaveInfo.value(index).fileName();
            if(masterInfo.value(i).lastModified() == slaveInfo.value(index).lastModified())
            {
                qDebug() << "Entry " + masterInfo.value(i).fileName() + " is equal";
            }
            else
            {
                qDebug() << "Entry " + masterInfo.value(i).fileName() + " is an older version";
                if(slaveInfo.value(index).isDir())
                {
                    tempDir.cd(slaveInfo.value(index).absoluteFilePath());
                    if(tempDir.removeRecursively())
                    {
                        qDebug() << "Entry removed";
                        slaveDir->mkdir(masterInfo.value(i).fileName());
                        tempDirName = slaveDir->absolutePath() + masterInfo.value(i).fileName();
                        tempDir.cd(tempDirName);
                        copyPath(masterInfo.value(i).absoluteFilePath(), tempDir.absolutePath());
                    }
                    else
                    {
                        qDebug() << "Failed to remove entry";
                    }
                }
                else if(slaveInfo.value(i).isFile())
                {
                    if(slaveDir->remove(slaveInfo.value(i).fileName()))
                    {
                        qDebug() << "Entry removed";
                        if(QFile::copy(masterInfo.value(i).absoluteFilePath(), slaveDir->absolutePath() + '/' + masterInfo.value(i).fileName()))
                        {
                            qDebug() << "File " + masterInfo.value(i).fileName() + " copied";
                        }
                        else
                        {
                            qDebug() << "Failed to copy file " + masterInfo.value(i).fileName();
                        }
                    }
                    else
                    {
                        qDebug() << "Failed to remove entry";
                    }
                }
                else
                {
                    qDebug() << "Unrecognized file";
                }
            }
        }
        else
        {
            qDebug() << "Entry does not exist";
            if(slaveInfo.value(index).isDir())
            {
                tempDir.cd(slaveInfo.value(index).absoluteFilePath());
                slaveDir->mkdir(masterInfo.value(i).fileName());
                tempDirName = slaveDir->absolutePath() + masterInfo.value(i).fileName();
                tempDir.cd(tempDirName);
                copyPath(masterInfo.value(i).absoluteFilePath(), tempDir.absolutePath());
            }
            else if(slaveInfo.value(i).isFile())
            {
                if(QFile::copy(masterInfo.value(i).absoluteFilePath(), slaveDir->absolutePath() + '/' + masterInfo.value(i).fileName()))
                {
                    qDebug() << "File " + masterInfo.value(i).fileName() + " copied";
                }
                else
                {
                    qDebug() << "Failed to copy file " + masterInfo.value(i).fileName();
                }
            }
            else
            {
                qDebug() << "Unrecognized file";
            }
        }
    }

    for(int i = 0; i < slaveInfo.size(); i++)
    {
        if(!masterDir->exists(slaveInfo.value(i).fileName()))
        {
            if(slaveInfo.value(index).isDir())
            {
                tempDir.cd(slaveInfo.value(index).absoluteFilePath());
                if(tempDir.removeRecursively())
                {
                    qDebug() << "Entry removed";
                }
                else
                {
                    qDebug() << "Failed to remove entry";
                }
            }
            else if(slaveInfo.value(i).isFile())
            {
                if(slaveDir->remove(slaveInfo.value(i).fileName()))
                {
                    qDebug() << "Entry removed";
                }
                else
                {
                    qDebug() << "Failed to remove entry";
                }
            }
            else
            {
                qDebug() << "Unrecognized file";
            }
        }
    }

    timerSync->start(syncTime);
}
