/**************************************************************************
* psc1_rs232_mutex.h
*
* Routines to handle the RS232 mutex
* (dialog between main window and RS232 port)
**************************************************************************/
#ifndef psc1_rs232_mutex_h
#define psc1_rs232_mutex_h

int Create_RS232Mutex();
int Delete_RS232Mutex();
int Read_RS232Mutex(bool &RS232_is_busy);
int Write_RS232Mutex(bool RS232_is_busy);

#endif
