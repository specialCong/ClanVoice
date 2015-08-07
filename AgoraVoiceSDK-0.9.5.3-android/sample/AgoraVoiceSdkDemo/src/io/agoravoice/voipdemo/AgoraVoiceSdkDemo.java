package io.agoravoice.voipdemo;

import android.app.Activity;
import android.content.Context;
import android.media.AudioManager;
import android.os.Bundle;
import android.provider.Settings;
import android.text.method.ScrollingMovementMethod;
import android.util.Log;
import android.view.Menu;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ScrollView;
import android.widget.TextView;
import android.widget.Toast;

public class AgoraVoiceSdkDemo extends Activity {

	private static final String LOG_TAG = AgoraVoiceSdkDemo.class.getSimpleName();

	private enum AppState {
		// Quit -> IDLE
		IDLE("Join"), // Join -> RUNNING
		RUNNING("Mute"), // Mute -> MUTED
		MUTED("Unmute"); // Unmute -> RUNNING

		private String buttonLabel;

		AppState(String l) {
			buttonLabel = l;
		}

		public String getLabel() {
			return buttonLabel;
		}
	}

	public static String getDeviceID(Context context) {
		// XXX according to the API docs,
		// this value may change after factory reset
		return Settings.Secure.getString(context.getContentResolver(),
				Settings.Secure.ANDROID_ID);
	}

	public static boolean isBlank(String str) {
		if (null == str || "".equals(str.trim())) {
			return true;
		}
		return false;
	}

	private AppState mState;

	private View mScrollView;
	private View mIdentificationView;
	private TextView mChannelStatus;
	private EditText mKey;
	private EditText mChannelId;
	private EditText mUserId;
	private Button mCtrlBtn;
	private Button mLeaveBtn;
	private Button mSwitcherBtn;
    private android.media.AudioManager mAm;

