#include "xbeecommandlistmodel.h"

#include <QColor>
#include <QDebug>
#include <QModelIndex>

XBeeCommandListModel::XBeeCommandListModel(QObject *parent):
    QAbstractItemModel(parent), port(0), mode(XBEE_NONE)
    , _destinationAddress(0xffff), _networkAddress(0x7fff)
    , apiEnable(false), remoteConfigureEnable(false), xbeeAPIStatus(XBA_NONE)
{
    QFile listcsv(":/command/xbeecommand.csv");
    if(listcsv.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        //qDebug("OK");
        QByteArray linearray;
        QList<XBeeCommand> categoryCommandList;
        listcsv.readLine();
        while((linearray = listcsv.readLine()) != QByteArray())
        {
            QString line(linearray);
            QStringList elements = line.split(",");

            if(categoryNames.size() != 0 && elements[0] != categoryNames.last())
            {
                commandList.push_back(categoryCommandList);
                categoryCommandList.clear();
            }
            if(categoryNames.size() == 0 || elements[0] != categoryNames.last())
            {
                categoryNames.push_back(elements[0]);
            }

            uint8_t type = 0;
            if(elements[3].contains("Read only"))
                type |= XBC_READABLE;
            else if(elements[3].contains("Write only"))
                type |= XBC_WRITABLE;
            else if(!elements[3].contains("No parameter"))
                type |= XBC_WRITABLE | XBC_READABLE;


            unsigned long minValue = 0;
            unsigned long maxValue = 0;
            if(elements[3].contains("Number"))
            {
                type |= XBC_NUMBER;
                if(type & XBC_WRITABLE)
                {
                    minValue = elements[4].toULong(NULL, 16);
                    maxValue = elements[5].toULong(NULL, 16);
                }
            }
            else if(elements[3].contains("String"))
            {
                type |= XBC_STRING;
            }
            else
            {
                type = XBC_NO_PARAMETER;
            }

            categoryCommandList.push_back(XBeeCommand(elements[1], elements[2],
                                          createIndex(categoryCommandList.size(), 0, categoryNames.size() - 1),
                                          createIndex(categoryCommandList.size(), 1, categoryNames.size() - 1),
                                          type, maxValue, minValue));

        }
        commandList.push_back(categoryCommandList);
        listcsv.close();
        //qDebug("Closed");
    }
    else
    {
        qDebug("Failed");
    }

    for(int i = 0; i < categoryNames.size(); i++)
    {
#define CATEGORY_INTERNAL_ID 100
        categories.push_back(createIndex(i, 0, CATEGORY_INTERNAL_ID));
    }

    timer = new QTimer(this);
    timer->setInterval(10);
    timer->setSingleShot(false);
    connect(timer, SIGNAL(timeout()), this, SLOT(checkPort()));

    timeoutTimer = new QTimer(this);
    timeoutTimer->setInterval(1000 * 3);
    timeoutTimer->setSingleShot(true);
    connect(timeoutTimer, SIGNAL(timeout()), this, SLOT(commandTimeout()));
}

// ------------------ ListModel related -------------------------
int XBeeCommandListModel::columnCount ( const QModelIndex & /*parent*/ ) const
{
    return 2;
}

Qt::ItemFlags XBeeCommandListModel::flags ( const QModelIndex & index ) const
{
    qDebug() << index.internalId() << index.column() << index.row();
    if(index.internalId() == CATEGORY_INTERNAL_ID)
    {
        return Qt::ItemIsEnabled;
    }
    else if(index.column() == 0)
    {
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    }
    else if(index.column() == 1)
    {
        if(commandList[index.internalId()][index.row()].isWritable())
        {
            return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
        }
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    }
    return 0;
}

bool XBeeCommandListModel::setData ( const QModelIndex & index, const QVariant & value, int role )
{
    if(!index.isValid())
        return false;

    switch(role)
    {
    case Qt::EditRole:
        if(commandList[index.internalId()][index.row()].setValue(value.toString(), true))
        {
            emit dataChanged(commandList[index.internalId()][index.row()].valueIndex(),
                             commandList[index.internalId()][index.row()].valueIndex());

            commandList[index.internalId()][index.row()].setModified(true);
            return true;
        }
        else
        {
            QMessageBox mesbox;
            mesbox.setText("Cannot change value.");
            mesbox.setInformativeText("Out of range or illegal format.");
            mesbox.setIcon(QMessageBox::Warning);
            mesbox.exec();
        }


        XBeeCommand cmd = commandList[index.internalId()][index.row()];

        qDebug("Change : %s => %s", cmd.command().toUtf8().data(), value.toString().toUtf8().data());
        break;
    }
    return false;
}

