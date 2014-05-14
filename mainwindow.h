#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QModelIndex>
#include <QDesktopServices>
#include <QUrl>

#include "hexvalidator.h"

#include "serialportselector.h"
#include "xbeecommandlistmodel.h"
#include "senddialog.h"

namespace Ui
{
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    XBeeCommandListModel *commandListModel;
    SerialPort::SerialPort *port;

private slots:
    void on_actionFull_Screen_triggered();
    void on_actionDigi_Homepage_triggered();
    void on_destinationAddress_textEdited(QString );
    void on_networkAddress_textEdited(QString );
    void on_sendButton_clicked();
    void on_actionClear_Console_triggered();
    void on_apiCheckBox_clicked();
    void on_commandList_clicked(QModelIndex index);
    void on_actionQuit_triggered();
    void on_actionDisconnect_triggered();
    void on_actionAbout_Qt_triggered();
    void on_actionAbout_triggered();
    void on_actionConnect_triggered();

    void model_error(const QString& message);
};

#endif // MAINWINDOW_H
