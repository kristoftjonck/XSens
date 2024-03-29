/*	Copyright (c) 2003-2017 Xsens Technologies B.V. or subsidiaries worldwide.
	All rights reserved.

	Redistribution and use in source and binary forms, with or without modification,
	are permitted provided that the following conditions are met:

	1.	Redistributions of source code must retain the above copyright notice,
		this list of conditions and the following disclaimer.

	2.	Redistributions in binary form must reproduce the above copyright notice,
		this list of conditions and the following disclaimer in the documentation
		and/or other materials provided with the distribution.

	3.	Neither the names of the copyright holders nor the names of their contributors
		may be used to endorse or promote products derived from this software without
		specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
	EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
	MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
	THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
	SPECIAL, EXEMPLARY OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
	OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
	HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR
	TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
	SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MYXDA_H
#define MYXDA_H

#include <xsensdeviceapi.h> // The Xsens device API header

#include <QObject>
#include <QMetaType>
#include <QMutex>
#include <QMutexLocker>

#include <stdexcept>
#include <set>
#include <map>
#include "mat.h"
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QDateTime>

#pragma comment(lib, "libmat.lib")
#pragma comment(lib, "libmx.lib")
//#pragma comment(lib, "libmatlb.lib")
//#pragma comment(lib, "libmmfile.lib")

Q_DECLARE_METATYPE(XsPortInfo)
Q_DECLARE_METATYPE(XsConnectivityState)
Q_DECLARE_METATYPE(XsResultValue)
Q_DECLARE_METATYPE(XsDeviceState)
Q_DECLARE_METATYPE(XsDeviceId)
Q_DECLARE_METATYPE(XsDataPacket)
Q_DECLARE_METATYPE(XsInfoRequest)

class MetaTypeInitializer {
public:
	MetaTypeInitializer() {
		// For implementing the communication interface from XDA to the GUI
		// the classes of XDA are registered so that they can be passed as signal
		// parameters.
		qRegisterMetaType<XsPortInfo>("XsPortInfo");
		qRegisterMetaType<XsConnectivityState>("XsConnectivityState");
		qRegisterMetaType<XsResultValue>("XsResultValue");
		qRegisterMetaType<XsDeviceState>("XsDeviceState");
		qRegisterMetaType<XsDeviceId>("XsDeviceId");		
		qRegisterMetaType<XsDataPacket>("XsDataPacket");
		qRegisterMetaType<XsInfoRequest>("XsInfoRequest");
	}
};

class MyXda : public QObject
{
	Q_OBJECT
public:
	MyXda();
	virtual ~MyXda();
	XsDevice* getDevice(XsDeviceId) const;

public slots:
	void reset();
    void openPort(XsPortInfo);

    /*!
      \fn void convert(XsString destFolder,XsString newFolder,QHash<QString, QString> * map)

      Converts data loaded from the loaded file into a csv.
      The data will be saved in a new folder with the name \a newFolder.
      This folder will be made in the folder \a destFolder. A hashmap \a map must be passed
      with the names of the MTw's.
    */
    void convert(XsString destFolder,XsString newFolder,QHash<QString, QString> *);
    /*!
      \fn bool loadFile(XsString file)

     Loads the MTB file \a file into the XDA.

      Returns \c true if the file successfully loaded
    */
    bool loadFile(XsString file);
    /*!
      \fn bool checkUpdateRate(int rate)

      Checks if the sampling rate \a rate from all the devices is correct.

      Returns \c true if the sample rates are all correct.
    */
    bool checkUpdateRate(int rate);

signals:
	void wirelessMasterDetected(XsPortInfo);
	void dockedMtwDetected(XsPortInfo);
	void mtwUndocked(XsPortInfo);	
	void openPortSuccessful(XsPortInfo);
	void openPortFailed(XsPortInfo);

protected slots:
	void scanPorts();

private:
	typedef std::set<XsPortInfo> DetectedDevices;
	typedef std::map<XsPortInfo, XsDevicePtr> OpenDevices;

	XsControl* m_control;
	DetectedDevices m_detectedDevices;
};

class MyWirelessMasterCallback : public QObject, public XsCallback // NOTE: always put QObject first!
{
	Q_OBJECT
public:
	std::set<XsDeviceId> getConnectedMTws() const;

signals:
	void deviceError(XsDeviceId, XsResultValue);
	void measurementStarted(XsDeviceId);
	void measurementStopped(XsDeviceId);
	void waitingForRecordingStart(XsDeviceId);
	void recordingStarted(XsDeviceId);
	void mtwDisconnected(XsDeviceId);
	void mtwRejected(XsDeviceId);
	void mtwPluggedIn(XsDeviceId);
	void mtwWireless(XsDeviceId);
	void mtwFile(XsDeviceId);
	void mtwUnknown(XsDeviceId);
	void mtwError(XsDeviceId);
	void progressUpdate(XsDeviceId, int, int, QString);
	
protected:
	virtual void onConnectivityChanged(XsDevicePtr, XsConnectivityState);
	virtual void onDeviceStateChanged(XsDevicePtr, XsDeviceState, XsDeviceState);
	virtual void onError(XsDevicePtr, XsResultValue);
	virtual void onProgressUpdated(XsDevice* dev, int current, int total, const XsString *identifier);

private:
	mutable QMutex m_mutex;
	std::set<XsDeviceId> m_connectedMTws;
};

class MyMtwCallback : public QObject, public XsCallback // NOTE: always put QObject first!
{
	Q_OBJECT
signals:
	void dataAvailable(XsDeviceId, XsDataPacket);
	void batteryLevelChanged(XsDeviceId, int);
protected:
	virtual void onLiveDataAvailable(XsDevicePtr, const XsDataPacket* packet);
	virtual void onInfoResponse(XsDevice* dev, XsInfoRequest request);
};

#endif
