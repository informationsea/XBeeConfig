#ifndef SERIALPORTPOSIX_H
#define SERIALPORTPOSIX_H

#include "serialport.h"

#ifdef __APPLE__
#define __POSIX__
#define SERIALPORT_PREFIX "cu."
#define SERIALPORT_DEV "/dev/"
#elif defined(__linux__)
#define __POSIX__
#define SERIALPORT_PREFIX "ttyUSB"
#define SERIALPORT_DEV "/dev/"
#elif defined(__unix__)
#error This Operation System is not supported.
#endif

#ifdef __POSIX__

#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <paths.h>
#include <termios.h>
#include <sysexits.h>
#include <sys/param.h>
#include <sys/select.h>
#include <sys/time.h>
#include <time.h>
#include <dirent.h>

namespace SerialPort{
    class SerialPortPosix : public SerialPort
    {
    public:
        SerialPortPosix(const std::string port,long baudrate);
        virtual ~SerialPortPosix(){close();}

        virtual bool connect();
        virtual bool isConnected();
        virtual void close();
        virtual int send(const char *buf,int length);
        virtual int read(char *buf,int length,int timeout_ms);
    private:
        bool connected;
        int fd;
        struct termios newtio, oldtio;
    };
}

#endif // __POSIX__

#endif // SERIALPORTPOSIX_H
