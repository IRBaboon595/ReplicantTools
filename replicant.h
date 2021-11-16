#ifndef REPLICANT_H
#define REPLICANT_H

#include <QObject>
#include <QDir>
#include <QFile>
#include <QDateTime>
#include <QString>
#include <QFileInfo>
#include <QTimer>
#include <QCryptographicHash>
#include <QByteArray>
#include <FolderCompressor.h>

class replicant : public QObject
{
    Q_OBJECT
public:
    explicit replicant(QObject *parent = nullptr);

    void        getArgs(QStringList list);
    void        copyPath(QString src, QString dst);
    QByteArray  getEntryHash(QFileInfo *fileInfo);
    bool        compare(QFileInfo mInfo, QFileInfo sInfo);

public slots:
    void sync();

private:
    QDir    *masterDir;
    QDir    *slaveDir;
    QFile   *logFile;

    QString masterCat;
    QString slaveCat;
    QString logCat;

    int     syncTime;
    QTimer  *timerSync;
};

#endif // REPLICANT_H