QVariant XBeeCommandListModel::data ( const QModelIndex & index, int role ) const
{
    if(!index.isValid())
        return QVariant();

    switch(role)
    {
    case Qt::DisplayRole:
    case Qt::EditRole:
        if(index.parent() == QModelIndex())
        {
            return QVariant(categoryNames[index.row()]);
        }
        else if(index.parent().internalId() == CATEGORY_INTERNAL_ID)
        {
            if(index.column() == 0)
            {
                QString commandDescription =
                    QString("%1 - %2").arg(commandList[index.internalId()][index.row()].command(),
                                           commandList[index.internalId()][index.row()].description());
                return QVariant(commandDescription);
            }
            else if(index.column() == 1)
            {
                return QVariant(commandList[index.internalId()][index.row()].value());
            }
        }
        break;
    case Qt::ToolTipRole:
        //case Qt::StatusTipRole:
        if(index.parent().isValid() && (index.parent().internalId() == CATEGORY_INTERNAL_ID) && index.row() != -1)
        {
            //qDebug("data - %d %lld %d",role,index.internalId(),index.row());
            XBeeCommand cmd = commandList[index.internalId()][index.row()];
            QString comment;
            if((cmd.type() & XBC_READABLE) && (cmd.type() & XBC_WRITABLE))
            {
                //comment.append(" - Read/Write");
            }
            else if(cmd.type() & XBC_READABLE)
            {
                comment.append("Read only - ");
            }
            else if(cmd.type() & XBC_WRITABLE)
            {
                comment.append("Write only - ");
            }
            else
            {
                comment.append("No paramter");
            }
            if(cmd.type() & XBC_STRING)
            {
                comment.append("String value");
            }
            else if(cmd.type() & XBC_NUMBER)
            {
                comment.append("Hex value");
                if(cmd.type() & XBC_WRITABLE)
                {
                    comment.append(" (0x");
                    comment.append(QString::number(cmd.minValue(), 16));
                    comment.append(" to 0x");
                    comment.append(QString::number(cmd.maxValue(), 16));
                    comment.append(")");
                }
                comment.append("  <");
                comment.append(QString::number(cmd.value().toInt(NULL, 16)));
                comment.append("d>");
            }
            return comment;
        }
        return QVariant();
    case Qt::BackgroundRole:
        if(index.parent().isValid() && (index.parent().internalId() == CATEGORY_INTERNAL_ID) && index.row() != -1)
        {
            if(index.column() == 1)
            {
                if(commandList[index.internalId()][index.row()].isModified())
                {
                    //return QVariant(Qt::yellow);
                    return QVariant(QColor(Qt::yellow));
                }
            }
        }
        return QVariant();
    default:
        return QVariant();
    }

    return QVariant();
}

QVariant XBeeCommandListModel::headerData ( int section, Qt::Orientation /*orientation*/, int role ) const
{
    switch(role)
    {
    case Qt::DisplayRole:
        switch(section)
        {
        case 0:
            return tr("AT Command");
        case 1:
            return tr("Value");
        }
    default:
        return QVariant();
    }
}

QModelIndex XBeeCommandListModel::index ( int row, int column, const QModelIndex & parent ) const
{
    //return createIndex(row,column,,QString("Data"));
    //qDebug("index %d Parent:%lld Row:%d Column:%d",parent.isValid(),parent.internalId(),row,column);
    if(!parent.isValid())
    {
        //qDebug("index Row:%d Column:%d",row,column);
        if(column == 0 && row >= 0)
        {
            return categories[row];
        }
        else
        {
            return QModelIndex();
        }
    }
    else if(parent.internalId() == CATEGORY_INTERNAL_ID)
    {
        if(row < 0)
        {
            return QModelIndex();
        }
        if(column == 0)
        {
            return commandList[parent.row()][row].commandIndex();
        }
        else if(column == 1)
        {
            return commandList[parent.row()][row].valueIndex();
        }
    }
    return QModelIndex();
}

QModelIndex XBeeCommandListModel::parent ( const QModelIndex & index ) const
{
    //qDebug("Parent %d",index.internalId());
    if(index.internalId() == CATEGORY_INTERNAL_ID)
        return QModelIndex();
    else
        return categories[index.internalId()];
}

bool XBeeCommandListModel::hasChildren ( const QModelIndex & parent ) const
{
    if(parent.internalId() == CATEGORY_INTERNAL_ID || parent == QModelIndex())
    {
        return true;
    }
    return false;
}

int XBeeCommandListModel::rowCount ( const QModelIndex & parent ) const
{
    if(!parent.isValid())
    {
        return categories.size();
    }
    else if(parent.internalId() == CATEGORY_INTERNAL_ID)
    {
        //qDebug("rowCount %d %d",parent.row(),commandList[parent.row()].size());
        return commandList[parent.row()].size();
    }

    return 0;
}

// -------------------------- Serial Port related ---------------------------

