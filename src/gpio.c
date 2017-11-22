#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include "gpio.h"




//Register GPIO in linux kernel
void gpioExport(int gpio)
{
    int fd;
    char buf[255];
    fd = open("/sys/class/gpio/export", O_WRONLY);
    sprintf(buf, "%d", gpio); 
    write(fd, buf, strlen(buf));
    close(fd);
}

void gpioUnExport(int gpio)
{
    int fd;
    char buf[255];
    fd = open("/sys/class/gpio/unexport", O_WRONLY);
    sprintf(buf, "%d", gpio);
    write(fd, buf, strlen(buf));
    close(fd);
}

//Open file descriptor
int gpioOpen(int gpio)
{
    int fd;
    char buf[255];
    sprintf(buf, "/sys/class/gpio/gpio%d/value", gpio);
    fd = open(buf, O_WRONLY);
    return fd;
}

//Close file descripto
int gpioClose(int fd)
{
    close(fd);
}

//Data direction
void gpioDirection(int gpio, int direction) // 1 for output, 0 for input
{
    int fd;
    char buf[255];
    sprintf(buf, "/sys/class/gpio/gpio%d/direction", gpio);
    fd = open(buf, O_WRONLY);

    if (direction)
    {
        write(fd, "out", 3);
    }
    else
    {
        write(fd, "in", 2);
    }
    close(fd);
}

//Set GPIO, '1' = 1, '0' = 0
void gpioSet(int gpioHandler, char value)
{
    write(gpioHandler, &value, 1);
}


