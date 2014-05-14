#include "xbeecommandhelper.h"

QString byteArrayToString(const char * data, int length)
{
    QString text("");
    for(int i = 0; i < length; i++)
    {
        //text.append(QString::number((unsigned char)data[i],16));
        text.append(QString("%1").arg((unsigned char)data[i], 2, 16, QChar('0')));
        text.append(" ");
    }
    return text;
}

QString byteArrayToString(const QByteArray& data)
{
    return byteArrayToString(data.data(), data.size());
}

QString processAPI_NodeDiscover(const QByteArray& data)
{
    QString html = "<table border=\"1\">";
    html.append("<tr><th>Serial Number</th><th>Node Identifier</th>"
                "<th>Network Address</th><th>Device Type</th><th>Profile ID</th>"
                "<th>Manifacture ID</th></tr>");
    int parseStep = 0;
    int readBytes = 0;
    qulonglong serialNumber = 0;
    unsigned short networkAddress = 0;
    unsigned short profileId = 0;
    unsigned short manifactureId = 0;
    QByteArray nodeIdentifier;
    QString deviceType;
    for(int i = 0; i < data.length(); i++)
    {
        switch(parseStep)
        {
        case 0:
            if((unsigned char)data[i] == 0xff)
                parseStep = 1;
            break;
        case 1:
            if((unsigned char)data[i] == 0xfe) //initialize
            {
                parseStep = 2;
                serialNumber = 0;
                profileId = 0;
                manifactureId = 0;
                readBytes = 0;
                html.append("<tr>");
            }
            else
            {
                parseStep = 0;
            }
            break;
        case 2://serial number
            serialNumber <<= 8;
            serialNumber |= (unsigned char)data[i];
            readBytes++;
            if(readBytes == 8)
            {
                readBytes = 0;
                parseStep = 3;
                html.append(QString("<td>%1</td>").arg(serialNumber, 0, 16));
            }
            break;
        case 3://node identifier
            nodeIdentifier.append(data[i]);
            if(data[i] == 0)
            {
                parseStep = 4;
                html.append(QString("<td>%1</td>").arg(QString(nodeIdentifier)));
            }
            break;
        case 4://network address
            networkAddress <<= 8;
            networkAddress |= (unsigned char)data[i];
            readBytes++;
            if(readBytes == 2)
            {
                readBytes = 0;
                parseStep = 5;
                html.append(QString("<td>%1</td>").arg(networkAddress, 0, 16));
            }
            break;
        case 5://device type
            switch(data[i])
            {
            case 0:
                deviceType = "Coordinator";
                break;
            case 1:
                deviceType = "Router";
                break;
            case 2:
                deviceType = "End device";
                break;
            }
            html.append(QString("<td>%1</td>").arg(deviceType));
            parseStep++;
            break;
        case 6://status(reserved)
            parseStep++;
            break;
        case 7://profile id
            profileId <<= 8;
            profileId |= (unsigned char)data[i];
            readBytes++;
            if(readBytes == 2)
            {
                readBytes = 0;
                parseStep = 8;
                html.append(QString("<td>%1</td>").arg(profileId, 0, 16));
            }
            break;
        case 8://Manifacture ID
            manifactureId <<= 8;
            manifactureId |= (unsigned char)data[i];
            readBytes++;
            if(readBytes == 2)
            {
                readBytes = 0;
                parseStep = 0;
                html.append(QString("<td>%1</td></tr>").arg(manifactureId, 0, 16));

                // finalize
            }
            break;
        }
    }
    html.append("</table>");
    return html;
}
