#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) , port(0)
{
    ui->setupUi(this);
    setUnifiedTitleAndToolBarOnMac(true);

    commandListModel = new XBeeCommandListModel(ui->commandList);
    ui->commandList->setModel(commandListModel);

    connect(ui->console, SIGNAL(keyPressed(QString &)), commandListModel, SLOT(sendMessage(QString&)));
    connect(commandListModel, SIGNAL(sentMessage(const QString&)), ui->console, SLOT(sentMessage(const QString&)));
    connect(commandListModel, SIGNAL(receivedMessage(const QString&)), ui->console, SLOT(receivedMessage(const QString&)));
    connect(commandListModel, SIGNAL(errorMessage(const QString &)), this, SLOT(model_error(const QString &)));
    connect(commandListModel, SIGNAL(statusMessage(const QString &)), ui->console, SLOT(statusMessage(const QString &)));
    connect(commandListModel, SIGNAL(destinationChanged()), this, SLOT(on_apiCheckBox_clicked()));
    connect(ui->readButton, SIGNAL(clicked()), commandListModel, SLOT(readParamters()));
    connect(ui->writeButton, SIGNAL(clicked()), commandListModel, SLOT(writeParameters()));
    connect(ui->apiCheckBox, SIGNAL(clicked(bool)), commandListModel, SLOT(apiMode(bool)));
    connect(ui->commandList, SIGNAL(doubleClicked(QModelIndex)), commandListModel, SLOT(sendATCommandRequest(QModelIndex)));
    connect(ui->remoteConfigCheckBox, SIGNAL(clicked(bool)), commandListModel, SLOT(setRemoteConfigure(bool)));

    ui->commandList->expandAll();
    ui->commandList->resizeColumnToContents(0);

    ui->networkAddress->setValidator(new HexValidator(16, this));
    ui->destinationAddress->setValidator(new HexValidator(64, this));

    setWindowIcon(QIcon(":/xbee.png"));
    showMaximized ();
}

MainWindow::~MainWindow()
{
    if(port)
    {
        delete port;
        port = 0;
    }
    delete ui;
}

void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::about(this,
                       tr("About XBee Configuration"),
                       tr("XBee Configuration Tool 0.1\n"
                          "(C) 2009 Y.Okamura\n"
                          "Released under GPL\n\n"
                          "Oxygen icons\n"
                          "(C) 2005-2007 Oxygen Icon project\n"
                          "Released under Creative Common Attribution-ShareAlike 3.0 License or LGPL")
                      );
}


void MainWindow::on_actionConnect_triggered()
{
    SerialPortSelector *sps = new SerialPortSelector(this);
    if(sps->exec() == QDialog::Accepted)
    {
        if(port && port->isConnected())
        {
            on_actionDisconnect_triggered();
        }

        port = sps->getPort();
        if(!port->connect())
        {
            ui->console->appendMessage(QString("Serial Port opening is failed.<br>%1 (%2bps)<br>")
                                       .arg(port->getPort().c_str(),
                                            QString::number(port->getBaudrate())),
                                       "green", true);
            delete port;
            port = 0;
        }
        else
        {
            ui->console->setSerialPort(port);
            commandListModel->setSerialPort(port);
            //qDebug("Serial Port is opened successfully.");
            ui->console->appendMessage(QString("Serial Port is opened successfully.<br>%1 (%2bps)<br>")
                                       .arg(port->getPort().c_str(),
                                            QString::number(port->getBaudrate())),
                                       "green", true);
            statusBar()->showMessage(QString("Connected to %1 (%2bps)")
                                     .arg(port->getPort().c_str())
                                     .arg(QString::number(port->getBaudrate())));
        }
    }
    delete sps;
}


void MainWindow::on_actionAbout_Qt_triggered()
{
    QApplication::aboutQt();
}

void MainWindow::on_actionDisconnect_triggered()
{
    if(port)
    {
        ui->console->appendMessage("<br>Serial Port is closed.<br>", "green", true);
        port->close();
        ui->console->setSerialPort(0);
        commandListModel->setSerialPort(0);
        statusBar()->showMessage(QString("Serial port is closed."));
        delete port;
        port = 0;
    }
}

void MainWindow::on_actionQuit_triggered()
{
    this->close();
}

void MainWindow::on_commandList_clicked(QModelIndex index)
{
    ui->comment->setText(commandListModel->data(index, Qt::ToolTipRole).toString());
}

void MainWindow::model_error(const QString& message)
{
    QString html = QString("<br>ERROR : %1<br>").arg(message);
    ui->console->appendMessage(html, "green", true);
}

void MainWindow::on_apiCheckBox_clicked()
{
    ui->destinationAddress->setText(QString::number(commandListModel->destinationAddress(), 16));
    ui->networkAddress->setText(QString::number(commandListModel->networkAddress(), 16));
}

void MainWindow::on_actionClear_Console_triggered()
{
    ui->console->setText("_");
}

void MainWindow::on_sendButton_clicked()
{
    SendDialog *sendDialog = new SendDialog(this);
    if(sendDialog->exec() == QDialog::Accepted)
    {
        commandListModel->sendTransmitRequest(sendDialog->senddata());
    }
    delete sendDialog;
}

void MainWindow::on_networkAddress_textEdited(QString newna)
{
    bool ok;
    unsigned short na = newna.toUShort(&ok, 16);
    if(ok)
    {
        commandListModel->setNetworkAddress(na);
    }
    else
    {
        QMessageBox mesbox;
        mesbox.setText("Cannot change value.");
        mesbox.setInformativeText("Wrong format.");
        mesbox.setIcon(QMessageBox::Warning);
        mesbox.exec();
    }
}

void MainWindow::on_destinationAddress_textEdited(QString newda)
{
    bool ok;
    qulonglong da = newda.toULongLong(&ok, 16);
    if(ok)
    {
        commandListModel->setDestinationAddress(da);
    }
    else
    {
        QMessageBox mesbox;
        mesbox.setText("Cannot change value.");
        mesbox.setInformativeText("Wrong format.");
        mesbox.setIcon(QMessageBox::Warning);
        mesbox.exec();
    }
}


void MainWindow::on_actionDigi_Homepage_triggered()
{
    QDesktopServices::openUrl(QUrl("http://www.digi.com/support/productdetl.jsp?pid=3524&osvid=0&s=378&tp=3"));
}

void MainWindow::on_actionFull_Screen_triggered()
{
    if(isFullScreen())
    {
        this->showMaximized();
        //setUnifiedTitleAndToolBarOnMac(true);
    }
    else
    {
        //setUnifiedTitleAndToolBarOnMac(false);
        this->showFullScreen();
    }
}
