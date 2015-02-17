package com.mediatek.op.audioprofile;

import java.util.HashMap;

import com.mediatek.audioprofile.AudioProfileManager;
import com.mediatek.audioprofile.AudioProfileState;
import com.mediatek.audioprofile.AudioProfileManager.Scenario;

public class DefaultProfileStatesGetterOP01 extends DefaultProfileStatesGetter {
	@Override
	public HashMap<Integer, AudioProfileState> getDefaultProfileStates() {
		final HashMap<Integer, AudioProfileState> DEFAULTSTATES = new HashMap<Integer, AudioProfileState>(
				AudioProfileManager.PREDEFINED_PROFILES_COUNT);

		/** Default values of ringer volume for different audio profiles. */
		final int DEFAULT_RINGER_VOLUME_GENERAL = 8;
		final int DEFAULT_RINGER_VOLUME_SILENT = 0;
		final int DEFAULT_RINGER_VOLUME_MEETING = 0;
		final int DEFAULT_RINGER_VOLUME_OUTDOOR = 12;

		/** Default values of notification volume for different audio profiles. */
		final int DEFAULT_NOTIFICATION_VOLUME_GENERAL = 8;
		final int DEFAULT_NOTIFICATION_VOLUME_SILENT = 0;
		final int DEFAULT_NOTIFICATION_VOLUME_MEETING = 0;
		final int DEFAULT_NOTIFICATION_VOLUME_OUTDOOR = 12;

		/** Default values of alarm volume for different audio profiles. */
		final int DEFAULT_ALARM_VOLUME_GENERAL = 8;
		final int DEFAULT_ALARM_VOLUME_SILENT = 0;
		final int DEFAULT_ALARM_VOLUME_MEETING = 8;
		final int DEFAULT_ALARM_VOLUME_OUTDOOR = 12;

		/** Default values of vibration for different audio profiles. */
		final boolean DEFAULT_VIBRATION_GENERAL = false;
		final boolean DEFAULT_VIBRATION_SILENT = false;
		final boolean DEFAULT_VIBRATION_MEETING = true;
		final boolean DEFAULT_VIBRATION_OUTDOOR = true;

		/**
		 * Default values that indicate whether the audible DTMF tone should be
		 * played by the dialer when dialing.
		 */
		final boolean DEFAULT_DTMFTONE_GENERAL = true;
		final boolean DEFAULT_DTMFTONE_SILENT = false;
		final boolean DEFAULT_DTMFTONE_MEETING = false;
		final boolean DEFAULT_DTMFTONE_OUTDOOR = true;

		/**
		 * Default values of sound effect(Key clicks, lid open/close...) for
		 * different audio profiles.
		 */
		final boolean DEFAULT_SOUNDEFFECT_GENERAL = false;
		final boolean DEFAULT_SOUNDEFFECT_SILENT = false;
		final boolean DEFAULT_SOUNDEFFECT_MEETING = false;
		final boolean DEFAULT_SOUNDEFFECT_OUTDOOR = false;

		/**
		 * Default values that indicate whether the lock screen sound are
		 * enabled.
		 */
		final boolean DEFAULT_LOCK_SCREEN_GENERAL = true;
		final boolean DEFAULT_LOCK_SCREEN_SILENT = false;
		final boolean DEFAULT_LOCK_SCREEN_MEETING = false;
		final boolean DEFAULT_LOCK_SCREEN_OUTDOOR = true;

		/**
		 * Default values that indicate whether the haptic feedback are enabled.
		 */
		final boolean DEFAULT_HAPTIC_FEEDBACK_GENERAL = false;
		final boolean DEFAULT_HAPTIC_FEEDBACK_SILENT = false;
		final boolean DEFAULT_HAPTIC_FEEDBACK_MEETING = true;
		final boolean DEFAULT_HAPTIC_FEEDBACK_OUTDOOR = true;

		// Init general state and push it to DEFAULTSTATES
		AudioProfileState generalState = new AudioProfileState.Builder(
				AudioProfileManager.getProfileKey(Scenario.GENERAL)).ringtone(
				AudioProfileManager.DEFAULT_RINGER_STREAM_URI,
				AudioProfileManager.DEFAULT_NOTIFICATION_STREAM_URI,
				AudioProfileManager.DEFAULT_VIDEO_STREAM_URI).volume(
				DEFAULT_RINGER_VOLUME_GENERAL,
				DEFAULT_NOTIFICATION_VOLUME_GENERAL,
				DEFAULT_ALARM_VOLUME_GENERAL).vibration(
				DEFAULT_VIBRATION_GENERAL).dtmfTone(DEFAULT_DTMFTONE_GENERAL)
				.soundEffect(DEFAULT_SOUNDEFFECT_GENERAL).lockScreenSound(
						DEFAULT_LOCK_SCREEN_GENERAL).hapticFeedback(
						DEFAULT_HAPTIC_FEEDBACK_GENERAL).build();
		DEFAULTSTATES.put(Scenario.GENERAL.ordinal(), generalState);

		// Init silent state and push it to DEFAULTSTATES
		AudioProfileState silentState = new AudioProfileState.Builder(
				AudioProfileManager.getProfileKey(Scenario.SILENT)).ringtone(
				AudioProfileManager.DEFAULT_RINGER_STREAM_URI,
				AudioProfileManager.DEFAULT_NOTIFICATION_STREAM_URI,
				AudioProfileManager.DEFAULT_VIDEO_STREAM_URI)
				.volume(DEFAULT_RINGER_VOLUME_SILENT,
						DEFAULT_NOTIFICATION_VOLUME_SILENT,
						DEFAULT_ALARM_VOLUME_SILENT).vibration(
						DEFAULT_VIBRATION_SILENT).dtmfTone(
						DEFAULT_DTMFTONE_SILENT).soundEffect(
						DEFAULT_SOUNDEFFECT_SILENT).lockScreenSound(
						DEFAULT_LOCK_SCREEN_SILENT).hapticFeedback(
						DEFAULT_HAPTIC_FEEDBACK_SILENT).build();
		DEFAULTSTATES.put(Scenario.SILENT.ordinal(), silentState);

		// Init meeting state and push it to DEFAULTSTATES
		AudioProfileState meetingState = new AudioProfileState.Builder(
				AudioProfileManager.getProfileKey(Scenario.MEETING)).ringtone(
				AudioProfileManager.DEFAULT_RINGER_STREAM_URI,
				AudioProfileManager.DEFAULT_NOTIFICATION_STREAM_URI,
				AudioProfileManager.DEFAULT_VIDEO_STREAM_URI).volume(
				DEFAULT_RINGER_VOLUME_MEETING,
				DEFAULT_NOTIFICATION_VOLUME_MEETING,
				DEFAULT_ALARM_VOLUME_MEETING).vibration(
				DEFAULT_VIBRATION_MEETING).dtmfTone(DEFAULT_DTMFTONE_MEETING)
				.soundEffect(DEFAULT_SOUNDEFFECT_MEETING).lockScreenSound(
						DEFAULT_LOCK_SCREEN_MEETING).hapticFeedback(
						DEFAULT_HAPTIC_FEEDBACK_MEETING).build();
		DEFAULTSTATES.put(Scenario.MEETING.ordinal(), meetingState);

		// Init outdoor state and push it to DEFAULTSTATES
		AudioProfileState outdoorState = new AudioProfileState.Builder(
				AudioProfileManager.getProfileKey(Scenario.OUTDOOR)).ringtone(
				AudioProfileManager.DEFAULT_RINGER_STREAM_URI,
				AudioProfileManager.DEFAULT_NOTIFICATION_STREAM_URI,
				AudioProfileManager.DEFAULT_VIDEO_STREAM_URI).volume(
				DEFAULT_RINGER_VOLUME_OUTDOOR,
				DEFAULT_NOTIFICATION_VOLUME_OUTDOOR,
				DEFAULT_ALARM_VOLUME_OUTDOOR).vibration(
				DEFAULT_VIBRATION_OUTDOOR).dtmfTone(DEFAULT_DTMFTONE_OUTDOOR)
				.soundEffect(DEFAULT_SOUNDEFFECT_OUTDOOR).lockScreenSound(
						DEFAULT_LOCK_SCREEN_OUTDOOR).hapticFeedback(
						DEFAULT_HAPTIC_FEEDBACK_OUTDOOR).build();
		DEFAULTSTATES.put(Scenario.OUTDOOR.ordinal(), outdoorState);

		return DEFAULTSTATES;
	}
}
