package io.agoravoice.voipdemo;

import java.util.concurrent.CopyOnWriteArraySet;
import android.os.Message;
import io.agoravoice.voiceengine.AgoraEvent;
import io.agoravoice.voiceengine.IAudioEventHandler;

public class MediaHandlerMgr implements IAudioEventHandler {

	public MediaHandlerMgr(MediaDemoApplication app) {
		mApp = app;
	}
	private MediaDemoApplication mApp;
	private CopyOnWriteArraySet<MediaHandler> mHandlers = new CopyOnWriteArraySet<MediaHandler>();

	public void add(MediaHandler handler) {
		mHandlers.add(handler);
	}

	public void remove(MediaHandler handler) {
		mHandlers.remove(handler);
	}

	public boolean notify2UIThread(int message) {
		return notify2UIThread(message, (Object[]) null);
	}

	public boolean notify2UIThread(int message, Object... params) {
		for (MediaHandler handler : mHandlers) {
			if (handler.canHandleMessage(message)) {
				Message msg = handler.obtainMessage();
				msg.what = message;
				msg.obj = params;
				handler.sendMessage(msg);
			}
		}
		return true;
	}

	@Override
	public void onLogEvent(int level, String message) {
		notify2UIThread(MediaMessage.onWriteLog, message);
	}

	@Override
	public void onError(int arg0) {
        notify2UIThread(MediaMessage.onWriteLog, "Agora Voice SDK report error: " + arg0);
	}

	@Override
	public void onJoinSuccess(String sid, int uid, int elapsed) {
		notify2UIThread(MediaMessage.onWriteLog, "Channel joined: sid " + sid + " uid " + (uid&0xFFFFFFFFL) + " elapsed "+ elapsed + " ms");
	}

    @Override
    public void onRejoinSuccess(String sid, int uid, int elapsed) {
        notify2UIThread(MediaMessage.onWriteLog, "Channel rejoined: sid " + sid + " uid " + (uid&0xFFFFFFFFL) + " elapsed "+ elapsed + " ms");
    }

	@Override
	public void onLeaveChannel(SessionStats stats) {
		notify2UIThread(MediaMessage.onWriteLog, "end of call: duration " + stats.totalDuration + " secs, total " + stats.totalBytes + " bytes");
	}

	@Override
	public void onUpdateSessionStats(SessionStats stats) {
		onNetworkQuality(stats.networkQuality);
	}
	@Override
	public void onLoadAudioEngineSuccess() {
        notify2UIThread(MediaMessage.onWriteLog, "Agora audio engine loaded and call started");
	}
	static String getQualityDesc(int quality) {
		switch (quality) {
		case AgoraEvent.MediaQuality.EXCELLENT:
			return "5";
		case AgoraEvent.MediaQuality.GOOD:
			return "4";
		case AgoraEvent.MediaQuality.POOR:
			return "3";
		case AgoraEvent.MediaQuality.BAD:
			return "2";
		case AgoraEvent.MediaQuality.VBAD:
			return "1";
		case AgoraEvent.MediaQuality.DOWN:
			return "0";
		case AgoraEvent.MediaQuality.UNKNOWN:
		default:
			return "unknown";
		}
	}

	@Override
        public void onAudioQuality(int uid, int quality, short delay, short jitter, short lost, short lost2) {
        String msg = String.format("user %d quality %s delay %d jitter %d lost %d/%d", (uid&0xFFFFFFFFL), getQualityDesc(quality), delay, jitter, lost, lost2);
		notify2UIThread(MediaMessage.onWriteLog, msg);
	}
	
	@Override
	public void onUserJoined(int uid, int elapsed) {
	
	}
	
	@Override
	public void onUserOffline(int uid) {
        notify2UIThread(MediaMessage.onWriteLog, "user " + (uid&0xFFFFFFFFL) + " is offline");
	}
	@Override
	public void onRecapStat(byte[] recap) {
		
	}
	
	@Override
	public void onUserMuteAudio(int uid, boolean muted) {
	
	}
	
	@Override
    public void onUserMuteVideo(int uid, boolean muted) {
	
	}
	
	@Override
	public void onSpeakersReport(SpeakerInfo[] speakers, int mixVolume) {
		if (speakers != null) {
			//notify2UIThread(MediaMessage.onWriteLog, "user " + (speakers[0].uid&0xFFFFFFFFL) + " volume "+speakers[0].volume);
		}
	}
	@Override
	public void onNetworkQuality(int quality) {
//        String msg = String.format("network quality %s", getQualityDesc(quality));
//        notify2UIThread(MediaMessage.onWriteLog, msg);
	}
	//@Override
	public void onPeerConnected(int callSetupTime) {
      String msg = String.format("peer connected, call setup time %d ms", callSetupTime);
      notify2UIThread(MediaMessage.onWriteLog, msg);
	}
	
    @Override
    public void onLocalVideoStat(int sentBytes, int sentFrames, int sentQP, int sentRtt, int sentLoss) {

    }

    @Override
    public void onFirstVideoFrame(int uid, int width, int height) {

    }
	
	@Override
	public void onRemoteVideoStat(int uid, int frameCount, int delay, int receivedBytes){
	
	}
  
}
