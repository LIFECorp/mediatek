/*
 * Copyright (C) 2008 The Android Open Source Project
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

package com.mediatek.settings.plugin;

import android.preference.PreferenceActivity;
import android.preference.PreferenceGroup;
import android.os.Bundle;

import com.mediatek.op02.tests.R;

public class MockActivity extends PreferenceActivity {

    public static final String MOCK_RREFERENCE_KEY = "mock_preference";

    private PreferenceGroup mMockPreferenceGroup;

    @Override
    protected void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        addPreferencesFromResource(R.xml.mock_preference);
        mMockPreferenceGroup = (PreferenceGroup) findPreference(MOCK_RREFERENCE_KEY);
    }

    public PreferenceGroup getMockPreferenceGroup() {
        return mMockPreferenceGroup;
    }
}
