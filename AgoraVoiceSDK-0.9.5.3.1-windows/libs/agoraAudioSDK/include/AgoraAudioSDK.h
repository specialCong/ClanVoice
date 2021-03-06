//
//  AgoraAudioSDK.h
//  agorasdk
//
//  Created by sting feng on 2014-7-20.
//  Copyright (c) 2014 Agora Voice. All rights reserved.
//

#ifndef AGORA_AUDIO_SDK_H
#define AGORA_AUDIO_SDK_H

#include <stdio.h>

#if defined(_WIN32)
#ifndef snprintf
#define snprintf _snprintf
#endif
#define CALL_API __cdecl
#if defined(MEDIASDK_EXPORT)
#define AGORA_API __declspec(dllexport)
#else
#define AGORA_API __declspec(dllimport)
#endif

struct VideoCanvas {
    void* view;
    float top;
    float left;
    float bottom;
    float right;
    
    VideoCanvas()
        : view(NULL)
        , top(0.0)
        , left(0.0)
        , bottom(1.0)
        , right(1.0)
    {}
};

struct chat_engine_context_t {
    bool videoEnabled;
    VideoCanvas remote;
    VideoCanvas local;
};

#else
#define AGORA_API
#define CALL_API
#endif

typedef unsigned int uid_t;

#define MAX_AUDIO_DEVICE_NAME_LENGTH 128
#define MAX_AUDIO_DEVICE_GUID_LENGTH 128

class IAgoraPacketObserver;

class IAgoraAudioEventHandler
{
public:
	struct SpeakerInfo {
		uid_t uid;
		unsigned int volume; // [0,255]
	};
	struct SessionStat {
		unsigned int duration;
		unsigned int txBytes;
		unsigned int rxBytes;
		unsigned short txKBitRate;
		unsigned short rxKBitRate;
	};
    enum AUDIO_ENGINE_EVENT_CODE
    {
        AUDIO_ENGINE_RECORDING_ERROR = 0,
        AUDIO_ENGINE_PLAYOUT_ERROR = 1,
        AUDIO_ENGINE_RECORDING_WARNING = 2,
        AUDIO_ENGINE_PLAYOUT_WARNING = 3
    };
	enum AUDIO_DEVICE_STATE_TYPE {
		AUDIO_DEVICE_STATE_ACTIVE = 1,
		AUDIO_DEVICE_STATE_DISABLED = 2,
		AUDIO_DEVICE_STATE_NOT_PRESENT = 4,
		AUDIO_DEVICE_STATE_UNPLUGGED = 8
	};
	enum AUDIO_DEVICE_TYPE {
		UNKNOWN_AUDIO_DEVICE = -1,
		PLAYOUT_DEVICE = 0,
		RECORDING_DEVICE = 1
	};
	enum MEDIA_QUALITY {
		MEDIA_QUALITY_UNKNOWN = 0,
		MEDIA_QUALITY_EXCELLENT = 1,
		MEDIA_QUALITY_GOOD = 2,
		MEDIA_QUALITY_POOR = 3,
		MEDIA_QUALITY_BAD = 4,
		MEDIA_QUALITY_VBAD = 5,
		MEDIA_QUALITY_DOWN = 6,
	};
public:
	virtual void onLoadAudioEngineSuccess() {}
	virtual void onGetAudioSvrAddrSuccess() {}
	virtual void onJoinSuccess(const char* channel, uid_t uid, int elapsed) {}
	virtual void onRejoinSuccess(const char* channel, uid_t uid, int elapsed) {}
	virtual void onError(int rescode, const char* msg) {}
	virtual void onLogEvent(const char* msg) {}
	virtual void onAudioQuality(uid_t uid, int quality, unsigned short delay, unsigned short jitter, unsigned short lost, unsigned short lost2) {}
	virtual void onRecapStat(const char* recap_state, int length) {}
	virtual void onSpeakersReport(const SpeakerInfo* speakers, unsigned int speakerNumber, int mixVolume) {}
	virtual void onLeaveChannel(const SessionStat& stat) {}
	virtual void onUpdateSessionStats(const SessionStat& stat) {}
	virtual void onAudioEngineEvent(int evt) {}
	virtual void onAudioDeviceStateChanged(const char* deviceId, int deviceType, int deviceState) {}
	virtual void onNetworkQuality(int quality) {}
	virtual void onFirstVideoFrame(int elapsed) {}
	virtual void onUserJoined(uid_t uid, int elapsed) {}
	virtual void onUserOffline(uid_t uid) {}
	virtual void onUserMuteAudio(uid_t uid, bool muted) {}
	virtual void onUserMuteVideo(uid_t uid, bool muted) {}
	virtual void onLocalVideoStat(int sentBytes, int sentFrames, int sentQP, int sentRtt, int sentLoss) {}
	virtual void onRemoteVideoStat(uid_t uid, int frameCount, int delay, int receivedBytes) {}
};

class IAgoraAudioDeviceCollection
{
public:
	virtual int getCount() = 0;
	virtual int getDevice(int index, int& deviceIndex, char deviceName[MAX_AUDIO_DEVICE_NAME_LENGTH], char deviceId[MAX_AUDIO_DEVICE_GUID_LENGTH]) = 0;
	virtual int setDevice(const char deviceId[MAX_AUDIO_DEVICE_GUID_LENGTH]) = 0;
	virtual int setDevice(int deviceIndex) = 0;
	virtual void release() = 0;
};

