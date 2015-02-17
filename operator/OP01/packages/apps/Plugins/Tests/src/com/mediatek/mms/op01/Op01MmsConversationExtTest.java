package com.mediatek.op01.tests;

import android.content.Context;
import android.content.Intent;
//import android.provider.MediaStore;
import android.test.InstrumentationTestCase;
import android.view.KeyEvent;
//import com.jayway.android.robotium.solo.Solo;
import android.view.Menu;
import android.view.MenuItem;
import java.util.ArrayList;
import android.view.SubMenu;
import android.content.ComponentName;
import android.graphics.drawable.Drawable;
import android.view.ContextMenu.ContextMenuInfo;
import android.view.View;
import android.view.ActionProvider;

import com.mediatek.xlog.Xlog;
import com.mediatek.pluginmanager.PluginManager;
import com.mediatek.pluginmanager.Plugin;
import com.mediatek.mms.op01.Op01MmsConversationExt;
import com.mediatek.mms.ext.IMmsConversationHost;

public class Op01MmsConversationExtTest extends InstrumentationTestCase implements IMmsConversationHost {

    private static final String TAG = "Conv test";
    
    private Context mContext = null;
    //private Instrumentation mInst = null;
    private Op01MmsConversationExt mConversationPlugin = null;
    private MmsTestMenu mMenu = null;
    private static final int sMenuBase = 0x100;
    private boolean mChangeModeSelected = false;
    private boolean mSimSmsSelected = false;
    
    @Override
    protected void setUp() throws Exception {
        Xlog.d(TAG, "setUp");
        super.setUp();
        
        mContext = this.getInstrumentation().getContext();
        //mInst = this.getInstrumentation();
        mMenu = new MmsTestMenu();
        mConversationPlugin = 
            (Op01MmsConversationExt)PluginManager.createPluginObject(mContext, "com.mediatek.mms.ext.IMmsConversation");

        if (mConversationPlugin != null) {
            Xlog.d(TAG, "get plugin failed");
            mConversationPlugin.init(this);
        }
    }
    
    @Override    
    protected void tearDown() throws Exception {
        Xlog.d(TAG, "tearDown");
        super.tearDown();
    }
/*
    private void initTestActivity() {
        Intent intent = new Intent(mContext, ConversationTestActivity.class);
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        mActivity = (ConversationTestActivity)mInst.startActivitySync(intent);
        sleep(300);
        mSolo = new Solo(mInst, mActivity);
    }
    */
    public void testAddOptionMenu() {
        Xlog.d(TAG, "testAddOptionMenu");
        mConversationPlugin.addOptionMenu(mMenu, sMenuBase);
        Xlog.d(TAG, "size=" + mMenu.size());
        assertEquals(2, mMenu.size());
    }

    public void testOnOptionsItemSelected() {
        Xlog.d(TAG, "testOnOptionsItemSelected");
        mConversationPlugin.addOptionMenu(mMenu, sMenuBase);
        MenuItem item = mMenu.getItem(0);
        Xlog.d(TAG, "0 = " + item.getItemId());
        mConversationPlugin.onOptionsItemSelected(item);
        Xlog.d(TAG, "0 select end");
        item = mMenu.getItem(1);
        Xlog.d(TAG, "1 = " + item.getItemId());
        mConversationPlugin.onOptionsItemSelected(item);
        Xlog.d(TAG, "1 select end");
        assertEquals(true, mSimSmsSelected);
        //assertEquals(true, mChangeModeSelected);
    }
    
    public void showSimSms() {
        Xlog.d(TAG, "getShowSimSms");
        mSimSmsSelected = true;
    }

    public void changeMode() {
        Xlog.d(TAG, "changeMode");
        mChangeModeSelected = true;
    }
}

class MmsTestMenu implements Menu {
    private static final String TAG = "Conv test";
    
    //private ArrayList<MmsTestMenuItem> mItems;
    private MmsTestMenuItem[] mItems;
    private int mCount = 0;

    public MmsTestMenu() {
        //mItems = new ArrayList<MmsTestMenuItem>();
        mItems = new MmsTestMenuItem[2];
    }
    
    public MenuItem add(CharSequence title) {
        return null;
    }
    
    public MenuItem add(int titleRes) {
        return null;
    }

