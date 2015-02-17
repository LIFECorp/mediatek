package com.mediatek.ppl.ui;

import android.app.Activity;
import android.app.AlertDialog.Builder;
import android.app.Dialog;
import android.app.DialogFragment;
import android.content.Context;
import android.content.DialogInterface;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.ImageView.ScaleType;
import android.widget.ListView;
import android.widget.TextView;

import com.mediatek.ppl.PlatformManager;
import com.mediatek.ppl.R;

import java.util.HashMap;
import java.util.List;

public class ChooseSimDialogFragment extends DialogFragment {
    public static final String ARG_KEY_ITEMS = "items";
    public static final String ARG_KEY_VALUES = "values";
    public static final String TAG = "PPL/ChooseSim";

    public static interface ISendMessage {
        void sendMessage(int simId);
    }

    @Override
    public Dialog onCreateDialog(Bundle savedInstanceState) {
        final Activity activity = getActivity();
        Bundle args = getArguments();
        String[] items = args.getStringArray(ARG_KEY_ITEMS);
        final int[] values = args.getIntArray(ARG_KEY_VALUES);

        List<HashMap<String, Object>> data = PlatformManager.buildSimInfo(this.getActivity());

        if (data.size() == items.length) {

            for (int i = 0; i < data.size(); i++) {
                data.get(i).put("SimId", values[i]);
            }

            ChooseDialog dialog = new ChooseDialog(this.getActivity(), data);
            return dialog;

        } else {
            Builder builder = new Builder(activity);
            builder.setTitle(R.string.title_choose_sim);

            builder.setNegativeButton(android.R.string.cancel, null);
            builder.setItems(items, new DialogInterface.OnClickListener() {
                public void onClick(DialogInterface dialog, int which) {
                    ((ISendMessage) activity).sendMessage(values[which]);
                }
            });

            return builder.create();
        }
    }

    private class ChooseDialog extends Dialog  {

        public ChooseDialog(final Context context, final List<HashMap<String, Object>> data) {
            super(context);
            setContentView(R.layout.choose_sim);
            setTitle(R.string.title_choose_sim);

            ListView listView = (ListView) findViewById(R.id.list_choose_sim);

            MyBaseAdapter adapter = new MyBaseAdapter(context, data);
            listView.setAdapter(adapter);

            listView.setOnItemClickListener(new OnItemClickListener() {
                @Override
                public void onItemClick(AdapterView<?> arg0, View v, int position, long id) {
                    ((ISendMessage) context).sendMessage((Integer) data.get(position).get("SimId"));
                }
            });
        }
    }

    private class MyBaseAdapter extends BaseAdapter {
        private Context mContext;
        private List<HashMap<String, Object>> mDataList;

        public MyBaseAdapter(Context context, List<HashMap<String, Object>> data) {
            mContext = context;
            mDataList = data;
        }

        @Override
        public int getCount() {
            return mDataList.size();
        }

        @Override
        public HashMap<String, Object> getItem(int position) {
            return mDataList.get(position);
        }

        @Override
        public long getItemId(int position) {
            return position;
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            convertView = LayoutInflater.from(mContext).inflate(
                    R.layout.list_item, null);
            ImageView image = (ImageView) convertView.findViewById(R.id.ItemImage);
            TextView title = (TextView) convertView.findViewById(R.id.ItemTitle);

            title.setText((String) getItem(position).get("ItemTitle"));
            setSimIndicatorIcon(image, (Integer) getItem(position).get("ItemImage"));
            setSimBackgroundColor(image, (Integer) getItem(position).get("Color"));

            image.setScaleType(ScaleType.CENTER);
            return convertView;
        }
    }

    private void setSimIndicatorIcon(ImageView imageView, int indicator) {
        if (indicator == -1) {
            imageView.setVisibility(View.GONE);
            Log.i(TAG, "Unable to show indicator icon");
        } else {
            int res = PlatformManager.getStatusResource(indicator);
            imageView.setImageResource(res);
            imageView.setVisibility(View.VISIBLE);
        }
    }

    private void setSimBackgroundColor(ImageView imageView, int colorId) {
        if (colorId >= 0) {
            int resColor = PlatformManager.getSimColorResource(colorId);
            if (resColor >= 0) {
                imageView.setBackgroundResource(resColor);
                imageView.setVisibility(View.VISIBLE);
                return;
            }
        }
        imageView.setVisibility(View.GONE);
    }
}
