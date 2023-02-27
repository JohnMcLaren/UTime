/*******************************************************
 * License GPLv3 2007
 * UTime 2019 (c) JML <johnmclaren@tuta.io>
*******************************************************/
#include "mainwindow.h"
#include "ui_mainwindow.h"
//-------------------------------------------------------------------------------------------------------------
MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	setWindowTitle(QString(APP_NAME) + " v" + APP_VERSION);
	setWindowFlags(Qt::CustomizeWindowHint|Qt::WindowTitleHint|Qt::WindowCloseButtonHint|Qt::WindowSystemMenuHint|Qt::Dialog|Qt::MSWindowsFixedSizeDialogHint);
	setFixedSize(size());

	SETTINGS_PATH =QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);

#if defined(Q_OS_LINUX)

QProcessEnvironment env = QProcessEnvironment::systemEnvironment();

		if(env.value("SUDO_USER").isEmpty() && env.value("USER") != "root") // not su & not root
			QMessageBox::warning(this, "Attention", "This program requires superuser privileges to work with the NTP port.");
		else
		if(!env.value("SUDO_USER").isEmpty()) // su but not root
			SETTINGS_PATH.replace("/root", "/home/" + env.value("SUDO_USER"));
		else // root
			;
#endif
//.......................................................... tray icon init magic
	trayIcon = new QSystemTrayIcon(this);
	//trayIcon->setIcon(this->style()->standardIcon(QStyle::SP_ComputerIcon));
	trayIcon->setIcon(QIcon(":/img/icon-tray.png"));
	trayIcon->setToolTip(this->windowTitle());

QMenu *menu = new QMenu(this);
QMenu *submenu = new QMenu("Servers", this);

	menu->addAction("Show", this, SLOT(show()));
	menu->addSeparator();
	menu->addMenu(submenu); // Servers list
	menu->addSeparator();
	menu->addAction("Exit", this, SLOT(closeAction()));
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

	ntp = new NtpClient();

	if(ntp->bErrorState || !ntp->IsPortFree())
	{
		QMessageBox::warning(this, "Error", "Ntp port [123] busy by system or another program.");
		ui->lblSync->setText("*");
		ui->lblSync->setStyleSheet("Color: red;");

	return; // !!! stop init
	}
//................................................................................................
    connect(ntp, SIGNAL(replyReceived(const QHostAddress, quint16, const NtpReply)), this, SLOT(NTPReplyReceived(const QHostAddress, quint16, const NtpReply)));
//................................ Settings
	if(!QDir(SETTINGS_PATH).exists())
		QDir().mkdir(SETTINGS_PATH);

	if(!loadSettings())
	{
		Servers << "0.pool.ntp.org"
				<< "1.pool.ntp.org"
				<< "2.pool.ntp.org"
				<< "3.pool.ntp.org"
				<< "ntp0.ntp-servers.net"
				<< "ntp1.ntp-servers.net"
				<< "ntp2.ntp-servers.net"
				<< "ntp3.ntp-servers.net"
				<< "ntp4.ntp-servers.net"
				<< "ntp5.ntp-servers.net"
				<< "ntp6.ntp-servers.net"
				<< "ntp7.ntp-servers.net"

				<< "ntp.mobatime.ru"
				<< "time.nist.gov"
				<< "time.windows.com";

		iCurrentServer =0;
	}

	// Add Servers to menu
QActionGroup *group = new QActionGroup(this); // only one server can be selected

	for(int c =0; c < Servers.size(); c++)
	{
	QAction *action = new QAction(Servers.at(c));

		group->addAction(action);
		action->setCheckable(true);

		if(c == iCurrentServer)
			action->setChecked(true);

		connect(action, &QAction::triggered, [this, c]() { // menu index action lambda handler
			iCurrentServer =c;
		});
	}

	submenu->addActions(group->actions());
