#include "core/FileOperate.h"

// ---- Вспомогательные функции для работы с файлами -----------------------------------------------------------

// Проверка файла на существование и на режим записи / чтения
bool checkFile(QString const& fileFullPath, QString const& mode) {
    // mode == Read -- чтение
    // mode == Write -- запись

    // Режим чтения
    if ( mode == "read" ){
        // Проверка существования файла
        if ( !QFileInfo::exists(fileFullPath) || !QFileInfo(fileFullPath).isFile() ){
            qDebug() << "Файл:" << fileFullPath << "не найден";
            return 0;
        }
        // Проверка на возможность чтения
        if ( !QFileInfo(fileFullPath).isReadable() ){
            qDebug() << "Файл:" << fileFullPath << "не доступен для чтения";
            return 0;
        }
        // Проверка на пустоту
        if ( QFileInfo(fileFullPath).size() == 0 ){ // Проверка на пустоту файла
            qDebug() << "Файл:" << fileFullPath << "не содержит данных";
            return 0;
        }
    }
    // Режим записи
    if ( mode == "write" )
        if ( QFileInfo::exists(fileFullPath) && !QFileInfo(fileFullPath).isWritable() ){
            qDebug() << "Файл:" << fileFullPath << "не доступен для записи";
            return 0;
        }
    return 1;
}

// -------------------------------------------------------------------------------------------------------------
