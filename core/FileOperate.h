#ifndef FILEOPERATE_H
#define FILEOPERATE_H

#include <QString>
#include <QDebug>
#include <QFileInfo>

// Проверка файла на существование и на режим записи / чтения
bool checkFile(QString const& fileFullPath, QString const& mode);

#endif // FILEOPERATE_H
