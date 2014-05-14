#ifndef SENDDIALOG_H
#define SENDDIALOG_H

#include <QDialog>

namespace Ui {
    class SendDialog;
}

class SendDialog : public QDialog {
    Q_OBJECT
public:
    SendDialog(QWidget *parent = 0);
    ~SendDialog();
    QString senddata();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::SendDialog *m_ui;
};

#endif // SENDDIALOG_H
