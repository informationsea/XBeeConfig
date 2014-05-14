#ifndef XBEECOMMANDLISTMODEL_H
#define XBEECOMMANDLISTMODEL_H

#include <stdint.h>
#include <QAbstractItemModel>
#include <QModelIndex>
#include <QFile>
#include <QStringList>
#include <QTimer>
#include <QMessageBox>
#include <QByteArray>

#include "serialport.h"
#include "xbeecommand.h"
#include "xbeecommandhelper.h"

enum XBeeCommandMode {XBEE_NONE,XBEE_READ,XBEE_WRITE};
enum XBeeAPIStatus {XBA_NONE,XBA_HEAD,XBA_RECEIVE,XBA_APIRES,XBA_REMOTE_APIRES,XBA_TRANSMIT_STATUS,XBA_MODEM_STATUS,XBA_UNKOWN};

struct ReceivedPacket {
    int networkAddress;
    qlonglong targetAddress;
    bool broadcast;
    QByteArray receiveBuffer;
} ;

struct XBeeAPIResponse {
    int frameId;
    char atcommand[3];
    int status;
    QByteArray receiveBuffer;
    int length;

    qulonglong serialNumber; // used in remote api response
    unsigned short networkAddress; // used in remote api response
};

struct XBeeTransmitStatus {
    int frameId;
    unsigned int networkAddress;
    int retryCount;
    int delivaryStatus;
    int discoveryStatus;
};

struct XBeeModemStatus {
    int status;
};

class XBeeCommandListModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    XBeeCommandListModel(QObject *parent = 0);
    virtual int columnCount ( const QModelIndex & parent = QModelIndex() ) const;
    virtual QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
    virtual QModelIndex index ( int row, int column, const QModelIndex & parent = QModelIndex() ) const;
    virtual QModelIndex parent ( const QModelIndex & index ) const;
    virtual int rowCount ( const QModelIndex & parent = QModelIndex() ) const;
    virtual QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
    virtual Qt::ItemFlags flags ( const QModelIndex & index ) const;
    virtual bool hasChildren ( const QModelIndex & parent = QModelIndex()  ) const;
    virtual bool setData ( const QModelIndex & index, const QVariant & value, int role = Qt::EditRole );

    void setSerialPort(SerialPort::SerialPort *port);
    void setNetworkAddress(unsigned int newna);
    unsigned int networkAddress();
    void setDestinationAddress(qulonglong newda);
    qulonglong destinationAddress();

    //xbee
    void sendTransmitRequest(QString mess);
    XBeeCommand& searchCommand(QString atcommand);
    void sendATCommand(XBeeCommand& command,bool write);

public slots:
    void sendMessage(QString &message);
    void sendMessage(const char *message,int length = -1);
    void readParamters();
    void writeParameters();
    void apiMode(bool enable);
    void setRemoteConfigure(bool enable);
    void sendATCommandRequest(QModelIndex index);

signals:
    void sentMessage(const QString &message);
    void receivedMessage(const QString &message);
    void errorMessage(const QString &message);
    void statusMessage(const QString &html);
    void dataChanged ( const QModelIndex & topLeft, const QModelIndex & bottomRight );

    void destinationChanged();

private slots:
    void checkPort();
    void commandTimeout();

private:
    QList<QString> categoryNames;
    QList<QModelIndex> categories;
    QList< QList<XBeeCommand> > commandList;
    QTimer *timer;
    QTimer *timeoutTimer;
    SerialPort::SerialPort *port;

    XBeeCommandMode mode;
    int currentCategory;
    int currentCommand;
    QString lastLine;
    qulonglong _destinationAddress;
    unsigned int _networkAddress;


    bool apiEnable;
    bool remoteConfigureEnable;
    int apiReceivedLength;
    int apiLength;
    XBeeAPIStatus xbeeAPIStatus;

    uint8_t checksum;
    ReceivedPacket receviedPacket;
    XBeeAPIResponse apiResponse;
    XBeeTransmitStatus transmitStatus;
    XBeeModemStatus modemStatus;

    bool isAllowedCommand(int category,int num,bool write);
    void processLine(QString line);
    void checkChecksum();
    void processAPI(unsigned char *buf, int length);

    bool sendNextATCommand();
};

#endif // XBEECOMMANDLISTMODEL_H
