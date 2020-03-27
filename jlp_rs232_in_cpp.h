/********************************************************************
* From: https://github.com/kosme/grbl0.8-Kosme/blob/master/software/rs232.h
* (Version of 07/12/2015)
*
* JLP
* Version 08/12/2015
********************************************************************/
#ifndef MyRS232_H
#define MyRS232_H

#include <stdio.h>
#include <string.h>
#include <Windows.h>

class MyRS232
{
public:
    MyRS232();
    //methods
    int OpenComport(int comport_number);
    int PollComport(int comport_number, char *buf, int size);
    int SendBuf(int comport_number, char *buf, int size);
    void CloseComport(int comport_number);
    void Reset(int comport_number);
    void flush(int comport_number);
};

#endif // RS232_H