    public MenuItem add(int groupId, int itemId, int order, CharSequence title) {
        Xlog.d(TAG, "menu add," + itemId + title);
        MmsTestMenuItem menuItem = new MmsTestMenuItem(itemId);
        //int pos = mItems.size();
        //Xlog.d(TAG, "pos=" + pos);
        //mItems.add(pos, menuItem);

        //MmsTestMenuItem item = mItems.get(pos);
        //Xlog.d(TAG, "id=" + item.getItemId());
        mItems[mCount] = menuItem;
        mCount++;
        return menuItem;
    }

    public MenuItem add(int groupId, int itemId, int order, int titleRes) {
        return null;
    }

    public SubMenu addSubMenu(final CharSequence title) {
        return null;
    }

    public SubMenu addSubMenu(final int titleRes) {
        return null;
    }

    public SubMenu addSubMenu(final int groupId, final int itemId, int order, final CharSequence title) {

        return null;
    }

    public SubMenu addSubMenu(int groupId, int itemId, int order, int titleRes) {
        return null;
    }

    public int addIntentOptions(int groupId, int itemId, int order,
                                ComponentName caller, Intent[] specifics,
                                Intent intent, int flags, MenuItem[] outSpecificItems) {
        return 0;
    }

    public void removeItem(int id) {
        return;
    }

    public void removeGroup(int groupId) {
        return;
    }

    public void clear() {
        //mItems.clear();
        for (int i = 0; i < mCount; i++) {
            mItems[i] = null;
        }
        mCount = 0;
    }

    public void setGroupCheckable(int group, boolean checkable, boolean exclusive) {
        return;
    }

    public void setGroupVisible(int group, boolean visible) {
        return;
    }
    
    public void setGroupEnabled(int group, boolean enabled) {
        return;
    }
    
    public boolean hasVisibleItems() {
        return false;
    }

    public MenuItem findItem(int id) {
        Xlog.d(TAG, "findItem," + id);
        for (MmsTestMenuItem item : mItems) {
            Xlog.d(TAG, "item," + item.getItemId());
            if (item.getItemId() == id) {
                return item;
            }
        }
        return null;
    }

    public int size() {
        return mCount;//mItems.size();
    }

    public MenuItem getItem(int index) {
        Xlog.d(TAG, "getItem," + index);
        Xlog.d(TAG, "item num=" + mCount);
        //MenuItem item = mItems.get(index);
        MenuItem item = mItems[index];
        Xlog.d(TAG, "item id=" + item.getItemId());

        MenuItem test = mItems[0]; //mItems.get(0);
        Xlog.d(TAG, "0 id=" + test.getItemId());
        test = mItems[0];//Items.get(1);
        Xlog.d(TAG, "1 id=" + test.getItemId());
        return item;
    }
    
    public void close() {
    }
    
    public boolean performShortcut(int keyCode, KeyEvent event, int flags) {
        return false;
    }

    public boolean isShortcutKey(int keyCode, KeyEvent event) {
        return false;
    }
    
    public boolean performIdentifierAction(int id, int flags) {
        return false;
    }

    public void setQwertyMode(boolean isQwerty) {
    }
}

class MmsTestMenuItem implements MenuItem {
    private static final String TAG = "Conv test";
    
    //private static final int MENU_ADD_SUBJECT = 0;
    private static int mId = 0;

    public MmsTestMenuItem() {
    }

    public MmsTestMenuItem(int id) {
        Xlog.d(TAG, "MmsTestMenuItem, " + id);
        mId = id;
    }
    
    public char getAlphabeticShortcut() {
        // TODO Auto-generated method stub
        return 0;
    }

    public int getGroupId() {
        // TODO Auto-generated method stub
        return 0;
    }

    public Drawable getIcon() {
        // TODO Auto-generated method stub
        return null;
    }

    public Intent getIntent() {
        // TODO Auto-generated method stub
        return null;
    }

    public int getItemId() {
        Xlog.d(TAG, "getItemId=" + mId);
        return mId;
    }

    public ContextMenuInfo getMenuInfo() {
        // TODO Auto-generated method stub
        return null;
    }

    public char getNumericShortcut() {
        // TODO Auto-generated method stub
        return 0;
    }

    public int getOrder() {
        // TODO Auto-generated method stub
        return 0;
    }

    public SubMenu getSubMenu() {
        // TODO Auto-generated method stub
        return null;
    }

    public CharSequence getTitle() {
        // TODO Auto-generated method stub
        return null;
    }

