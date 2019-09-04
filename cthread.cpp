/*******************************************************
 * License GPLv3 2007
 * UTime 2019 (c) JML <johnmclaren@tuta.io>
*******************************************************/
#include "cthread.h"
//--------------------------------------------------------------------- Thread with args
CThread::CThread(THREAD_ARG startaddress, void *args):
	threadroutine_arg(startaddress), threadargs(args)
{
	start();
}
//.................................................... Thread without args
CThread::CThread(THREAD startaddress):
	threadroutine(startaddress), threadroutine_arg(nullptr)
{
	start();
}
//---------------------------------------------------------------------
CThread::~CThread()
{
	quit();
	wait();
}
//---------------------------------------------------------------------
void CThread::run()
{
	if(threadroutine_arg)
    {
		//qDebug("Start thread [%X]", currentThreadId());
		threadroutine_arg(threadargs);
    }
	else
	if(threadroutine)
		threadroutine();

	//qDebug("Thread [%X] ended.", currentThreadId());

	emit signalThreadFinished(threadargs);
}
//---------------------------------------------------------------------
