/*******************************************************
 * License GPLv3 2007
 * UTime 2019 (c) JML <johnmclaren@tuta.io>
*******************************************************/
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "cthread.h"
//------------------------------------------------------------------------------------------------------------
MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	setWindowTitle("UTime v1.1a");
	setWindowFlags(Qt::CustomizeWindowHint|Qt::WindowTitleHint|Qt::WindowCloseButtonHint |Qt::WindowSystemMenuHint);
	setFixedSize(size());
//.......................................................... tray icon init magic
	trayIcon = new QSystemTrayIcon(this);
	trayIcon->setIcon(this->style()->standardIcon(QStyle::SP_ComputerIcon));
	trayIcon->setToolTip(this->windowTitle());

QMenu *menu = new QMenu(this);
QAction *actShow = new QAction(trUtf8("Show"), this);
QAction *actExit = new QAction(trUtf8("Exit"), this);

	connect(actShow, SIGNAL(triggered()), this, SLOT(show()));
	connect(actExit, SIGNAL(triggered()), this, SLOT(closeAction()));
	menu->addAction(actShow);
	menu->addSeparator();
	menu->addAction(actExit);
	trayIcon->setContextMenu(menu);
	trayIcon->show();

	connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));
//.......................................................... init main timer
	tmrMain = new QTimer;
	tmrMain->setTimerType(Qt::PreciseTimer);
	connect(tmrMain, SIGNAL(timeout()), SLOT(MainTimer()));
	tmrMain->start(1000);

	ui->cmdBeepOn->setEnabled(true);
	ui->cmdTimerOn->setEnabled(true);
	ui->cmdTop->setChecked(true);
	ui->cmdBeepOn->setChecked(true);

	ui->txtHH->installEventFilter(this);
	ui->txtMM->installEventFilter(this);
	ui->txtSS->installEventFilter(this);

	ntp = new NtpClient(QHostAddress::Any, 123);

	if(ntp->bErrorState || !ntp->IsNtpPortFree())
	{
		QMessageBox::warning(this, "Error", "Ntp port [123] busy by system or another program.");
		ui->lblSync->setText("*");
		ui->lblSync->setStyleSheet("Color: red;");

	return;
	}
//................................................................................................
    connect(ntp, SIGNAL(replyReceived(const QHostAddress, quint16, const NtpReply)), this, SLOT(NTPReplyReceived(const QHostAddress, quint16, const NtpReply)));

	ui->lstTimeServers->addItems(QStringList() << "0.pool.ntp.org"
												<< "1.pool.ntp.org"
												<< "2.pool.ntp.org"
												<< "3.pool.ntp.org"
												<< "ntp1.stratum1.ru"
                                                << "ntp3.stratum1.ru"
                                                << "ntp4.stratum1.ru"
                                                << "ntp5.stratum1.ru"
                                                << "ntp1.stratum2.ru"
                                                << "ntp2.stratum2.ru"
                                                << "ntp3.stratum2.ru"
                                                << "ntp4.stratum2.ru"
                                                << "ntp5.stratum2.ru"

                                                << "ntp.mobatime.ru"
                                                << "time.nist.gov"
                                                << "time.windows.com");
    ui->lstTimeServers->setCurrentRow(0);
//.............................................. init sync timer
    tmrSync = new QTimer;
    connect(tmrSync, SIGNAL(timeout()), SLOT(SyncTimer()));
	tmrSync->start(11111);
}
//---------------------------------------------------------------------------------------------------------------
MainWindow::~MainWindow()
{
	delete ui;
}
//--------------------------------------------------------------------------------------------------------------- ntp magic
void MainWindow::NTPReplyReceived(const QHostAddress &address, quint16 port, const NtpReply &reply)
{
    qwDiffTime =reply.localClockOffset();

int iRemainMSecServer =(1000 - reply.ServerTime().time().msec());
int iRemainMSecTimer =tmrMain->remainingTime();

	if(std::abs(iRemainMSecServer - iRemainMSecTimer) > 20)
	{
		QTimer::singleShot(iRemainMSecServer, Qt::PreciseTimer, this, SLOT(syncSec())); // seconds re-synchronization
		tmrMain->stop();
	}

	ui->lblSync->setText("");

	//qDebug("RemainMSecServer: %u RemainMSecTimer: %u", iRemainMSecServer, iRemainMSecTimer);
}
//............................
void MainWindow::syncSec()
{
	tmrMain->start();
	MainTimer(); // second event

	//qDebug() << "-- Seconds synchronized --";
}
//------------------------------------------------------------------------------------------------------------- countdown timer magic

