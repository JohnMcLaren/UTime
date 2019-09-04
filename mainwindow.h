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

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();

private slots:
	void NTPReplyReceived(const QHostAddress &address, quint16 port, const NtpReply &reply);

	void on_cmdGetTime_clicked();

    void MainTimer();
    void SyncTimer();

    void on_cmdTop_toggled(bool checked);

	void on_cmdTimerOn_toggled(bool checked);

private:
	Ui::MainWindow *ui;
	QSystemTrayIcon *trayIcon;

	NtpClient	*ntp;
    QTimer      *tmrMain;
    QTimer      *tmrSync;

    qint64   qwDiffTime;

	bool bTimerOn =false;
	qint64 qiTimerValue =0;

protected:
	bool eventFilter(QObject *object, QEvent *event);
	void closeEvent(QCloseEvent * event);

private slots:

	static void beep();

	void on_cmdBeepOn_toggled(bool checked);
	void iconActivated(QSystemTrayIcon::ActivationReason reason);

signals:

	void alarm();
};

#endif // MAINWINDOW_H