class IAgoraAudioDeviceManager
{
public:
    virtual ~IAgoraAudioDeviceManager(){}
	virtual IAgoraAudioDeviceCollection* enumeratePlayoutDevices() = 0;
	virtual IAgoraAudioDeviceCollection* enumerateRecordingDevices() = 0;
	virtual int setPlayoutDevice(int deviceIndex) = 0;
	virtual int setPlayoutDevice(const char deviceId[MAX_AUDIO_DEVICE_GUID_LENGTH]) = 0;
	virtual int setRecordingDevice(int deviceIndex) = 0;
	virtual int setRecordingDevice(const char deviceId[MAX_AUDIO_DEVICE_GUID_LENGTH]) = 0;
	virtual int startSpeakerTest(const char* audioFileName) = 0;
	virtual int stopSpeakerTest() = 0;
	virtual int startMicrophoneTest(int reportInterval) = 0;
	virtual int stopMicrophoneTest() = 0;
};

class IAgoraAudio
{
public:
    virtual int joinChannel(const char* key, const char* channel, const char* info, uid_t uid) = 0;
    virtual int leave() = 0;
	virtual int initialize(const char* deviceId, const char* dataDir, const char* cacheDir) = 0;
	virtual int getProfile(char* buffer, size_t* length) = 0;
	virtual int setProfile(const char* profile, bool merge) = 0;
	virtual int setParameters(const char* parameters) = 0;
	virtual int getParameters(const char* parameters, char* buffer, size_t* length) = 0;
	virtual IAgoraAudioDeviceManager* getAudioDeviceManager() = 0;
	virtual const char* getSdkVersion() = 0;
	virtual int rate(const char* callId, int rating) = 0; // 0~10
	virtual int complain(const char* callId) = 0;
	virtual int startEchoTest(const char* key) = 0;
	virtual int stopEchoTest() = 0;
	virtual int enableServiceMode() = 0;
	virtual int disableServiceMode() = 0;
	virtual int enableNetworkTest(const char* key) = 0;
	virtual int disableNetworkTest() = 0;
	virtual int notifyNetworkChange(int networkType) = 0;
	virtual void setAppContext(void* ctx, size_t size) = 0;
	virtual void* getAppContext() = 0;
	virtual const char* getCallId() = 0;
	virtual int makeQualityReportUrl(const char* vendorKey, const char* channel, uid_t listenerUid, uid_t speakerUid, int format, char* buffer, size_t* length) = 0;
	virtual int registerObserver(IAgoraPacketObserver* observer) = 0;
	virtual void release() = 0;
protected:
	friend class AgoraAudioParameters;
	virtual int setBooleanParameter(const char* module, const char* name, bool value);
	virtual int setIntegerParameter(const char* module, const char* name, int value);
	virtual int setStringParameter(const char* module, const char* name, const char* value);
};

class AgoraAudioParameters
{
public:
	AgoraAudioParameters(IAgoraAudio* engine)
		:m_engine(engine){}
	int mute(bool mute)
	{
		return m_engine->setBooleanParameter("mediaSdk", "mute", mute);
	}
	// mute/unmute all peers. unmute will clear all muted peers specified mutePeer() interface
	int mutePeers(bool mute)
	{
		return m_engine->setBooleanParameter("mediaSdk", "mutePeers", mute);
	}
	int mutePeer(bool mute, uid_t uid)
	{
		char buffer[256];
		snprintf(buffer, sizeof(buffer)-1, "{\"mediaSdk\":{\"mutePeer\":{\"mute\":%s,\"peerUid\":%u}}}", mute?"true":"false", uid);
		buffer[sizeof(buffer)-1] = '\0';
		return m_engine->setParameters(buffer);
	}
	int muteLocalVideo(bool mute)
	{
		char buffer[256];
		const char* p = mute?"false":"true";
		snprintf(buffer, sizeof(buffer)-1, "{\"mediaSdk\":{\"enableLocalVideo\":%s}, \"audioEngine\":{\"video\":{\"enableLocal\": %s}}}", p, p);
		buffer[sizeof(buffer)-1] = '\0';
		return m_engine->setParameters(buffer);
	}
	int muteRemoteVideo(bool mute)
	{
		char buffer[256];
		const char* p = mute?"false":"true";
		snprintf(buffer, sizeof(buffer)-1, "{\"audioEngine\":{\"video\":{\"enableRemote\": %s}}}", p);
		buffer[sizeof(buffer)-1] = '\0';
		return m_engine->setParameters(buffer);
	}
	int enableSpeaker(bool enable)
	{
		return m_engine->setBooleanParameter("audioEngine", "speakerOn", enable);
	}
	int setSpeakerVolume(int volume) // [0,255]
	{
		return m_engine->setIntegerParameter("audioEngine", "speakerVolume", volume);
	}
	int setMicrophoneVolume(int volume) // [0,255]
	{
		return m_engine->setIntegerParameter("audioEngine", "micVolume", volume);
	}
	int setSpeakersReport(int interval, int smoothFactor) // in ms: <= 0: disable, > 0: enable, interval in ms
	{
		if (interval < 0)
			interval = 0;

		char buffer[256];
		snprintf(buffer, sizeof(buffer)-1, "{\"audioEngine\":{\"enableVolumeReport\":%d, \"volumeSmoothFactor\":%d}}", interval, smoothFactor);
		buffer[sizeof(buffer)-1] = '\0';
		return m_engine->setParameters(buffer);
	}
	int setLogFilter(unsigned int filter)
	{
		char buffer[256];
		snprintf(buffer, sizeof(buffer)-1, "{\"mediaSdk\":{\"logFilter\":%u}}", filter);
		buffer[sizeof(buffer)-1] = '\0';
		return m_engine->setProfile(buffer, true);
	}
private:
	IAgoraAudio* m_engine;
};


extern "C" AGORA_API IAgoraAudio* CALL_API createAgoraAudioInstance(IAgoraAudioEventHandler* pHandler);
extern "C" AGORA_API void CALL_API finalDestroyAgoraAudioInstance();

#endif
