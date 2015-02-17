package com.mediatek.backuprestore;

public class SettingData {
    String title = null;
    String summary = null;
    boolean checked = false;
    String type = null;
    

    public SettingData(String title, String summary, Boolean checked, String type) {
        this.title = title;
        this.summary = summary;
        this.checked = checked;
        this.type = type;
    }
    
    public String getTitle() {
        return title;
    }

    public void setTitle(String title) {
        this.title = title;
    }

    public String getSummary() {
        return summary;
    }

    public void setSummary(String summary) {
        this.summary = summary;
    }

    public boolean isChecked() {
        return checked;
    }

    public void setChecked(boolean checked) {
        this.checked = checked;
    }

    public String getType() {
        return type;
    }

    public void setType(String type) {
        this.type = type;
    }
}
