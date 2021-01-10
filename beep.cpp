/*******************************************************
 * License GPLv3 2007
 * UTime 2019 (c) JML <johnmclaren@tuta.io>
*******************************************************/
#include "beep.h"

BEEP::BEEP(QObject *parent) : QObject(parent)
{
	init();
}

BEEP::BEEP(const int iFreq, const int iDuration, const qreal fVolume, QObject *parent)
{
	init();
	makeBeep(iFreq, iDuration, fVolume);
}

BEEP::~BEEP()
{
	delete input;
	delete audio;
}
//---------------------------------------------------------------------------------
void BEEP::init()
{
	// Make a QBuffer from our data array
	input = new QBuffer();

	// Set up the format, eg.
	format.setSampleRate(SAMPLE_RATE);
	format.setChannelCount(1);
	format.setSampleSize(16); // 8
	format.setCodec("audio/pcm");
	format.setByteOrder(QAudioFormat::LittleEndian);
	format.setSampleType(QAudioFormat::SignedInt);

	// Create an output with our premade QAudioFormat (See example in QAudioOutput)
	audio = new QAudioOutput(format);
}

void BEEP::makeBeep(const int iFreq, const int iDuration, const qreal fVolume)
{
double w =(2.0 * M_PI * iFreq); // Angular frequency

	if(input->isOpen())
		input->close();

	buff.resize((int)(iDuration / 1000.0) * SAMPLE_RATE);

	// data array
	for(int i=0; i < buff.size(); i++)
		buff[i] =(qint16)(0x7FFF * qSin(w * i / SAMPLE_RATE)); // float to 0...0xFFFF (UnSigned) diapazone or -32767 to +32767 (Signed)

	input->setData((char *)buff.data(), buff.size() * 2); // *2 >>> 16-bit =2 bytes
	audio->setVolume(fVolume); // work on SignedInt sample only

	input->open(QIODevice::ReadOnly);
}

void BEEP::play(const int iFreq, const int iDuration, const qreal fVolume, const bool bWait)
{
	if(bWait && (audio->state() == QAudio::ActiveState))
		waitFor(audio, SIGNAL(stateChanged(QAudio::State)));

	makeBeep(iFreq, iDuration, fVolume);
	play();
}

void BEEP::play()
{
	if(input->isOpen())
	{
		audio->start(input);
		input->reset(); // rewind to the beginning of the buffer
	}
	// playback starts on exit the function!
}
//--------------------------------------------------------
void BEEP::waitFor(const QObject *pObject, const char *signal, int timeout)
{
QEventLoop loop;

	if(timeout)
		QTimer::singleShot(timeout, &loop, SLOT(quit()));

	loop.connect(pObject, signal, &loop, SLOT(quit()), Qt::QueuedConnection);
	loop.exec();
}

