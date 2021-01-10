/*******************************************************
 * License GPLv3 2007
 * UTime 2019 (c) JML <johnmclaren@tuta.io>
*******************************************************/
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QHostAddress>
#include <QHostInfo>
#include <QListWidget>
#include <QTimer>
#include <windows.h>
#include <QDebug>
#include "qntp/NtpClient.h"
#include "qntp/NtpReply.h"
#include <QKeyEvent>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDir>

#include "beep.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = nullptr);
	~MainWindow();

private:
	Ui::MainWindow *ui;
	QSystemTrayIcon *trayIcon;
	const QString WorkFolder =QDir::homePath() + "/Documents/UTime";

	NtpClient	*ntp;
	QTimer      *tmrMain; // seconds count
	QTimer      *tmrSync; // synchronization request to NTP server
	QStringList Servers;

	qint64 qwDiffTime =0;

	bool bTimerOn =false;
	qint64 qiTimerTime =0;
	bool bForceClose =false;
	bool bSkipSetTimer =false;
	int iCurrentServer =(-1);
	BEEP beep {1000, 1000, 1, this};

	void displayTimerValue(qint64 qiTimerValue);
	void saveSettings();
	bool loadSettings();
	bool saveJsonFile(const QString &FileName, const QJsonDocument &JsonDoc);
	QJsonObject loadJsonFile(const QString &FileName);

private slots:
	void NTPReplyReceived(const QHostAddress &address, quint16 port, const NtpReply &reply);
	void MainTimer();
	void SyncTimer();
	void iconActivated(QSystemTrayIcon::ActivationReason reason);
	void closeAction();
	void syncSec(); // seconds count synchronization

    void on_cmdTop_toggled(bool checked);
	void on_cmdTimerOn_toggled(bool checked);

protected:
	bool eventFilter(QObject *object, QEvent *event);
	void closeEvent(QCloseEvent * event);

signals:
	void alarm();
};

#endif // MAINWINDOW_H
