#include <QCoreApplication>
#include "replicant.h"
#include <stdlib.h>
#include <stdint.h>
#include <iostream>

using namespace std;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    replicant r;

    r.getArgs(a.arguments());

    return a.exec();
}