    public CharSequence getTitleCondensed() {
        // TODO Auto-generated method stub
        return null;
    }

    public boolean hasSubMenu() {
        // TODO Auto-generated method stub
        return false;
    }

    public boolean isCheckable() {
        // TODO Auto-generated method stub
        return false;
    }

    public boolean isChecked() {
        // TODO Auto-generated method stub
        return false;
    }

    public boolean isEnabled() {
        // TODO Auto-generated method stub
        return false;
    }

    public boolean isVisible() {
        // TODO Auto-generated method stub
        return false;
    }

    public MenuItem setAlphabeticShortcut(char alphaChar) {
        // TODO Auto-generated method stub
        return null;
    }

    public MenuItem setCheckable(boolean checkable) {
        // TODO Auto-generated method stub
        return null;
    }

    public MenuItem setChecked(boolean checked) {
        // TODO Auto-generated method stub
        return null;
    }

    public MenuItem setEnabled(boolean enabled) {
        // TODO Auto-generated method stub
        return null;
    }

    public MenuItem setIcon(Drawable icon) {
        // TODO Auto-generated method stub
        return null;
    }

    public MenuItem setIcon(int iconRes) {
        // TODO Auto-generated method stub
        return null;
    }

    public MenuItem setIntent(Intent intent) {
        // TODO Auto-generated method stub
        return null;
    }

    public MenuItem setNumericShortcut(char numericChar) {
        // TODO Auto-generated method stub
        return null;
    }

    public MenuItem setOnMenuItemClickListener(
            OnMenuItemClickListener menuItemClickListener) {
        // TODO Auto-generated method stub
        return null;
    }

    public MenuItem setShortcut(char numericChar, char alphaChar) {
        // TODO Auto-generated method stub
        return null;
    }

    public MenuItem setTitle(CharSequence title) {
        // TODO Auto-generated method stub
        return null;
    }

    public MenuItem setTitle(int title) {
        // TODO Auto-generated method stub
        return null;
    }

    public MenuItem setTitleCondensed(CharSequence title) {
        // TODO Auto-generated method stub
        return null;
    }

    public MenuItem setVisible(boolean visible) {
        // TODO Auto-generated method stub
        return null;
    }

    public void setShowAsAction(int actionEnum) {
        // TODO Auto-generated method stub

    }

    @Override
    public MenuItem setActionView(View view) {
        // TODO Auto-generated method stub
        return null;
    }

    @Override
    public View getActionView() {
        // TODO Auto-generated method stub
        return null;
    }

    @Override
    public MenuItem setActionView(int resId) {
        // TODO Auto-generated method stub
        return null;
    }

    @Override
    public MenuItem setShowAsActionFlags(int actionEnum) {
        // TODO Auto-generated method stub
        return null;
    }

    @Override
    public boolean expandActionView() {
        // TODO Auto-generated method stub
        return false;
    }

    @Override
    public boolean collapseActionView() {
        // TODO Auto-generated method stub
        return false;
    }

    @Override
    public boolean isActionViewExpanded() {
        return false;
    }

    @Override
    public MenuItem setOnActionExpandListener(OnActionExpandListener listener) {
        // TODO Auto-generated method stub
        return null;
    }

    @Override
    public MenuItem setActionProvider(ActionProvider actionProvider) {
        // TODO Auto-generated method stub
        return null;
    }

    @Override
    public ActionProvider getActionProvider() {
        // TODO Auto-generated method stub
        return null;
    }

}

/*
class ConversationTestActivity extends Activity implements IMmsConversationHost {
        
    private static final String TAG = "ConversationTestActivity";
    private Context mContext = null;
    private Op01MmsConversationExt mConversationPlugin = null;

    private static final int sMenuBase = 0x100;
    private boolean mChangeMode = false;
    private boolean mSimSms = false;
    
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        mConversationPlugin = 
            (Op01MmsConversationExt)PluginManager.createPluginObject(mContext, "com.mediatek.mms.ext.IMmsConversation");

        if (mConversationPlugin != null) {
            mConversationPlugin(this)
        }
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        mConversationPlugin.addOptionMenu(menu, sMenuBase);
    }

    void showSimSms() {
        mChangeMode = true;
    }

    void changeMode() {
        mSimSms = false;
    }

    boolean getShowSimSms() {
        return mSimSms;
    }

    boolean getChangeMode() {
        return mChangeMode;
    }
}
*/

