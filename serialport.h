#ifndef SERIALPORT_H
#define SERIALPORT_H

#include <cstdio>
#include <cstdlib>
#include <vector>
#include <string>

namespace SerialPort{
    class SerialPort
    {
    public:
        SerialPort();
        virtual ~SerialPort(){}

        virtual bool connect() = 0;
        virtual bool isConnected() = 0;
        virtual void close() = 0;
        virtual int send(const char *buf,int length) = 0;
        virtual int read(char *buf,int length,int timeout_ms) = 0;

        virtual std::string getPort(){return port;}
        virtual long getBaudrate(){return baudrate;}

    protected:
        std::string port;
        long baudrate;
    };

    std::vector<std::string> getPortList();
    SerialPort* getPort(const std::string,long baudrate);

}

#endif // SERIALPORT_H
