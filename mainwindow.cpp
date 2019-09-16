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

	if(!loadSettings())
	{
		Servers << "0.pool.ntp.org"
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
				<< "time.windows.com";

		iCurrentServer =0;
	}
//.............................................. init sync timer
    tmrSync = new QTimer;
    connect(tmrSync, SIGNAL(timeout()), SLOT(SyncTimer()));
	tmrSync->start(11111);
	SyncTimer(); // startup sync
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

		if(bSkipSetTimer)
		{
			bSkipSetTimer =false;

		return;
		}

		// UTC now
		qint64 qiUTCNowSec =QDateTime::currentDateTimeUtc().addMSecs(qwDiffTime).toSecsSinceEpoch();
		qint64 qiTimerValue =(ui->txtHH->text().toInt() * 3600 + ui->txtMM->text().toInt() * 60 + ui->txtSS->text().toInt());

		if(qiTimerValue <= 0)
			ui->cmdTimerOn->setChecked(false);
		// UTC now secs + timer secs -> UTC time sec
		else
			qiTimerTime =(qiUTCNowSec + qiTimerValue);
	}
	else
	{
		bTimerOn =false;
		qiTimerTime =0;
		ui->cmdTimerOn->setText("Start");
	}
}
//.....................................................
void MainWindow::displayTimerValue(qint64 qiTimerValue)
{
	if(qiTimerValue > (3600 - 1))
		ui->txtHH->setText(QString::number(qiTimerValue / 3600));
	else
		ui->txtHH->setText("0");

	if(qiTimerValue > (60 - 1))
		ui->txtMM->setText(QString::number(qiTimerValue / 60 % 60));
	else
		ui->txtMM->setText("0");

	if(qiTimerValue > 0)
		ui->txtSS->setText(QString::number(qiTimerValue % 60));
	else
		ui->txtSS->setText("0");
}
//----------------------------------------------------------------
void MainWindow::MainTimer()
{
QDateTime UTCNow =QDateTime::currentDateTimeUtc().addMSecs(qwDiffTime);

	ui->lblTime->setText(UTCNow.toString("hh:mm:ss")); // .zzz - millisecond

	if(bTimerOn && qiTimerTime > 0)
	{
	qint64 qiTimerValue =(qiTimerTime - UTCNow.toSecsSinceEpoch());

		displayTimerValue(qiTimerValue);

		if(qiTimerValue <= 0)
		{
			ui->cmdTimerOn->setChecked(false);

			if(ui->cmdBeepOn->isChecked())
				new CThread((THREAD)beep); // beep() is blocked function, then create new thread for beep

			//emit alarm(); // reserved
		}
	}
}
//..............................
void MainWindow::SyncTimer()
{
QList<QHostAddress> host_addr;

	if(!Servers.count() || iCurrentServer <= 0)
		return;

	host_addr =QHostInfo::fromName(Servers.at(iCurrentServer)).addresses();

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
	else
		saveSettings();
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
//-------------------------------------------------------------------------------------- init file magic
void MainWindow::saveSettings()
{
QJsonObject settings;
QJsonObject instance;
QJsonArray instances =loadJsonFile(qApp->applicationDirPath() + "/settings.ini")["instances"].toArray();

// common settings ...........................
	settings["servers"] =QJsonArray::fromStringList(Servers);

// instance settings .........................
	instance["CurrentServer"] =iCurrentServer;
	instance["TopMost"] =ui->cmdTop->isChecked();
	instance["BeepOn"] =ui->cmdBeepOn->isChecked();
	instance["TimerOn"] =ui->cmdTimerOn->isChecked();

	qint64 qiTimerValue;

	if(ui->cmdTimerOn->isChecked())
		qiTimerValue =qiTimerTime;
	else
		qiTimerValue =(ui->txtHH->text().toInt() * 3600 + ui->txtMM->text().toInt() * 60 + ui->txtSS->text().toInt());

	instance["TimerValue"] =qiTimerValue;

	instances.append(instance);
	settings["instances"] =instances;

	saveJsonFile(qApp->applicationDirPath() + "/settings.ini", QJsonDocument(settings));
}
//............................................................
bool MainWindow::loadSettings()
{
QJsonObject settings(loadJsonFile(qApp->applicationDirPath() + "/settings.ini"));

	if(!settings.isEmpty())
	{
		// servers list
		foreach(const QJsonValue &value, settings["servers"].toArray())
			Servers << value.toString();

		iCurrentServer =0;

	QJsonArray instances =settings["instances"].toArray();
	QJsonObject instance =instances.takeAt(0).toObject(); // delete settings of the currently running instance from instances array

// restore instance settings
		if(!instance["CurrentServer"].isUndefined())
			iCurrentServer =instance["CurrentServer"].toInt();

		if(!instance["TopMost"].isUndefined())
			ui->cmdTop->setChecked(instance["TopMost"].toBool());

		if(!instance["BeepOn"].isUndefined())
			ui->cmdBeepOn->setChecked(instance["BeepOn"].toBool());

		if(!instance["TimerOn"].isUndefined() && !instance["TimerValue"].isUndefined())
		{
			if(instance["TimerOn"].toBool()) // user timer running
			{
				bSkipSetTimer =true;
				qiTimerTime =instance["TimerValue"].toDouble();
			}
			else
				displayTimerValue(instance["TimerValue"].toDouble());

			ui->cmdTimerOn->setChecked(instance["TimerOn"].toBool());
		}

		settings["instances"] =instances; // new settings state
		saveJsonFile(qApp->applicationDirPath() + "/settings.ini", QJsonDocument(settings));

	return(true);
	}

return(false);
}
//...............................................................
bool MainWindow::saveJsonFile(const QString &FileName, const QJsonDocument &JsonDoc)
{
QFile file(FileName);

	if(file.open(QFile::WriteOnly))
	{
		file.write(JsonDoc.toJson(QJsonDocument::Indented));
		file.close();

	return(true);
	}

return(false);
}
//..............................................................
QJsonObject MainWindow::loadJsonFile(const QString &FileName)
{
QJsonObject JsonObj;
QFile file(FileName);

	if(file.open(QFile::ReadOnly))
	{
		JsonObj =QJsonDocument::fromJson(file.readAll()).object();
		file.close();
	}

return(JsonObj);
}