void XBeeCommandListModel::setSerialPort(SerialPort::SerialPort *newport)
{
    port = newport;
    if(port)
    {
        timer->start();
    }
    else
    {
        timer->stop();
    }
}

void XBeeCommandListModel::sendMessage(QString &message)
{
    sendMessage(message.toUtf8().data());
}

void XBeeCommandListModel::sendMessage(const char *message, int length)
{
    if(port)
    {
        if(length == -1)
        {
            length = strlen(message);
        }
        port->send(message, length);

        QString text;

        if(apiEnable)
        {
            /*
            text = QString("");
            for(int i = 0;i < length;i++){
                text.append(QString::number((unsigned char)message[i],16));
                text.append(" ");
            }
            */
            text = byteArrayToString(message, length);
        }
        else
        {
            text = QString(message);
        }
        emit sentMessage(text);
    }
}

void XBeeCommandListModel::checkPort()
{
    if(port)
    {
        unsigned char rbuf[1024 * 4 + 1];
        int len = port->read((char *)rbuf, sizeof(rbuf) - 1, 0);
        if(len)
        {
            if(apiEnable)
            {
                processAPI(rbuf, len);
            }
            else
            {
                rbuf[len] = 0;//Null terminate
                QString mess((char *)rbuf);
                emit receivedMessage(mess);

                QStringList lines = mess.split("\r");
                lines[0].insert(0, lastLine);
                for(int i = 0; i < lines.length() - 1; i++)
                {
                    processLine(lines[i]);
                }
                lastLine = lines.last();
            }
        }
    }
}

// ----------------------- XBEE related -----------------


void XBeeCommandListModel::setNetworkAddress(unsigned int newna)
{
    _networkAddress = newna;
}

void XBeeCommandListModel::setDestinationAddress(qulonglong newda)
{
    _destinationAddress = newda;
}

unsigned int XBeeCommandListModel::networkAddress()
{
    return _networkAddress;
}

qulonglong XBeeCommandListModel::destinationAddress()
{
    return _destinationAddress;
}

void XBeeCommandListModel::sendTransmitRequest(QString mess)
{
    if(port)
    {
        mess.replace("\n", "\r");
        if(apiEnable)
        {
            QByteArray bytes ;
            QByteArray messageBytes = mess.toUtf8();
            unsigned int length = messageBytes.length() + 14;
            bytes.append(0x7e);//start
            bytes.append((unsigned char)(length >> 8));
            bytes.append((unsigned char)length);
            bytes.append(0x10);//transmit request
            bytes.append(0x10);//frame id
            for(int i = 0; i < 8; i++)
            {
                bytes.append((unsigned char)(_destinationAddress >> (7 - i) * 8));
            }
            for(int i = 0; i < 2; i++)
            {
                bytes.append((unsigned char)(_networkAddress >> (1 - i) * 8));
            }
            bytes.append((char)0x00);//Broadcast Radius
            bytes.append((char)0x00);//Option
            bytes.append(messageBytes);

            unsigned char checksum = 0;
            for(int i = 0; i < bytes.length() - 3; i++)
            {
                checksum += bytes[i + 3];
            }
            bytes.append(0xff - checksum);
            sendMessage(bytes.data(), bytes.length());

            QString html = QString("<table border=\"1\">"
                                   "<tr><th>Packet type</th><td><font color=\"blue\">Transmit Request</font></td></tr>"
                                   "<tr><th>Destination Address</th><td>%1</td></tr>"
                                   "<tr><th>Network Address</th><td>%2</td></tr>"
                                   "<tr><th>Contents</th><td>%3</td></tr>"
                                   "</table>")
                           .arg(_destinationAddress, 0, 16)
                           .arg(_networkAddress, 0, 16)
                           .arg(mess);
            statusMessage(html);
        }
        else
        {
            sendMessage(mess);
        }
    }
}

bool XBeeCommandListModel::isAllowedCommand(int category, int num, bool write)
{
    if(commandList[category].size() == num)
        return false;
    if(write && !commandList[category][num].isModified())
        return false;
    if(!write && !commandList[category][num].isReadable())
        return false;
    return true;
}

void XBeeCommandListModel::commandTimeout()
{
    //qDebug("Timeout");
    mode = XBEE_NONE;
#if 0
    QMessageBox mesbox;
    mesbox.setText("Cannot read/write value.");
    mesbox.setInformativeText("XBee doesn't answer. Please check baudrate and connection.");
    mesbox.setIcon(QMessageBox::Warning);
    mesbox.exec();
#endif
    emit errorMessage("Timeout");
}

