#include "hexvalidator.h"

HexValidator::HexValidator(int _bitwidth, QObject *parent)
    : QValidator(parent)
{
    bitwidth = _bitwidth;
}

QValidator::State HexValidator::validate ( QString & input, int & pos ) const
{
    if(input.size() == 0)
    {
        return QValidator::Intermediate;
    }

    bool ok;
    qulonglong value = input.toULongLong(&ok, 16);
    if(ok)
    {
        if(bitwidth == 64)
        {
            return QValidator::Acceptable;
        }
        qulonglong max = 1 << bitwidth;
        if(value < max)
        {
            return QValidator::Acceptable;
        }
    }
    return QValidator::Invalid;
}