	private volatile boolean mSpeakerOn;

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		getMenuInflater().inflate(R.menu.main, menu);
		return true;
	}

	private MediaHandler mHandler = new MediaHandler() {
		@MessageHandler(message = MediaMessage.onJoinRes)
		public void onJoinRes() {

			runOnUiThread(new Runnable() {
				@Override
				public void run() {
					mChannelStatus.setText("joined");
				}
			});
		}

		@MessageHandler(message = MediaMessage.onWriteLog)
		public void onWriteLog(final String log) {
			runOnUiThread(new Runnable() {
				@Override
				public void run() {
					mChannelStatus.append(log + "\n");
					notifyToEnd();
				}
			});
		}
	};

	private void notifyToEnd() {
		ScrollView scroll = (ScrollView) findViewById(R.id.scrollView);
		scroll.fullScroll(View.FOCUS_DOWN);
	}

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);

		MediaDemoApplication app = (MediaDemoApplication) getApplication();
		app.setActivity(this);

		mState = AppState.IDLE;

		mScrollView = findViewById(R.id.scrollView);
		mIdentificationView = findViewById(R.id.identification_control);

		mChannelStatus = (TextView) findViewById(R.id.channel_status);
		mChannelStatus.setMovementMethod(ScrollingMovementMethod.getInstance());

        mChannelStatus.setText("Welcome to Agora Voip Demo!\n");

		mKey = (EditText) findViewById(R.id.key);
		mChannelId = (EditText) findViewById(R.id.channel_id);
		mUserId = (EditText) findViewById(R.id.user_id);

		mCtrlBtn = (Button) findViewById(R.id.btn_ctrl);
		mCtrlBtn.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View v) {
				if (mState == AppState.IDLE) {
					// String key = "key for Vendor granted by Agora";
					String key = mKey.getText().toString(); // "6D7A26A1D3554A54A9F43BE6797FE3E2"
					String extraInfo = "extra info you pass to SDK";
					final String channel = mChannelId.getText().toString();

					String rawUid = mUserId.getText().toString();
					final int userId = Integer.parseInt(isBlank(rawUid) ? "0" : rawUid);

					if (isBlank(channel) || isBlank(key)) {
						Toast.makeText(getApplicationContext(), "Channel ID or key is empty!", Toast.LENGTH_LONG).show();
						return;
					}

					MediaDemoApplication app = (MediaDemoApplication) getApplication();
					Log.d(LOG_TAG, "before join channel");
					if (requestAudioFocus())
					{
						app.getAgoraAudio().joinChannel(key, channel, extraInfo, userId);
						setAudioOutput();
					}
					else
					{
						Toast.makeText(getApplicationContext(), "Audio focus is required to workaround an android platform issue", Toast.LENGTH_LONG).show();
						Log.d(LOG_TAG, "failed to acquire audio focus");
						return;
					}
					Log.d(LOG_TAG, "after join channel");
					mState = AppState.RUNNING;

					mIdentificationView.setVisibility(View.GONE);
					mScrollView.setVisibility(View.VISIBLE);

					mSwitcherBtn.setVisibility(View.VISIBLE); // show output switcher
					
					
				} else if (mState == AppState.RUNNING) {
					MediaDemoApplication app = (MediaDemoApplication) getApplication();
					app.getAgoraAudio().mute(true);
					mState = AppState.MUTED;
				} else if (mState == AppState.MUTED) {
					MediaDemoApplication app = (MediaDemoApplication) getApplication();
					app.getAgoraAudio().mute(false);
					mState = AppState.RUNNING;
				}

				mCtrlBtn.setText(mState.getLabel());
			}
		});

		mLeaveBtn = (Button) findViewById(R.id.btn_leave);
		mLeaveBtn.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View v) {
				doQuit();
			}
		});

		mSwitcherBtn = (Button) findViewById(R.id.btn_switcher);
		mSwitcherBtn.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View v) {
				mSpeakerOn = !mSpeakerOn;
				setAudioOutput();
			}
		});
		mSpeakerOn = false;

        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        setVolumeControlStream(AudioManager.STREAM_VOICE_CALL);

        app.createAudioSDKInstance(true);
	}
	
	@Override
	protected void onDestroy() {
		super.onDestroy();
		mState = AppState.IDLE;
		getWindow().clearFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
	}

	@Override
	protected void onResume() {
		super.onResume();

		MediaDemoApplication app = (MediaDemoApplication) getApplication();
		app.getMediaHandlerMgr().add(mHandler);
	}

	@Override
	protected void onPause() {
		super.onPause();

		MediaDemoApplication app = (MediaDemoApplication) getApplication();
		app.getMediaHandlerMgr().remove(mHandler);
	}

	@Override
	public void onBackPressed() {
		doQuit();
	}

	public void setAudioOutput() {
		if (mSpeakerOn) {
			mSwitcherBtn.setText("Earpiece");
			Log.i(LOG_TAG, "Set audio output to speaker");
		} else {
			mSwitcherBtn.setText("Speaker");
			Log.i(LOG_TAG, "Set audio output to earpiece");
		}
		MediaDemoApplication app = (MediaDemoApplication) getApplication();
		app.getAgoraAudio().setSpeaker(mSpeakerOn);
	}

	private void doQuit() {
		if (mState != AppState.IDLE) {
			MediaDemoApplication app = (MediaDemoApplication) getApplication();
			Log.d(LOG_TAG, "before leave channel");
			app.getAgoraAudio().leaveChannel();

 			abandonAudioFocus();
			Log.d(LOG_TAG, "after leave channel");
			mState = AppState.IDLE;

			mIdentificationView.setVisibility(View.VISIBLE);
			mScrollView.setVisibility(View.GONE);
			mCtrlBtn.setText(mState.getLabel());

			mSwitcherBtn.setVisibility(View.GONE); // hide output switcher
		} else {
			finish();
		}
	}
    boolean requestAudioFocus() { // can always call requestAudioFocus, if already has it, then result is still AUDIOFOCUS_REQUEST_GRANTED
        // try do request audio focus in audio thread(try to keep the setMode in sequence)
        mAm = (android.media.AudioManager) this.getSystemService(Context.AUDIO_SERVICE);
        int result = mAm.requestAudioFocus(afChangeListener,
                android.media.AudioManager.STREAM_VOICE_CALL,
                android.media.AudioManager.AUDIOFOCUS_GAIN);


        if (result == android.media.AudioManager.AUDIOFOCUS_REQUEST_FAILED) {
            return false;
        }

        if (result == android.media.AudioManager.AUDIOFOCUS_REQUEST_GRANTED) {
            //mSystemWideAudioMode = mAm.getMode();
            //mAm.setMode(android.media.AudioManager.MODE_IN_COMMUNICATION);
//            int volume = mAm.getStreamVolume(android.media.AudioManager.STREAM_VOICE_CALL);
//            int maxVolume = mAm.getStreamMaxVolume(android.media.AudioManager.STREAM_VOICE_CALL);
            return true;
        }
        throw new IllegalAccessError("Trespass");
    }
    void abandonAudioFocus() {
        mAm = (android.media.AudioManager) this.getSystemService(Context.AUDIO_SERVICE);
        mAm.abandonAudioFocus(afChangeListener);
    }
    android.media.AudioManager.OnAudioFocusChangeListener afChangeListener = new android.media.AudioManager.OnAudioFocusChangeListener() {
        public void onAudioFocusChange(int focusChange) {
            // for AUDIOFOCUS_LOSS_TRANSIENT_CAN_DUCK | AUDIOFOCUS_LOSS_TRANSIENT, we do nothing
            if (focusChange == android.media.AudioManager.AUDIOFOCUS_LOSS) {
                mAm.abandonAudioFocus(this);
//                sysMessage(BKMessageCode.AUDIO_FOCUS_LOST, null);
            } else if (focusChange == android.media.AudioManager.AUDIOFOCUS_GAIN) {
                //mAm.setMode(android.media.AudioManager.MODE_IN_COMMUNICATION);
            }
        }
    };
}