void XBeeCommandListModel::processLine(QString line)
{
    switch(mode)
    {
    case XBEE_READ:
    case XBEE_WRITE:
    {
        timeoutTimer->stop();
        if(currentCategory != -1)
        {
            if(mode == XBEE_READ)
            {
                commandList[currentCategory][currentCommand].setValue(line);
                if(commandList[currentCategory][currentCommand].command() == "DH")
                {
                    _destinationAddress = ((qlonglong)line.toInt(NULL, 16) << 32) |
                                          (_destinationAddress & (qlonglong)0xffffffff);
                    emit destinationChanged();
                }
                else if(commandList[currentCategory][currentCommand].command() == "DL")
                {
                    _destinationAddress = (line.toInt(NULL, 16)) |
                                          (_destinationAddress & (qlonglong)0xffffffff00000000LL);
                    emit destinationChanged();
                }
                else if(commandList[currentCategory][currentCommand].command() == "PA")
                {
                    _networkAddress = (line.toInt(NULL, 16));
                    emit destinationChanged();
                }
            }
            commandList[currentCategory][currentCommand].setModified(false);
            emit dataChanged(commandList[currentCategory][currentCommand].valueIndex(),
                             commandList[currentCategory][currentCommand].valueIndex());
        }
        else
        {
            currentCategory = 0;
        }

        if(!sendNextATCommand())
        {
            QString cmd = "ATCN\r";
            sendMessage(cmd);
            mode = XBEE_NONE;
            return;
        }
        break;
    }
    default:
        break;
    }
}

void XBeeCommandListModel::checkChecksum()
{
    if(checksum == 0xff)
    {
        emit statusMessage(QString("&lt;OK"));
    }
    else
    {
        emit statusMessage(QString("&lt;ERROR"));
    }
    //qDebug("API Finish");
    xbeeAPIStatus = XBA_NONE;
}

