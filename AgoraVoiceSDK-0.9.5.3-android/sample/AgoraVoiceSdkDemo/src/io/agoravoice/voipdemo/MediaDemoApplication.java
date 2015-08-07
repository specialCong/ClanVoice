package io.agoravoice.voipdemo;

import java.util.concurrent.atomic.AtomicReference;


import android.app.Application;
import android.util.Log;
import io.agoravoice.voiceengine.AgoraAudio;

public class MediaDemoApplication extends Application {

	private static final String LOG = MediaDemoApplication.class
			.getSimpleName();

	private AgoraAudio mNative = null;
	private MediaHandlerMgr mHandlerMgr = null;
	private AgoraVoiceSdkDemo mActivity = null;

	private static AtomicReference<MediaDemoApplication> mInstance = new AtomicReference<MediaDemoApplication>();

	public static MediaDemoApplication getInstance() {
		return mInstance.get();
	}
	public void setActivity(AgoraVoiceSdkDemo activity) {
		mActivity = activity;
	}
	public AgoraVoiceSdkDemo getActivity() {
		return mActivity;
	}

	@Override
	public void onCreate() {
		Log.i(LOG, "onCreate begin");

		super.onCreate();

		mInstance.set(this);
		mNative = null;
		mHandlerMgr = new MediaHandlerMgr(this);
		Log.i(LOG, "onCreate end");
	}

	public synchronized void createAudioSDKInstance(boolean useOpenslesRecordingDevice) {
		if (mNative == null) {
			mNative = new AgoraAudio(this, mHandlerMgr, false);
			mNative.monitorHeadsetEvent(true);
			mNative.monitorConnectionEvent(true);
			mNative.monitorBluetoothHeadsetEvent(true);
			mNative.enableHighPerfWifiMode(true);
		}
	}

	AgoraAudio getAgoraAudio() {
		return mNative;
	}

	MediaHandlerMgr getMediaHandlerMgr() {
		return mHandlerMgr;
	}

}
