PATH=/data/busybox/:/system/xbin/:/system/bin/:$PATH

getprop | grep JB
if [ $? ]; then
    # GB
    lockscreen="update secure set value=1 where name='lockscreen.disabled';"
    lockscreen="delete from secure where name='lockscreen.disabled'; insert into secure (name, value) values ('lockscreen.disabled','1');"
    lock_db="/data/data/com.android.providers.settings/databases/settings.db"
    stay_awake="update system set value=3 where name='stay_on_while_plugged_in';"
else
    # JB
    lockscreen="update locksettings set value=1 where name='lockscreen.disabled'"
    lock_db="/data/system/locksettings.db"
    stay_awake="update global set value=3 where name='stay_on_while_plugged_in';"
fi

while [ ! -f $lock_db ]; do sleep 1; done
sqlite3 $lock_db "${lockscreen}"
while [ $? != 0 ]; do sqlite3 $lock_db "${lockscreen}"; done

screen_timeout="update system set value=2147483647 where name='screen_off_timeout';"
settings_db="/data/data/com.android.providers.settings/databases/settings.db"

while [ ! -f $settings_db ]; do sleep 1; done
sqlite3 $settings_db "${screen_timeout}"
while [ $? != 0 ]; do sqlite3 $settings_db "${screen_timeout}"; done
sqlite3 $settings_db "${stay_awake}"
while [ $? != 0 ]; do sqlite3 $settings_db "${stay_awake}"; done


