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
    logFile->setFileName(logCat + "log.txt");
    if(!logFile->open(QIODevice::ReadWrite))
        qDebug() << "Could not open log file";
    QTextStream out(logFile);
    out << "Sync started at: " << QDateTime::currentDateTime().toString() + "\n";

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
    QTextStream out(logFile);

    timerSync->stop();

    QFileInfoList masterInfo = masterDir->entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries, QDir::Name);
    QFileInfoList slaveInfo = slaveDir->entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries, QDir::Name);

    for(int i = 0; i < masterInfo.size(); i++)
    {
        if(slaveDir->exists(masterInfo.value(i).fileName()))
        {
            tempFileInfo.setFile(slaveDir->path() + '/' +masterInfo.value(i).fileName());
            index = slaveInfo.indexOf(tempFileInfo);
            out << "Entry " << masterInfo.value(i).fileName() << " exists in a slave folder " << QDateTime::currentDateTime().toString() + "\n";
            if(masterInfo.value(i).lastModified() == slaveInfo.value(index).lastModified())
            {
                qDebug() << "Entry " + masterInfo.value(i).fileName() + " is equal";
                out << "Entry " + masterInfo.value(i).fileName() + " is equal\n";
                out.reset();
            }
            else
            {
                qDebug() << "Entry " + masterInfo.value(i).fileName() + " is an older version";
                out << "Entry " + masterInfo.value(i).fileName() + " is an older version\n";
                if(slaveInfo.value(index).isDir())
                {
                    tempDir.cd(slaveInfo.value(index).absoluteFilePath());
                    if(tempDir.removeRecursively())
                    {
                        out << "Entry" + slaveInfo.value(index).fileName() + "removed\n";
                        qDebug() << "Entry" + slaveInfo.value(index).fileName() + "removed";
                        slaveDir->mkdir(masterInfo.value(i).fileName());
                        tempDirName = slaveDir->absolutePath() + masterInfo.value(i).fileName();
                        tempDir.cd(tempDirName);
                        copyPath(masterInfo.value(i).absoluteFilePath(), tempDir.absolutePath());
                        qDebug() << "File " + masterInfo.value(i).fileName() + " copied";
                        out << "File " + masterInfo.value(i).fileName() + " copied\n";
                    }
                    else
                    {
                        qDebug() << "Failed to remove entry " + slaveInfo.value(index).fileName();
                        out << "Failed to remove entry " + slaveInfo.value(index).fileName() + "\n";
                    }
                }
                else if(slaveInfo.value(index).isFile())
                {
                    if(slaveDir->remove(slaveInfo.value(index).fileName()))
                    {
                        qDebug() << "Entry removed " + slaveInfo.value(index).fileName();
                        out << "Entry removed " + slaveInfo.value(index).fileName();
                        if(QFile::copy(masterInfo.value(i).absoluteFilePath(), slaveDir->absolutePath() + '/' + masterInfo.value(i).fileName()))
                        {
                            qDebug() << "File " + masterInfo.value(i).fileName() + " copied";
                            out << "File " + masterInfo.value(i).fileName() + " copied\n";
                        }
                        else
                        {
                            qDebug() << "File " + masterInfo.value(i).fileName() + " copied";
                            out << "File " + masterInfo.value(i).fileName() + " copied\n";
                        }
                    }
                    else
                    {
                        qDebug() << "Failed to remove entry " + slaveInfo.value(index).fileName();
                        out << "Failed to remove entry " + slaveInfo.value(index).fileName() + "\n";
                    }
                }
                else
                {
                    qDebug() << "Unrecognized file";
                    out << "Unrecognized file\n";
                }
            }
        }
        else
        {
            qDebug() << "Entry " + masterInfo.value(i).fileName() + " does not exist";
            out << "Entry " + masterInfo.value(i).fileName() + " does not exist\n";

            if(masterInfo.value(i).isDir())
            {
                slaveDir->mkdir(masterInfo.value(i).fileName());
                tempDirName = slaveDir->absolutePath() + masterInfo.value(i).fileName();
                tempDir.cd(tempDirName);
                copyPath(masterInfo.value(i).absoluteFilePath(), tempDir.absolutePath());
                qDebug() << "Entry " + masterInfo.value(i).fileName() + " copied";
                out << "Entry " + masterInfo.value(i).fileName() + " copied\n";
            }
            else if(masterInfo.value(i).isFile())
            {
                if(QFile::copy(masterInfo.value(i).absoluteFilePath(), slaveDir->absolutePath() + '/' + masterInfo.value(i).fileName()))
                {
                    qDebug() << "File " + masterInfo.value(i).fileName() + " copied";
                    out << "File " + masterInfo.value(i).fileName() + " copied\n";
                }
                else
                {
                    qDebug() << "Failed to copy file " + masterInfo.value(i).fileName();
                    out << "Failed to copy file " + masterInfo.value(i).fileName() + "\n";
                }
            }
            else
            {
                qDebug() << "Unrecognized file";
                out << "Unrecognized file\n";
            }
        }
    }

    masterInfo = masterDir->entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries, QDir::Name);
    slaveInfo = slaveDir->entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries, QDir::Name);

    for(int i = 0; i < slaveInfo.size(); i++)
    {
        qDebug() << "Searching for junk..." << slaveInfo.value(i).fileName();
        out << "Searching for junk..." << slaveInfo.value(i).fileName() + "\n";
        if(!masterDir->exists(slaveInfo.value(i).fileName()))
        {
            qDebug() << "Junk found " + slaveInfo.value(i).fileName() + " proceed for removal";
            out << "Junk found " + slaveInfo.value(i).fileName() + " proceed for removal\n";
            if(slaveInfo.value(i).isDir())
            {
                tempDir.cd(slaveInfo.value(i).absoluteFilePath());
                if(tempDir.removeRecursively())
                {
                    qDebug() << "Entry removed";
                    out << "Entry removed\n";
                }
                else
                {
                    qDebug() << "Failed to remove entry";
                    out << "Failed to remove entry\n";
                }
            }
            else if(slaveInfo.value(i).isFile())
            {
                if(slaveDir->remove(slaveInfo.value(i).fileName()))
                {
                    qDebug() << "Entry removed";
                    out << "Entry removed\n";
                }
                else
                {
                    qDebug() << "Failed to remove entry";
                    out << "Failed to remove entry\n";
                }
            }
            else
            {
                qDebug() << "Unrecognized file";
                out << "Unrecognized file\n";
            }
        }
    }

    timerSync->start(syncTime);
}

QByteArray replicant::getEntryHash(QFileInfo *fileInfo)
{
    QByteArray  md5;
    QString     tempPath = fileInfo->absoluteFilePath();
    QFile       tempFile;
    tempFile.setFileName(tempPath);

    md5 = QCryptographicHash::hash(tempFile.readAll(), QCryptographicHash::Md5);

    return md5;
}
