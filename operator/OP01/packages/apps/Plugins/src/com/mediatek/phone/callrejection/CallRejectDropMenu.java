package com.mediatek.phone.callrejection;

import android.content.Context;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.PopupMenu;
import android.widget.PopupMenu.OnMenuItemClickListener;

import com.mediatek.op01.plugin.R;

import java.util.ArrayList;

public class CallRejectDropMenu implements OnMenuItemClickListener {

    //private static final String TAG = "CallRejectDropMenu";

    public static class DropDownMenu {
        private Button mButton;
        private PopupMenu mPopupMenu;
        private Menu mMenu;

        public DropDownMenu(Context context, Button selectItem, int menuId,
                OnMenuItemClickListener listener) {
            mButton = selectItem;
            //mButton.setBackgroundDrawable(context.getResources().getDrawable( R.drawable.dropdown_normal_holo_dark));
            mPopupMenu = new PopupMenu(context, mButton);
            mMenu = mPopupMenu.getMenu();
            // add the menu("Select all" or "Deselect all") on the button("XX selected").
            mPopupMenu.getMenuInflater().inflate(menuId, mMenu);
            mPopupMenu.setOnMenuItemClickListener(listener);
            mButton.setOnClickListener(new OnClickListener() {
                public void onClick(View v) {
                    // when click the button, show the popupMenu.
                    mPopupMenu.show();
                }
            });
        }

        /**
         * find menu item from menu id.
         * @param id
         * @return the menu
         */
        public MenuItem findItem(int id) {
            return mMenu.findItem(id);
        }

        public void show() {
            mPopupMenu.show();
        }
    }

    private Context mContext;
    private ArrayList<DropDownMenu> mMenus;
    private OnMenuItemClickListener mListener;

    public CallRejectDropMenu(Context context) {
        mContext = context;
        mMenus = new ArrayList<DropDownMenu>();
    }

    /**
     * new and add a menu on select_item.
     * @param select_item
     * @param menuId
     * @return
     */
    public DropDownMenu addDropDownMenu(Button selectItem, int menuId) {
        DropDownMenu menu = new DropDownMenu(mContext, selectItem, menuId, this);
        mMenus.add(menu);
        return menu;
    }
    public void setOnMenuItemClickListener(OnMenuItemClickListener listener) {
        mListener = listener;
    }
    public boolean onMenuItemClick(MenuItem item) {
        if (mListener != null) {
            return mListener.onMenuItemClick(item);
        }
        return false;
    }
}
