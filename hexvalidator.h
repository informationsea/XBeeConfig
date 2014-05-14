#ifndef HEXVALIDATOR_H
#define HEXVALIDATOR_H

#include <QValidator>

class HexValidator : public QValidator
{
public:
    HexValidator(int bitwidth, QObject *parent );

    virtual QValidator::State validate ( QString & input, int & pos ) const;

private:
    int bitwidth;
};

#endif // HEXVALIDATOR_H
