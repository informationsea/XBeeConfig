#include "serialportselector.h"
#include "ui_serialportselector.h"

SerialPortSelector::SerialPortSelector(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::SerialPortSelector),
    selectedport("")
{
    m_ui->setupUi(this);

    m_ui->portListWidget->clear();

    list = SerialPort::getPortList();
    std::vector<std::string>::iterator it;
    for(it = list.begin(); it != list.end(); it++)
    {
        m_ui->portListWidget->addItem(tr(it->c_str()));
    }
}

SerialPortSelector::~SerialPortSelector()
{
    delete m_ui;
}

void SerialPortSelector::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type())
    {
    case QEvent::LanguageChange:
        m_ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

SerialPort::SerialPort *SerialPortSelector::getPort()
{
    return SerialPort::getPort(selectedport, baudrate);
}

void SerialPortSelector::accept ()
{
    //QModelIndex items = m_ui->portListWidget->selectedIndexes();
    //printf("%d\n",items.row());

    QModelIndexList items = m_ui->portListWidget->selectionModel()->selectedIndexes();
    if(items.size() == 1)
    {
        QString text = m_ui->baudrateCombo->currentText();
        qDebug("Selected:%s %d", list[items.first().row()].c_str(), text.toInt());

        selectedport = list[items.first().row()];
        baudrate = text.toInt();

        QDialog::accept();
    }

    /*
        if(m_ui->portListWidget->selectedItems().size() == 1){
            qDebug("Port is selected.");
            qDebug("Port:%s",m_ui->portListWidget->selectedItems().takeFirst()->text());

        }
        */

    //m_ui->baudrateCombo->it
}
