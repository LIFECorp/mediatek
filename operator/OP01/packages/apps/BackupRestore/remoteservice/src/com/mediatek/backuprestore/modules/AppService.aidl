package com.mediatek.backuprestore.modules;

interface AppService {
    boolean disableApp(String appName);
    boolean enableApp(String appName);
}