void XBeeCommandListModel::processAPI(unsigned char *buf, int length)
{
    for(unsigned char *end = buf + length; buf < end; buf++)
    {
        QString mess;
        mess.append(QString("%1").arg((unsigned char)*buf, 2, 16, QChar('0')));
        mess.append(" ");
        emit receivedMessage(mess);
        checksum += *buf;

        switch(xbeeAPIStatus)
        {
        case XBA_NONE:
            if(*buf == 0x7E)
            {
                apiLength = 4;
                apiReceivedLength = 0;
                xbeeAPIStatus = XBA_HEAD;
                receviedPacket.targetAddress = 0;
                receviedPacket.networkAddress = 0;
                emit statusMessage(QString("&gt;"));
            }
            break;
        case XBA_HEAD:
            switch(apiReceivedLength)
            {
            case 1:
                apiLength = *buf << 8;
                break;
            case 2:
                apiLength |= *buf;
                apiLength += 4;
                checksum = 0;
                break;
            case 3:
            {
                switch(*buf)
                {
                case 0x90:
                    xbeeAPIStatus = XBA_RECEIVE;
                    break;
                case 0x8b:
                    xbeeAPIStatus = XBA_TRANSMIT_STATUS;
                    break;
                case 0x88:
                    xbeeAPIStatus = XBA_APIRES;
                    break;
                case 0x8a:
                    xbeeAPIStatus = XBA_MODEM_STATUS;
                    break;
                case 0x97:
                    xbeeAPIStatus = XBA_REMOTE_APIRES;
                    break;
                default:
                    xbeeAPIStatus = XBA_UNKOWN;
                    break;
                }
            }
            }
            break;
        case XBA_RECEIVE:
            if(apiReceivedLength < 12)
            {
                receviedPacket.targetAddress <<= 8;
                receviedPacket.targetAddress |= (uint8_t) * buf;
            }
            else if(apiReceivedLength < 14)
            {
                receviedPacket.networkAddress <<= 8;
                receviedPacket.networkAddress |= (uint8_t) * buf;
            }
            else if(apiReceivedLength < 15)
            {
                receviedPacket.receiveBuffer.clear();
                if(*buf == 2)
                    receviedPacket.broadcast = true;
                else
                    receviedPacket.broadcast = false;
            }
            else if(apiReceivedLength < apiLength - 1)
            {
                receviedPacket.receiveBuffer.append(*buf);
            }
            else
            {
                checkChecksum();
                receviedPacket.receiveBuffer.append((char)0);
                QString contents(receviedPacket.receiveBuffer);
                contents.replace("\r\n", "<br>");
                contents.replace("\n", "<br>");
                contents.replace("\r", "<br>");
                QString html = QString("<table border=\"1\">"
                                       "<tr><th>Packet type</th><td><font color=\"red\">Receive packet</font></td></tr>"
                                       "<tr><th>Network Address</th><td>%1</td></tr>"
                                       "<tr><th>Sender Address</th><td>%2</td></tr>"
                                       "<tr><th>Broadcast</th><td>%3</td></tr>"
                                       "<tr><th>Contents</th><td><pre>%4</pre></td></tr>"
                                       "</table>")
                               .arg(receviedPacket.networkAddress, 4, 16)
                               .arg(receviedPacket.targetAddress, 16, 16)
                               .arg(receviedPacket.broadcast ? "true" : "false")
                               .arg(contents);
                emit statusMessage(html);
            }
            break;
        case XBA_APIRES:
            if(apiReceivedLength < 5)
            {
                apiResponse.frameId = (unsigned char) * buf;
                apiResponse.length = apiLength - 9;
                apiResponse.receiveBuffer.clear();
            }
            else if(apiReceivedLength < 7)
            {
                apiResponse.atcommand[apiReceivedLength - 5] = *buf;
            }
            else if(apiReceivedLength < 8)
            {
                apiResponse.status = *buf;
            }
            else if(apiReceivedLength < apiLength - 1)
            {
                apiResponse.receiveBuffer.append((unsigned char)*buf);
            }
            else
            {
                checkChecksum();
                apiResponse.atcommand[2] = 0;

                QString mes;
                unsigned long numvalue = 0;

                if(QString("ND").compare(apiResponse.atcommand) == 0)
                {
                    mes = processAPI_NodeDiscover(apiResponse.receiveBuffer);
                }
                else
                {
                    for(int i = 0; i < apiResponse.receiveBuffer.length(); i++)
                    {
                        mes.append(QString("%1").arg((unsigned char)apiResponse.receiveBuffer[i], 2, 16, QChar('0')));
                        numvalue <<= 8;
                        numvalue |= (unsigned char)apiResponse.receiveBuffer[i];
                        mes.append(" ");
                    }
                }

                QString statusText;
                switch(apiResponse.status)
                {
                case 0:
                    statusText = "OK";
                    break;
                case 1:
                    statusText = "ERROR";
                    break;
                case 2:
                    statusText = "Invalid Command";
                    break;
                case 3:
                    statusText = "Invalid Parameter";
                    break;
                default:
                    statusText = "Unkown";
                    break;
                }

                QString html = QString("<table border=\"1\">"
                                       "<tr><th>Packet type</th><td><font color=\"red\">AT Command Response</font></td></tr>"
                                       "<tr><th>AT Command</th><td>%1</td></tr>"
                                       "<tr><th>Status</th><td>%2</td></tr>"
                                       "%3"
                                       "</table>")
                               .arg(apiResponse.atcommand)
                               .arg(statusText)
                               .arg(mes.length() ? QString("<tr><th>Contents</th><td>%1</td></tr>").arg(mes) : "");
                emit statusMessage(html);

                if(mode == XBEE_READ || mes.length())
                {
                    QString line;
                    if(searchCommand(apiResponse.atcommand).type() & XBC_STRING)
                    {
                        line = QString(apiResponse.receiveBuffer);
                    }
                    else
                    {
                        line = QString::number(numvalue, 16);
                    }
                    searchCommand(apiResponse.atcommand).setValue(line);

                    if(searchCommand(apiResponse.atcommand).command() == "DH")
                    {
                        _destinationAddress = ((qlonglong)line.toInt(NULL, 16) << 32) |
                                              (_destinationAddress & (qlonglong)0xffffffff);
                        emit destinationChanged();
                    }
                    else if(searchCommand(apiResponse.atcommand).command() == "DL")
                    {
                        _destinationAddress = (line.toInt(NULL, 16)) |
                                              (_destinationAddress & (qlonglong)0xffffffff00000000LL);
                        emit destinationChanged();
                    }
                    else if(searchCommand(apiResponse.atcommand).command() == "PA")
                    {
                        _networkAddress = (line.toInt(NULL, 16));
                        emit destinationChanged();
                    }
                }
                timeoutTimer->stop();
                if(mode == XBEE_READ || mode == XBEE_WRITE)
                {
                    searchCommand(apiResponse.atcommand).setModified(false);
                    emit dataChanged(searchCommand(apiResponse.atcommand).valueIndex(),
                                     searchCommand(apiResponse.atcommand).valueIndex());
                    XBeeCommandMode lastMode = mode;
                    usleep(20 * 1000);
                    if(!sendNextATCommand())
                    {
                        //qDebug("Finish write ro read.");
                        if(lastMode == XBEE_WRITE)
                        {
                            sendATCommand(searchCommand("AC"), false);
                        }
                    }
                }
                else if(mes.length())
                {
                    searchCommand(apiResponse.atcommand).setModified(false);
                    emit dataChanged(searchCommand(apiResponse.atcommand).valueIndex(),
                                     searchCommand(apiResponse.atcommand).valueIndex());
                }
            }
            break;
        case XBA_REMOTE_APIRES://API Remote Response
            if(apiReceivedLength < 5)
            {
                apiResponse.frameId = (unsigned char) * buf;
                apiResponse.length = apiLength - 9;
                apiResponse.networkAddress = 0;
                apiResponse.serialNumber = 0;
                apiResponse.receiveBuffer.clear();
            }
            else if(apiReceivedLength < 13)
            {
                apiResponse.serialNumber <<= 8;
                apiResponse.serialNumber |= (unsigned char) * buf;
            }
            else if(apiReceivedLength < 15)
            {
                apiResponse.networkAddress <<= 8;
                apiResponse.networkAddress |= (unsigned char) * buf;
            }
            else if(apiReceivedLength < 17)
            {
                apiResponse.atcommand[apiReceivedLength - 15] = *buf;
            }
            else if(apiReceivedLength < 18)
            {
                apiResponse.status = *buf;
            }
            else if(apiReceivedLength < apiLength - 1)
            {
                apiResponse.receiveBuffer.append((unsigned char)*buf);
            }
            else
            {
                checkChecksum();
                apiResponse.atcommand[2] = 0;

                QString mes;
                unsigned long numvalue = 0;

                if(QString("ND").compare(apiResponse.atcommand) == 0)
                {
                    mes = processAPI_NodeDiscover(apiResponse.receiveBuffer);
                }
                else
                {
                    for(int i = 0; i < apiResponse.receiveBuffer.length(); i++)
                    {
                        mes.append(QString("%1").arg((unsigned char)apiResponse.receiveBuffer[i], 2, 16, QChar('0')));
                        numvalue <<= 8;
                        numvalue |= (unsigned char)apiResponse.receiveBuffer[i];
                        mes.append(" ");
                    }
                }

                QString statusText;
                switch(apiResponse.status)
                {
                case 0:
                    statusText = "OK";
                    break;
                case 1:
                    statusText = "ERROR";
                    break;
                case 2:
                    statusText = "Invalid Command";
                    break;
                case 3:
                    statusText = "Invalid Parameter";
                    break;
                default:
                    statusText = "Unkown";
                    break;
                }

                QString html = QString("<table border=\"1\">"
                                       "<tr><th>Packet type</th><td><font color=\"red\">Remote AT Command Response</font></td></tr>"
                                       "<tr><th>AT Command</th><td>%1</td></tr>"
                                       "<tr><th>Status</th><td>%2</td></tr>"
                                       "<tr><th>Network Address</th><td>%3</td></tr>"
                                       "<tr><th>Target Address</th><td>%4</td></tr>"
                                       "%5"
                                       "</table>")
                               .arg(apiResponse.atcommand)
                               .arg(statusText)
                               .arg(apiResponse.networkAddress, 0, 16)
                               .arg(apiResponse.serialNumber, 0, 16)
                               .arg(mes.length() ? QString("<tr><th>Contents</th><td>%1</td></tr>").arg(mes) : "");
                emit statusMessage(html);

                if(mode == XBEE_READ || mes.length())
                {
                    QString line;
                    if(searchCommand(apiResponse.atcommand).type() & XBC_STRING)
                    {
                        line = QString(apiResponse.receiveBuffer);
                    }
                    else
                    {
                        line = QString::number(numvalue, 16);
                    }
                    searchCommand(apiResponse.atcommand).setValue(line);
                    //commandList[currentCategory][currentCommand].setValue(line);
                }
                timeoutTimer->stop();
                if(mode == XBEE_READ || mode == XBEE_WRITE)
                {
                    searchCommand(apiResponse.atcommand).setModified(false);
                    emit dataChanged(searchCommand(apiResponse.atcommand).valueIndex(),
                                     searchCommand(apiResponse.atcommand).valueIndex());
                    XBeeCommandMode lastMode = mode;
                    if(!sendNextATCommand())
                    {
                        //qDebug("Finish write ro read.");
                        if(lastMode == XBEE_WRITE)
                        {
                            sendATCommand(searchCommand("AC"), false);
                        }
                    }
                }
                else if(mes.length())
                {
                    searchCommand(apiResponse.atcommand).setModified(false);
                    emit dataChanged(searchCommand(apiResponse.atcommand).valueIndex(),
                                     searchCommand(apiResponse.atcommand).valueIndex());
                }
            }
            break;
        case XBA_TRANSMIT_STATUS:
            if(apiReceivedLength < 5)
            {
                transmitStatus.frameId = (unsigned char) * buf;
                transmitStatus.networkAddress = 0;
            }
            else if(apiReceivedLength < 7)
            {
                transmitStatus.networkAddress <<= 8;
                transmitStatus.networkAddress |= (unsigned char) * buf;
            }
            else if(apiReceivedLength < 8)
            {
                transmitStatus.retryCount = (unsigned char) * buf;
            }
            else if(apiReceivedLength < 9)
            {
                transmitStatus.delivaryStatus = (unsigned char) * buf;
            }
            else if(apiReceivedLength < 10)
            {
                transmitStatus.discoveryStatus = (unsigned char) * buf;
            }
            else if(apiReceivedLength == apiLength - 1)
            {
                checkChecksum();

                QString delivaryStatus;
                switch(transmitStatus.delivaryStatus)
                {
                case 0:
                    delivaryStatus = "Success";
                    break;
                case 0x02:
                    delivaryStatus = "CCA Failure";
                    break;
                case 0x15:
                    delivaryStatus = "Invalid destination endpoint";
                    break;
                case 0x21:
                    delivaryStatus = "Network ACK Failure";
                    break;
                case 0x22:
                    delivaryStatus = "Not Joined to Network";
                    break;
                case 0x23:
                    delivaryStatus = "Self-addressed";
                    break;
                case 0x24:
                    delivaryStatus = "Address Not Found";
                    break;
                case 0x25:
                    delivaryStatus = "Route Not Found";
                    break;
                default:
                    delivaryStatus = QString("Unknown code %1").arg(transmitStatus.delivaryStatus);
                    break;
                }

                QString discoveryStatus;
                switch(transmitStatus.discoveryStatus)
                {
                case 0x00:
                    discoveryStatus = "No Discovery Overhead";
                    break;
                case 0x01:
                    discoveryStatus = "Address Discovery";
                    break;
                case 0x02:
                    discoveryStatus = "Route Discovery";
                    break;
                case 0x03:
                    discoveryStatus = "Address & Route Discovery";
                    break;
                default:
                    discoveryStatus = QString("Unknown code %1").arg(transmitStatus.discoveryStatus);
                    break;
                }

                QString html = QString("<table border=\"1\">"
                                       "<tr><th>Packet type</th><td><font color=\"red\">Transmit Status</font></td></tr>"
                                       "<tr><th>Network Address</th><td>%1</td></tr>"
                                       "<tr><th>Retry Count</th><td>%2</td></tr>"
                                       "<tr><th>Delivary Status</th><td>%3</td></tr>"
                                       "<tr><th>Discovery Status</th><td>%4</td></tr>"
                                       "</table>")
                               .arg(transmitStatus.networkAddress, 0, 16)
                               .arg(transmitStatus.retryCount)
                               .arg(delivaryStatus)
                               .arg(discoveryStatus);
                statusMessage(html);

            }
            break;
        case XBA_MODEM_STATUS:
            if(apiReceivedLength < 5)
            {
                modemStatus.status = (unsigned char) * buf;
            }
            else if(apiReceivedLength == apiLength - 1)
            {
                checkChecksum();
                QString status;
                switch(modemStatus.status)
                {
                case 0:
                    status = "Hardware reset";
                    break;
                case 1:
                    status = "Software reset";
                    break;
                case 2:
                    status = "Associated";
                    break;
                default:
                    status = QString("Unknown code (%1)").arg(modemStatus.status);
                    break;
                }
                QString html = QString("<table border=\"1\">"
                                       "<tr><th>Packet type</th><td><font color=\"red\">Modem Status</font></td></tr>"
                                       "<tr><th>Status</th><td>%1</td></tr>"
                                       "</table>")
                               .arg(status);
                statusMessage(html);
            }
            break;
        default:
            if(apiReceivedLength == apiLength - 1)
            {
                statusMessage(" Unknown API Command ");
                checkChecksum();
            }
            break;
        }

        apiReceivedLength++;
    }
}

