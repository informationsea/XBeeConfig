#include "serialportwindows.h"

#ifdef __WIN32__

#define SERIAL_PREFIX "\\\\.\\"

using namespace std;

namespace SerialPort
{

SerialPortWindows::SerialPortWindows(const std::string _port, long _baudrate):
    connected(false), hCom(NULL)
{
    port = _port;
    if(port.compare(0, strlen(SERIAL_PREFIX), SERIAL_PREFIX)) // Not equal
    {
        port.insert(0, SERIAL_PREFIX);
    }

    //qDebug(">>%s",port.c_str());

    switch(_baudrate)
    {
    case 9600:
        baudrate = CBR_9600;
        break;
    case 38400:
        baudrate = CBR_38400;
        break;
    case 115200:
        baudrate = CBR_115200;
        break;
    default:
        fprintf(stderr, "Baudrate %d is not supported.", _baudrate);
        baudrate = CBR_9600;
        break;
    }
}

bool SerialPortWindows::connect()
{
    hCom = NULL;

    //char szBuf[256];
    DCB dcb;
    //COMMTIMEOUTS timeouts;

    /* open COM port */

    hCom = CreateFileA(port.c_str(), GENERIC_WRITE | GENERIC_READ, 0, NULL,
                       OPEN_EXISTING, 0, NULL);
    if(hCom == NULL)
    {
        fprintf(stderr, "ERROR: can't open COM port!\n");
        return false;
    }

    /* set DCB structure */
    GetCommState(hCom, &dcb);
    dcb.BaudRate = baudrate;
    dcb.ByteSize = 8;
    dcb.Parity = NOPARITY;
    dcb.StopBits = ONESTOPBIT;
    if (!SetCommState(hCom, &dcb))
    {
        fprintf(stderr, "ERROR: can't set DCB structure!\n");
        CloseHandle(hCom);
        hCom = NULL;
        return false;
    }
#if 0
    /* set timeouts */
    GetCommTimeouts(hCom, &timeouts);
    timeouts.ReadIntervalTimeout = 100;
    timeouts.ReadTotalTimeoutMultiplier = 100;
    timeouts.ReadTotalTimeoutConstant = 100;
    timeouts.WriteTotalTimeoutMultiplier = 100;
    timeouts.WriteTotalTimeoutConstant = 100;
    if (!SetCommTimeouts(hCom, &timeouts))
    {
        fprintf(stderr, "ERROR: can't set timeouts!\n");
        CloseHandle(hCom);
        hCom = NULL;
        return false;
    }
#endif
    /* clear interface */
    if (!PurgeComm(hCom, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR))
    {
        fprintf(stderr, "ERROR: can't clear interface!\n");
        CloseHandle(hCom);
        hCom = NULL;
        return false;
    }

    connected = true;

    return true;
}
bool SerialPortWindows::isConnected()
{
    return connected;
}
void SerialPortWindows::close()
{
    if (hCom == NULL)
    {
        return;
    }

    CloseHandle(hCom);
    hCom = NULL;
}

int SerialPortWindows::send(const char *buf, int length)
{
    if(!hCom)
    {
        fprintf(stderr, "ERROR: Serial Port is not usable.");
        return false;
    }

    unsigned int result;
    WriteFile(hCom, buf, (DWORD)length, (LPDWORD)&result, NULL);
    if(result != length)
    {
        return false;
    }
    return true;
}

int SerialPortWindows::read(char *buf, int length, int timeout_ms)
{
    if(!hCom)
    {
        fprintf(stderr, "ERROR: Serial Port is not usable.");
        return -1;
    }

    unsigned int result;

    COMMTIMEOUTS timeouts;
    timeouts.ReadIntervalTimeout = 1;
    timeouts.ReadTotalTimeoutMultiplier = 0;
    timeouts.ReadTotalTimeoutConstant = (timeout_ms == 0 ? 1 : timeout_ms);
    timeouts.WriteTotalTimeoutConstant = 1;
    timeouts.WriteTotalTimeoutMultiplier = 1;

    if (!SetCommTimeouts(hCom, &timeouts))
    {
        fprintf(stderr, "ERROR: can't set timeouts!\n");
        CloseHandle(hCom);
        hCom = NULL;
        return -1;
    }

    //printf("READ Start\n");
    ReadFile(hCom, buf, length, (LPDWORD)&result, NULL);
    //printf("Read End\n");

    return result;
}


std::vector<std::string> getPortList()
{
    vector<string> list;

    int i;
    char buf[20];
    HANDLE hCom;
    for(i = 0; i < 50; i++)
    {
        sprintf(buf, "\\\\.\\COM%d", i + 1);
        hCom = NULL;
        hCom = CreateFileA(buf, GENERIC_WRITE | GENERIC_READ, 0, NULL,
                           OPEN_EXISTING, 0, NULL);
        if(hCom != INVALID_HANDLE_VALUE )
        {
            sprintf(buf, "COM%d", i + 1);
            list.push_back(buf);
            CloseHandle(hCom);
        }
    }

    return list;
}

SerialPort* getPort(const std::string port, long baudrate)
{
    return new SerialPortWindows(port, baudrate);
}
}

#endif // __WIN32__
