#include "serialportposix.h"

#ifdef __POSIX__

using namespace std;

namespace SerialPort
{
SerialPortPosix::SerialPortPosix(const std::string _port, long _baudrate):
    fd(-1)
{
    port = _port;
    if(port.compare(0, strlen(SERIALPORT_DEV), SERIALPORT_DEV)) // Not equal
    {
        port.insert(0, SERIALPORT_DEV);
    }

    switch(_baudrate)
    {
    case 9600:
        baudrate = B9600;
        break;
    case 38400:
        baudrate = B38400;
        break;
    case 115200:
        baudrate = B115200;
        break;
    default:
        fprintf(stderr, "This baudrate is not supported.\n");
        baudrate = B9600;
        break;
    }
}

bool SerialPortPosix::connect()
{
    fd = open(port.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd == -1)
    {
        perror("Cannot open serial port.");
        goto onerror;
    }

    if (ioctl(fd, TIOCEXCL) == -1)  // prevent multiple open
    {
        perror("Some proccess using this file.");
        goto onerror;
    }

    if (fcntl(fd, F_SETFL, 0) == -1)  // clear the O_NONBLOCK flag
    {
        perror("Cannot clear nonblock flag");
        goto onerror;
    }

    if (tcgetattr(fd, &oldtio) == -1)  /* 現在の設定を退避 */
    {
        perror("Cannot get attr.");
        goto onerror;
    }

    newtio = oldtio; // copy setting structure


    cfmakeraw(&newtio); // Raw mode
    newtio.c_cc[VTIME] = 1; /* キャラクタ間タイマは未使用 */
    newtio.c_cc[VMIN] = 1; /* 1文字受け取るまでブロックする */
    cfsetspeed(&newtio, baudrate); // set speed

    tcflush(fd, TCIFLUSH); /* 入力をフラッシュ */

    if(tcsetattr(fd, TCSANOW, &newtio) == -1)  /* 設定を即時適用 */
    {
        perror("Cannot set attr.");
        goto onerror;
    }

    connected = true;
    return fd;

onerror:
    if(fd != -1)
    {
        ::close(fd);
        fd = -1;
    }
    fprintf(stderr, "%s\n", port.c_str());
    return false;
}

void SerialPortPosix::close()
{
    if (connected)
    {
        tcsetattr(fd, TCSANOW, &oldtio);
        //if(errno != EINTR){
        ::close(fd);
        //}
        connected = false;
    }
}

bool SerialPortPosix::isConnected()
{
    return connected;
}

int SerialPortPosix::send(const char *buf, int length)
{
    if (connected)
    {
        if(write(fd, buf, length) != length)
        {
            goto onerror;
        }
        return true;
    }
    return false;
onerror:
    perror("Serial Port Write");
    close();
    return false;
}

int SerialPortPosix::read(char *buf, int length, int timeout)
{
    if (connected)
    {
        struct timeval ti = { 0, timeout };
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(fd, &fds);
        if(select(fd + 1, &fds, NULL, NULL, timeout < 0 ? NULL : &ti) >= 0)
        {
            if (FD_ISSET(fd, &fds))
            {
                memset(buf, 0, length);
                int len = ::read(fd, buf, length - 1);
                if(len <= 0)
                {
                    goto onerror;
                }
                buf[len] = 0;
                //printf("%s",buf);
                return len;
            }
        }
        else
        {
            goto onerror;
        }
        return 0;
    }
    return -1;
onerror:
    perror("Serial Port Read");
    close();
    printf("Serial Port is closed\n");
    return -2;
}


std::vector<std::string> getPortList()
{
    DIR* dir = opendir("/dev/");
    vector<string> list;
    if(dir == NULL)
    {
        perror("Open Directory Error");
        return list;
    }
    dirent* entry;
    while((entry = readdir(dir)) != NULL)
    {
        if(::strncmp(SERIALPORT_PREFIX, entry->d_name, strlen(SERIALPORT_PREFIX)) == 0)
        {
            list.push_back(entry->d_name);
        }
    }
    return list;
}

SerialPort* getPort(const std::string port, long baudrate)
{
    return new SerialPortPosix(port, baudrate);
}
}
#endif