void XBeeCommandListModel::readParamters()
{
    if(port)
    {
        mode = XBEE_READ;
        //qDebug("READ");
        if(apiEnable)
        {
            currentCategory = 0;
            currentCommand = -1;
            sendNextATCommand();
        }
        else
        {
            currentCategory = -1;
            currentCommand = -1;
            QString commandMode = "+++";
            sendMessage(commandMode);
            timeoutTimer->start();
        }
    }
    else
    {
        QMessageBox mesbox;
        mesbox.setText("Cannot read value.");
        mesbox.setInformativeText("Serial port is not opened.");
        mesbox.setIcon(QMessageBox::Warning);
        mesbox.exec();
    }
}


void XBeeCommandListModel::writeParameters()
{
    if(port)
    {
        mode = XBEE_WRITE;
        if(apiEnable)
        {
            currentCategory = 0;
            currentCommand = -1;
            sendNextATCommand();
        }
        else
        {
            currentCategory = -1;
            currentCommand = -1;
            QString commandMode = "+++";
            sendMessage(commandMode);
            timeoutTimer->start();
        }
    }
    else
    {
        QMessageBox mesbox;
        mesbox.setText("Cannot write value.");
        mesbox.setInformativeText("Serial port is not opened.");
        mesbox.setIcon(QMessageBox::Warning);
        mesbox.exec();
    }
}

