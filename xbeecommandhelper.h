#ifndef XBEECOMMANDHELPER_H
#define XBEECOMMANDHELPER_H

#include <QByteArray>
#include <QString>

QString byteArrayToString(const char * data,int length);
QString byteArrayToString(const QByteArray& data);
QString processAPI_NodeDiscover(const QByteArray& data);

#endif // XBEECOMMANDHELPER_H
