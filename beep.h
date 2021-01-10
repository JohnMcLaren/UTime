/*******************************************************
 * License GPLv3 2007
 * UTime 2019 (c) JML <johnmclaren@tuta.io>
*******************************************************/
#ifndef BEEP_H
#define BEEP_H

#include <QObject>
#include <QVector>
#include <QtMath>
#include <QBuffer>
#include <QAudioOutput>
#include <QTimer>
#include <QEventLoop>
#include <QDebug>

#define SAMPLE_RATE 44100 // enough

// Frequency - Hz, Duration - mS, Volume - 0 to 1.0
class BEEP : public QObject
{
	Q_OBJECT

public:
	BEEP(QObject *parent = nullptr);
	BEEP(const int iFreq, const int iDuration, const qreal fVolume =1.0, QObject *parent = nullptr);
	~BEEP();

	void play(const int iFreq, const int iDuration, const qreal fVolume =1.0, const bool bWait =true);
	void play();

private:
	QVector<qint16> buff;
	QBuffer *input;
	QAudioFormat format;
	QAudioOutput *audio;

	void init();
	void makeBeep(const int iFreq, const int iDuration, const qreal fVolume);
	void waitFor(const QObject *pObject, const char *signal, int timeout =0);

public slots:

signals:
};

#endif // BEEP_H
