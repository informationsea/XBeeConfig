#include "textconsole.h"

TextConsole::TextConsole( QWidget * parent ): QTextEdit(parent), port(0)
{
    init();
}
TextConsole::TextConsole( const QString & text, QWidget * parent ): QTextEdit(text, parent), port(0)
{
    init();
}

TextConsole::~TextConsole()
{
    delete timer;
}

void TextConsole::init()
{
    timer = new QTimer(this);
    timer->setInterval(60);
    timer->setSingleShot(false);
    connect(timer, SIGNAL(timeout()), this, SLOT(checkPort()));

    this->setPlainText("_");
}

void TextConsole::setSerialPort(SerialPort::SerialPort *_port)
{
    port = _port;
    if(port)
    {
        //timer->start();
    }
    else
    {
        //timer->stop();
    }
}

void TextConsole::checkPort()
{
    //qDebug("TIMER");
}

void TextConsole::keyPressEvent ( QKeyEvent * event )
{
    checkPort();
    QString sendtext;
    //qDebug("%02x",event->key());
    switch(event->key())
    {
    case Qt::Key_Enter:
        sendtext = "\r";
        break;
    default:
        sendtext = event->text();
    }

#if 0
    if(port)
    {
        const char *buf = sendtext.toUtf8().data();
        port->send(buf, strlen(buf));
        appendMessage(event->text(), "blue");
        event->accept();
    }
#endif

    emit keyPressed(sendtext);
}

void TextConsole::appendMessage(const QString &mess, const QString &color, bool htmlEnable)
{
    if(mess.size())
    {
        QString html;
        if(!htmlEnable)
        {
            if(color.isNull())
            {
                html = QString("<pre>%2</pre>").arg(mess);
            }
            else
            {
                html = QString("<font color=\"%1\"><pre>%2</pre></font>").arg(color).arg(mess);
            }
            html.replace("\n", "");
            html.replace("\r", "<br>");
        }
        else
        {
            if(!color.isNull())
            {
                html = QString("<font color=\"%1\">%2</font>").arg(color).arg(mess);
            }
            else
            {
                html = mess;
            }
        }
        moveCursor(QTextCursor::End);
        moveCursor(QTextCursor::Left);
        insertHtml(html);
        ensureCursorVisible ();
    }
}

void TextConsole::sentMessage(const QString &mess)
{
    appendMessage(mess, "blue");
}

void TextConsole::receivedMessage(const QString &mess)
{
    //qDebug("Received");
    appendMessage(mess, "red");
}

void TextConsole::statusMessage(const QString &html)
{
    appendMessage(html, QString(), true);
}