//.............................................. init sync timer
    tmrSync = new QTimer;
    connect(tmrSync, SIGNAL(timeout()), SLOT(SyncTimer()));
	tmrSync->start(11111);
	SyncTimer(); // startup sync
}
//---------------------------------------------------------------------------------------------------------------
MainWindow::~MainWindow()
{
	delete tmrMain; // to avoid segmentation fault errors on Timer event
	delete tmrSync;

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
			{
				//emit alarm(); // reserved
				beep.play();
			}
		}
	}
}
//..............................
void MainWindow::SyncTimer()
{
QList<QHostAddress> host_addr;

	if(iCurrentServer >= Servers.size())
		return;

	host_addr =QHostInfo::fromName(Servers.at(iCurrentServer)).addresses();

	if(!host_addr.size())
		return;

	ui->lblSync->setText("*");

	if(!ntp->sendRequest(host_addr[0], 123))
		qDebug() << "UTime: Send error";
}
//-------------------------------------------------------------------------------------------------- on_top magic
void MainWindow::on_cmdTop_toggled(bool checked)
{
    if(checked)
		setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
    else
		setWindowFlags(windowFlags() ^ Qt::WindowStaysOnTopHint);

	show();
}
//----------------------------------------------------------------------------------------- keys magic
bool MainWindow::eventFilter(QObject *object, QEvent *event)
{
	if(object == ui->txtHH || object == ui->txtMM || object == ui->txtSS)
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
//--------------------------------------------------------------------------------------------------- tray magic
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

	delete trayIcon; // for Linux: The program will not be closed while there is a tray icon [Qt-BUG]
	close();
}
//-------------------------------------------------------------------------------------- init file magic
void MainWindow::saveSettings()
{
QJsonObject settings;
QJsonObject instance;
QJsonArray instances =loadJsonFile(SETTINGS_PATH + "/settings.ini")["instances"].toArray();

// common settings ...........................
	settings["servers"] =QJsonArray::fromStringList(Servers);

// instance settings .........................
	instance["CurrentServer"] =iCurrentServer;
	instance["TopMost"] =ui->cmdTop->isChecked();
	instance["BeepOn"] =ui->cmdBeepOn->isChecked();
	instance["TimerOn"] =ui->cmdTimerOn->isChecked();
	instance["Position"] =QJsonObject{{"x", window()->x()}, {"y", window()->y()}};

qint64 qiTimerValue;

	if(ui->cmdTimerOn->isChecked())
		qiTimerValue =qiTimerTime;
	else
		qiTimerValue =(ui->txtHH->text().toInt() * 3600 + ui->txtMM->text().toInt() * 60 + ui->txtSS->text().toInt());

	instance["TimerValue"] =qiTimerValue;

	instances.append(instance);
	settings["instances"] =instances;

	if(!iCurrentServer && !qiTimerValue) // don't save default settings
		return;

	saveJsonFile(SETTINGS_PATH + "/settings.ini", QJsonDocument(settings));
}
//............................................................
bool MainWindow::loadSettings()
{
	if(!QDir(SETTINGS_PATH).exists())
		return(false);

QJsonObject settings(loadJsonFile(SETTINGS_PATH + "/settings.ini"));

	if(!settings.isEmpty())
	{
		// servers list
		foreach(const QJsonValue &value, settings["servers"].toArray())
			Servers << value.toString();

		iCurrentServer =0;

	QJsonArray instances =settings["instances"].toArray();
	QJsonObject instance =instances.takeAt(0).toObject(); // delete settings of the currently running instance from instances array

// restore instance settings
		if(!instance["CurrentServer"].isNull())
			iCurrentServer =instance["CurrentServer"].toInt();

		if(!instance["TopMost"].isNull())
			ui->cmdTop->setChecked(instance["TopMost"].toBool());

		if(!instance["BeepOn"].isNull())
			ui->cmdBeepOn->setChecked(instance["BeepOn"].toBool());

		if(!instance["TimerOn"].isNull() && !instance["TimerValue"].isNull())
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

		if(!instance["Position"].isNull())
		{
		QJsonObject pos =instance["Position"].toObject();
		int x =pos["x"].toInt(), y =pos["y"].toInt();
		QScreen *screen =QGuiApplication::primaryScreen();

			if(x > 0 && x < screen->geometry().width() && y > 0 && y < screen->geometry().height())
				move(x, y); // move window to last saved position
		}

// new settings state
		settings["instances"] =instances;
		saveJsonFile(SETTINGS_PATH + "/settings.ini", QJsonDocument(settings));

		if(iCurrentServer < Servers.size())
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
/*******************************************************************************************************
 * Win10 virtual store - c:\Users\...\AppData\Local\VirtualStore\Program Files (x86)\UTime\

*******************************************************************************************************/
