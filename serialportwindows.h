#ifndef SERIALPORTWINDOWS_H
#define SERIALPORTWINDOWS_H

#include "serialport.h"

#ifdef __WIN32__

#include <windows.h>

namespace SerialPort{
    class SerialPortWindows : public SerialPort
    {
    public:
        SerialPortWindows(const std::string port,long baudrate);
        virtual ~SerialPortWindows(){close();}

        virtual bool connect();
        virtual bool isConnected();
        virtual void close();
        virtual int send(const char *buf,int length);
        virtual int read(char *buf,int length,int timeout_ms);
    private:
        bool connected;
        HANDLE hCom;
    };
}

#endif // __WIN32__

#endif // SERIALPORTWINDOWS_H
