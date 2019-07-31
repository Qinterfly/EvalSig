#ifndef FILEOPERATE_H
#define FILEOPERATE_H

#include <QString>

// Проверка файла на существование и на режим записи / чтения
int checkFile(QString const& fileFullPath, QString const& mode);

#endif // FILEOPERATE_H
