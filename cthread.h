/*******************************************************
 * License GPLv3 2007
 * UTime 2019 (c) JML <johnmclaren@tuta.io>
********************************************************
 * modified 02.09.2019
*******************************************************/

#ifndef THREAD_H
#define THREAD_H

#include <QObject>
#include <QThread>
//-----------------------------------------------------------------------------
typedef	void (*THREAD_ARG)(void *args);
typedef	void (*THREAD)();
//----------------------------------------------------------------------------
class CThread : public QThread
{
	Q_OBJECT

public:
	CThread(THREAD_ARG startaddress, void *args);
	CThread(THREAD startaddress);
	~CThread();

private:
	THREAD_ARG threadroutine_arg;
	THREAD threadroutine;
	void *threadargs;

protected:
	void run();

signals:
	void signalThreadFinished(void *pArgs);
};
//............................................................................

//----------------------------------------------------------------------------
#endif // THREAD_H
