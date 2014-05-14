#include "xbeecommand.h"

XBeeCommand::XBeeCommand(
    const QString &command,
    const QString &description,
    const QModelIndex &commandIndex,
    const QModelIndex &valueIndex,
    uint8_t type,
    unsigned long maxValue,
    unsigned long minValue)
{
    _command = command;
    _description = description;
    _type = type;
    _commandIndex = commandIndex;
    _valueIndex = valueIndex;
    _maxValue = maxValue;
    _minValue = minValue;
    _modified = false;

    //qDebug("%s %02x",_command.toUtf8().data(),type);
}

const QString& XBeeCommand::command() const
{
    return _command;
}

const QString& XBeeCommand::description() const
{
    return _description;
}

uint8_t XBeeCommand::type() const
{
    return _type;
}

unsigned long XBeeCommand::maxValue() const
{
    return _maxValue;
}

unsigned long XBeeCommand::minValue() const
{
    return _minValue;
}
QModelIndex XBeeCommand::commandIndex() const
{
    return _commandIndex;
}

QModelIndex XBeeCommand::valueIndex() const
{
    return _valueIndex;
}


QByteArray XBeeCommand::data() const
{
    if(_type & XBC_STRING)
    {
        return _value.toUtf8();
    }
    else if(_type & XBC_NUMBER)
    {
        QByteArray data;
        bool ok;
        unsigned long d = _value.toULong(&ok, 16);
        if(ok)
        {
            //bool havedata = false;
            for(int pos = sizeof(d) - 1; pos >= 0; pos--)
            {
                data.append((char)(d >> pos * 8));
            }
        }
        return data;
    }
    else
    {
        return QByteArray();
    }
}

QString XBeeCommand::value() const
{
    return _value;
}

bool XBeeCommand::setValue(QString value, bool check)
{
    if(_type == XBC_NO_PARAMETER)
    {
        return false;
    }
    if(check)
    {
        if(_type & XBC_STRING)
        {
            if(value.length() > 20)
            {
                return false;
            }
        }
        else if(_type & XBC_NUMBER)
        {
            bool ok;
            unsigned long v = value.toULong(&ok, 16);
            if(!ok)
            {
                return false;
            }
            if(v < _minValue || _maxValue < v)
            {
                return false;
            }
        }
    }
    _value = value;
    return true;
}

bool XBeeCommand::isReadable() const
{
    if(_type & XBC_READABLE)
    {
        //qDebug("Ready Readable %s %x",_command.toUtf8().data(),_type & XBC_READABLE);
        return true;
    }
    //qDebug("Not  Readable %s",_command.toUtf8().data());
    return false;
}

bool XBeeCommand::isWritable() const
{
    if(_type & XBC_WRITABLE)
        return true;
    return false;
}

bool XBeeCommand::isModified() const
{
    return _modified;
}

void XBeeCommand::setModified(bool modified)
{
    _modified = modified;
}
