/* This file is part of QNtp, a library that implements NTP protocol.
 *
 * Copyright (C) 2011 Alexander Fokin <apfokin@gmail.com>
 *
 * QNtp is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * QNtp is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License 
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with QNtp. If not, see <http://www.gnu.org/licenses/>. */
#include "NtpClient.h"
#include <QUdpSocket>
#include <QHostAddress>
#include "NtpPacket.h"
#include "NtpReply.h"
#include "NtpReply_p.h"
//----------------------------------------------------------------------------------
NtpClient::NtpClient(QObject *parent): QObject(parent)
{
	init();
}

NtpClient::NtpClient(const QHostAddress &bindAddress, quint16 bindPort, QObject *parent): QObject(parent)
{
	init(bindAddress, bindPort);
}
//----------------------------------------------------------------------------------
bool NtpClient::init(const QHostAddress &bindAddress, quint16 bindPort)
{
	mSocket = new QUdpSocket(this);

	if(mSocket->bind(bindAddress, bindPort))// , QUdpSocket::ReuseAddressHint|QUdpSocket::ShareAddress)
	{
		connect(mSocket, SIGNAL(readyRead()), this, SLOT(readPendingDatagrams()));

	return(true);
	}
	else
	{
		qDebug() << "Error bind socket: " << mSocket->errorString();
		bErrorState =true;
	}

return(false);
}
//----------------------------------------------------------------------------------
bool NtpClient::init()
{
	mSocket = new QUdpSocket(this);
	connect(mSocket, SIGNAL(readyRead()), this, SLOT(readPendingDatagrams()));
	qDebug() << "NtpClient init";

return(true);
}

NtpClient::~NtpClient()
{
	return;
}
//------------------------------------------------------------------------
bool NtpClient::IsPortFree(quint16 port)
{
QUdpSocket udp;

	if(udp.bind(QHostAddress::LocalHost, port))
	{

	return(true);
	}

return(false);
}
//-----------------------------------------------------------------------------------------
bool NtpClient::sendRequest(const QHostAddress &address, quint16 port)
{
	if(!bErrorState && mSocket->state() != QAbstractSocket::BoundState)
	{
		qDebug() << "NtpClient: Socket error - not bound state";

	return false;
	}

  /* Initialize the NTP packet. */
NtpPacket packet;

	memset(&packet, 0, sizeof(packet));
	packet.flags.mode =ClientMode;
	packet.flags.versionNumber =4;
	packet.transmitTimestamp =NtpTimestamp::fromDateTime(QDateTime::currentDateTimeUtc());

	/* Send it. */
	if(mSocket->writeDatagram(reinterpret_cast<const char *>(&packet), sizeof(packet), address, port) < 0)
	{
		qDebug() << "NtpClient: " << mSocket->errorString();

	return false;
	}

return true;
}
//-----------------------------------------------------------------------------------------
void NtpClient::readPendingDatagrams()
{
	while (mSocket->hasPendingDatagrams())
	{
	NtpFullPacket packet;

		memset(&packet, 0, sizeof(packet));

	QHostAddress address;
	quint16 port;

		if(mSocket->readDatagram(reinterpret_cast<char *>(&packet), sizeof(packet), &address, &port) < sizeof(NtpPacket))
			continue;

	QDateTime now =QDateTime::currentDateTime();

		/* Prepare reply. */
	NtpReplyPrivate *replyPrivate = new NtpReplyPrivate();

		replyPrivate->packet =packet;
		replyPrivate->destinationTime =now;

	NtpReply reply(replyPrivate);

		/* Notify. */
		Q_EMIT replyReceived(address, port, reply);
	}
}
//-------------------------------------------------------------------------------------------
/******************
netstat -aon
http://time.in.ua/ntp.html
https://habrahabr.ru/post/111239/

time.windows.com
time.nist.gov
ntp.mobatime.ru
ntp1.stratum2.ru
ntp2.stratum2.ru
ntp3.stratum2.ru
ntp4.stratum2.ru
ntp5.stratum2.ru
ntp1.stratum1.ru
ntp3.stratum1.ru
ntp4.stratum1.ru
ntp5.stratum1.ru
0.pool.ntp.org
1.pool.ntp.org
2.pool.ntp.org
3.pool.ntp.org
*******************/
