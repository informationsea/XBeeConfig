#ifndef SERIALPORTSELECTOR_H
#define SERIALPORTSELECTOR_H

#include <QDialog>
#include <QListWidget>
#include <QListWidgetItem>

#include <serialport.h>

namespace Ui {
    class SerialPortSelector;
}

class SerialPortSelector : public QDialog {
    Q_OBJECT
public:
    SerialPortSelector(QWidget *parent = 0);
    ~SerialPortSelector();

    SerialPort::SerialPort *getPort();

public slots:
    virtual void accept ();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::SerialPortSelector *m_ui;

    std::string selectedport;
    long baudrate;
    std::vector<std::string> list;
};

#endif // SERIALPORTSELECTOR_H
