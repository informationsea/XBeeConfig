#include "senddialog.h"
#include "ui_senddialog.h"

SendDialog::SendDialog(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::SendDialog)
{
    m_ui->setupUi(this);
}

SendDialog::~SendDialog()
{
    delete m_ui;
}

void SendDialog::changeEvent(QEvent *e)
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

QString SendDialog::senddata()
{
    return m_ui->sendtext->document()->toPlainText();
}