void MainWindow::on_cmdTimerOn_toggled(bool checked)
{
	if(checked)
	{
		bTimerOn =true;
		ui->cmdTimerOn->setText("Stop");

		qiTimerValue =(ui->txtHH->text().toInt() * 3600 + ui->txtMM->text().toInt() * 60 + ui->txtSS->text().toInt());

		if(qiTimerValue <= 0)
			ui->cmdTimerOn->setChecked(false);
	}
	else
	{
		bTimerOn =false;
		ui->cmdTimerOn->setText("Start");
	}
}
//----------------------------------------------------------------
void MainWindow::MainTimer()
{
    ui->lblTime->setText(QDateTime::currentDateTimeUtc().addMSecs(qwDiffTime).toString("hh:mm:ss")); // zzz - microsecond

	if(bTimerOn && qiTimerValue > 0)
	{
		qiTimerValue--;

		if(qiTimerValue > (3600 - 1))
			ui->txtHH->setText(QString::number(qiTimerValue / 3600));
		else
			ui->txtHH->setText("0");

		if(qiTimerValue > (60 - 1))
			ui->txtMM->setText(QString::number(qiTimerValue / 60 % 60));
		else
			ui->txtMM->setText("0");

		ui->txtSS->setText(QString::number(qiTimerValue % 60));

		if(qiTimerValue == 0)
		{
			ui->cmdTimerOn->setChecked(false);

			//emit alarm();
			if(ui->cmdBeepOn->isChecked())
				new CThread((THREAD)beep); // beep() is blocked function, then create new thread for beep
		}
	}
}
//..............................
void MainWindow::SyncTimer()
{
QList<QHostAddress> host_addr;

	if(!ui->lstTimeServers->count())
		return;

	host_addr =QHostInfo::fromName(ui->lstTimeServers->currentItem()->text()).addresses();

	if(!host_addr.size())
		return;

	ui->lblSync->setText("*");

	if(!ntp->sendRequest(host_addr[0], 123))
		qDebug() << "UTime: Send error";
}
//-------------------------------------------------------------------------------------------------------------- on_top magic
void MainWindow::on_cmdTop_toggled(bool checked)
{
    if(checked)
        SetWindowPos((HWND)winId(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    else
        SetWindowPos((HWND)winId(), HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
}
//----------------------------------------------------- beep magic
void MainWindow::beep()
{
	Beep(3100, 1000); // blocked function
}
//----------------------------------------------------------------------------------------- keys magic
bool MainWindow::eventFilter(QObject *object, QEvent *event)
{
	if (object == ui->txtHH || object == ui->txtMM || object == ui->txtSS)
	{
		if(event->type() == QEvent::FocusIn || event->type() == QEvent::MouseButtonPress)
		{
			((QLineEdit*)object)->selectAll();

		return(true);
		}

		if(event->type() == QEvent::KeyPress)
		{
		QKeyEvent *KeyEvent =(QKeyEvent*)event;

			if(KeyEvent->key() == Qt::Key_Left)
			{
				if(object == ui->txtHH)
					ui->txtSS->setFocus();
				else
					this->focusNextPrevChild(false);
			}
			else
			if(KeyEvent->key() == Qt::Key_Right)
			{
				if(object == ui->txtSS)
					ui->txtHH->setFocus();
				else
					this->focusNextPrevChild(true);
			}
			else
			if(KeyEvent->key() == Qt::Key_Return || KeyEvent->key() == Qt::Key_Enter)
				ui->cmdTimerOn->setChecked(!ui->cmdTimerOn->isChecked());
		}
	}

return(QObject::eventFilter(object, event));
}
//---------------------------------------------------------------------------------------------------- tray magic
void MainWindow::closeEvent(QCloseEvent *event)
{
	if(this->isVisible() && !bForceClose)
	{
		event->ignore();
		this->hide();
	}
}
//.............................................
void MainWindow::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
	if(reason == QSystemTrayIcon::Trigger || reason == QSystemTrayIcon::DoubleClick)
		if(this->isVisible())
			this->hide();
		else
			this->show();
}
//............................................
void MainWindow::closeAction()
{
	bForceClose =true;

	close();
}
