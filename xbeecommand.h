#ifndef XBEECOMMAND_H
#define XBEECOMMAND_H

#include <QObject>
#include <QAbstractItemModel>
#include <QModelIndex>
#include <QStringList>
#include <QByteArray>

#define XBC_NO_PARAMETER 0x00
#define XBC_NUMBER       0x01
#define XBC_STRING       0x02
#define XBC_WRITABLE     0x10
#define XBC_READABLE     0x20

class XBeeCommand {
public:
    XBeeCommand(const QString &command,
                const QString &description,
                const QModelIndex &commandIndex,
                const QModelIndex &valueIndex,
                uint8_t type = XBC_NO_PARAMETER,
                unsigned long maxValue = 0,
                unsigned long minValue = 0);
    XBeeCommand(){}

    const QString& command() const;
    const QString& description() const;
    uint8_t type() const;
    bool isReadable() const;
    bool isWritable() const;
    bool isModified() const;
    unsigned long maxValue() const;
    unsigned long minValue() const;

    QModelIndex commandIndex() const;
    QModelIndex valueIndex() const;

    QString value() const;
    QByteArray data() const;
    bool setValue(QString value,bool check = false);
    void setModified(bool modified);

private:
    QString _command;
    QString _description;
    uint8_t _type;
    unsigned long _maxValue;
    unsigned long _minValue;

    QModelIndex _commandIndex;
    QModelIndex _valueIndex;

    bool _modified;
    QString _value;
};

#endif // XBEECOMMAND_H