void XBeeCommandListModel::apiMode(bool enable)
{
    apiEnable = enable;
}

void XBeeCommandListModel::setRemoteConfigure(bool enable)
{
    remoteConfigureEnable = enable;
}

XBeeCommand& XBeeCommandListModel::searchCommand(QString atcommand)
{
    for(int i = 0; i < commandList.size(); i++)
    {
        for(int j = 0; j < commandList[i].size(); j++)
        {
            if(commandList[i][j].command().compare(atcommand) == 0)
            {
                return commandList[i][j];
            }
        }
    }
    return commandList[0][0];
}

bool XBeeCommandListModel::sendNextATCommand()
{
    if(mode == XBEE_READ || mode == XBEE_WRITE)
    {
        do
        {
            currentCommand++;

            if(commandList[currentCategory].size() == currentCommand)
            {
                currentCategory++;
                currentCommand = 0;

                if(categories.length() == currentCategory)
                {
                    mode = XBEE_NONE;
                    return false;
                }
            }
        }
        while(!isAllowedCommand(currentCategory, currentCommand, mode == XBEE_WRITE));

        sendATCommand(commandList[currentCategory][currentCommand], mode == XBEE_WRITE);

        return true;
    }
    return false;
}

void XBeeCommandListModel::sendATCommand(XBeeCommand& command, bool write)
{
    QString cmd = command.command();
    if(apiEnable)
    {
        //int length = 4;
        QByteArray buf;
        buf.append(0x7e); // Start
        buf.append((char)0x00); // Length temporary zero MSB
        buf.append((char)0x00); // Length temporary zero LSB
        buf.append(0x09); // Command Type

        QByteArray atcommand = cmd.toLatin1();//cmd.toAscii();

        buf.append(0x9);//frame id
        buf.append(atcommand[0]);
        buf.append(atcommand[1]);

        QByteArray contents;
        QString apiType;
        QString additionalLine;
        if(!write)
        {
            apiType = "AT Command";
            buf[3] = 0x08;
        }
        else
        {
            apiType = "AT Command - Queue Parameter Value";
            buf.append(contents = command.data());
        }

        if(remoteConfigureEnable)
        {
            apiType = "Remote AT Command";
            buf[3] = 0x17;
            buf[4] = _destinationAddress & 0xff;//set frame id
            for(int i = 0; i < 8; i++) // set destination address
            {
                buf.insert(5, (unsigned char)(_destinationAddress >> 8 * i));
            }
            for(int i = 0; i < 2; i++) // set network address
            {
                buf.insert(13, (unsigned char)(_networkAddress >> 8 * i));
            }
            buf.insert(15, (char)0); // Queue

            additionalLine = QString("<tr><th>Network Address</th><td>%1</td></tr>"
                                     "<tr><th>Destination Address</th><td>%2</td></tr>")
                             .arg(_networkAddress, 0, 16)
                             .arg(_destinationAddress, 0, 16);
        }

        // common data
        buf[1] = (unsigned char)((buf.size() - 3) >> 8);
        buf[2] = (unsigned char)(buf.size() - 3);

        //qDebug("%x %x | %x %x",buf[5],buf[6],atcommand[0],atcommand[1]);

        //calc checksum
        unsigned char checksum = 0;
        for(int i = 0; i < buf.length() - 3; i++)
        {
            checksum += buf[i + 3];
        }
        //qDebug("Checksum %02x",checksum);
        //buf[length+3] = 0xff - checksum;
        buf.append(0xff - checksum);

        //usleep(1000);
        sendMessage(buf.data(), buf.length());

        QString html = QString("<table border=\"1\">"
                               "<tr><th>Packet type</th><td><font color=\"blue\">%1</font></td></tr>"
                               "<tr><th>AT Command</th><td>%2</td></tr>"
                               "%3%4"
                               "</table>")
                       .arg(apiType)
                       .arg(cmd)
                       .arg(write ? QString("<tr><th>Contents</th><td><pre>%3</pre></td></tr>")
                            .arg(byteArrayToString(contents)) : "")
                       .arg(additionalLine);
        emit statusMessage(html);

        //delete buf;
    }
    else
    {
        cmd.insert(0, "AT");
        if(write)
        {
            cmd.append(" ");
            cmd.append(command.value());
        }
        cmd.append("\r");

        sendMessage(cmd);
    }
    if(command.isReadable() || command.isWritable())
    {
        timeoutTimer->start();
    }
}

void XBeeCommandListModel::sendATCommandRequest(QModelIndex index)
{
    if(port)
    {
        if(index.column() == 0 && index.internalId() != CATEGORY_INTERNAL_ID)
            sendATCommand(commandList[index.internalId()][index.row()], false);
    }
}
