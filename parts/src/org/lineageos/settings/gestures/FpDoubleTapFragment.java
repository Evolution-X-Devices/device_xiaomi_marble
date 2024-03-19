/*
 * Copyright (C) 2023 Paranoid Android
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package org.lineageos.settings.gestures;

import android.os.Bundle;
import android.os.UserHandle;
import android.provider.Settings;
import android.util.Log;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;

import androidx.preference.ListPreference;
import androidx.preference.Preference;
import androidx.preference.Preference.OnPreferenceChangeListener;
import androidx.preference.PreferenceFragmentCompat;

import com.android.settingslib.widget.MainSwitchPreference;

import org.lineageos.settings.R;

public class FpDoubleTapFragment extends PreferenceFragmentCompat implements
        OnPreferenceChangeListener, OnCheckedChangeListener {

    private static final String TAG = "FpDoubleTapFragment";
    private static final String PREF_ENABLE = "fp_double_tap_enable";
    private static final String PREF_ACTION = "fp_double_tap_action";

    private MainSwitchPreference mSwitchBar;
    private ListPreference mActionPref;

    @Override
    public void onCreatePreferences(Bundle savedInstanceState, String rootKey) {
        addPreferencesFromResource(R.xml.fp_double_tap_settings);

        final boolean enabled = GestureUtils.isFpDoubleTapEnabled(getActivity());
        final int action = GestureUtils.getFpDoubleTapAction(getActivity());

        mSwitchBar = (MainSwitchPreference) findPreference(PREF_ENABLE);
        mSwitchBar.addOnSwitchChangeListener(this);
        mSwitchBar.setChecked(enabled);

        mActionPref = (ListPreference) findPreference(PREF_ACTION);
        mActionPref.setEnabled(enabled);
        mActionPref.setOnPreferenceChangeListener(this);
        mActionPref.setValue(Integer.toString(action));
    }

    @Override
    public boolean onPreferenceChange(Preference preference, Object newValue) {
        switch (preference.getKey()) {
            case PREF_ACTION:
                final int action = Integer.parseInt((newValue.toString()));
                Settings.System.putIntForUser(getActivity().getContentResolver(),
                        GestureUtils.SETTING_KEY_ACTION, action, UserHandle.USER_CURRENT);
                return true;
            default:
                return false;
        }
    }

    @Override
    public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
        mActionPref.setEnabled(isChecked);
        Settings.System.putIntForUser(getActivity().getContentResolver(),
                GestureUtils.SETTING_KEY_ENABLE, isChecked ? 1 : 0, UserHandle.USER_CURRENT);
        GestureUtils.setFingerprintNavigation(isChecked);
    }

}
