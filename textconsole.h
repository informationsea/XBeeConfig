#ifndef TEXTCONSOLE_H
#define TEXTCONSOLE_H

#include <QTextEdit>
#include <QKeyEvent>
#include <QTimer>
#include <QApplication>
 #include <QClipboard>

#include "serialport.h"

class TextConsole : public QTextEdit
{
    Q_OBJECT
public:
    TextConsole( QWidget * parent = 0 );
    TextConsole( const QString & text, QWidget * parent = 0 );
    virtual ~TextConsole();

    void setSerialPort(SerialPort::SerialPort *port);
    void appendMessage(const QString &mess,const QString &color,bool html = false);

public slots:
    void checkPort();
    void sentMessage(const QString &mess);
    void receivedMessage(const QString &mess);
    void statusMessage(const QString &html);

signals:
    void keyPressed(QString &messageToSend);

protected:
    virtual void keyPressEvent ( QKeyEvent * event );

private:
    QTimer *timer;
    SerialPort::SerialPort *port;
    void init();
};

#endif // TEXTCONSOLE_H
