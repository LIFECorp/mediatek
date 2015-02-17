package com.hissage.util.http;

import java.io.BufferedInputStream;
import java.io.File;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.RandomAccessFile;
import java.net.HttpURLConnection;
import java.net.URL;

import org.apache.http.HttpResponse;
import org.apache.http.client.HttpClient;
import org.apache.http.client.methods.HttpGet;
import org.apache.http.client.methods.HttpPost;
import org.apache.http.client.methods.HttpUriRequest;
import org.apache.http.entity.StringEntity;
import org.apache.http.impl.client.DefaultHttpClient;
import org.apache.http.params.CoreConnectionPNames;
import org.apache.http.protocol.HTTP;
import org.apache.http.util.EntityUtils;

import com.hissage.util.log.NmsLog;

import android.os.AsyncTask;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.text.TextUtils;

public final class NmsHttpTask {

    private static final String LOG_TAG = "NmsHttpTask";

    private static final int GENERAL_ERROR = -1;
    private static final int FILE_OPER_ERROR = -2;
    private static final int TASK_CANCELED = -3;

    private static final int SLEEP_TIME_BEFORE_RUN_TASK = 1000;

    private Handler mHandler = null;

    private HttpWorkerInterface mHttpWorker = null;

    private HttpTaskListener mListener = null;

    private Object mSaveClientParam = null;
    private String mUrl = null;
    private String mPostData = null;
    private String mDownloadedFileName = null;
    private int mTryCount = 1;
    private int mConnectTimeout = 15 * 1000;
    private int mTransferTimeout = 15 * 1000;
    private boolean mIsSupportResumeDL = false;
    private boolean mIsGZipDownload = true;

    private int mResultCode = GENERAL_ERROR;
    private int mResultLength = 0;
    private String mResultString = null;

    private interface HttpWorkerInterface {

        public boolean startTask();

        public void update(int bytes);

        public boolean stopTask();
    }

    public interface HttpTaskListener {

        public void onProgress(NmsHttpTask httpTask, int downloadByte);

        public void onResult(NmsHttpTask httpTask);
    }

    private final class HttpAsyncTask extends AsyncTask<Integer, Integer, Integer> implements
            HttpWorkerInterface {

        @Override
        protected Integer doInBackground(Integer... params) {
            if (params == null) {
                NmsLog.error(LOG_TAG, "params is null");
                return null;
            }

            try {
                Thread.sleep(SLEEP_TIME_BEFORE_RUN_TASK);
            } catch (Exception e) {
                NmsLog.nmsPrintStackTraceByTag(LOG_TAG, e);
            }

            if (TextUtils.isEmpty(getDownLoadedFileName()))
                doHttpFetch();
            else
                doHttpFetchFile();

            return null;
        }

        @Override
        protected void onProgressUpdate(Integer... values) {
            if (mListener != null) {
                try {
                    mListener.onProgress(NmsHttpTask.this, values[0]);
                } catch (Exception e) {
                    NmsLog.nmsPrintStackTraceByTag(LOG_TAG, e);
                }
            }
        }

        @Override
        protected void onPostExecute(Integer result) {
            if (mListener != null) {
                try {
                    mListener.onResult(NmsHttpTask.this);
                } catch (Exception e) {
                    NmsLog.nmsPrintStackTraceByTag(LOG_TAG, e);
                }
            }
        }

        @Override
        public boolean startTask() {
            execute(-1);
            return false;
        }

        @Override
        public void update(int bytes) {
            publishProgress(bytes);
        }

        @Override
        public boolean stopTask() {
            return cancel(true);
        }

    }

    private final class HttpThread extends Thread implements HttpWorkerInterface {

        private final static int UPDATEED_MSG = 0;
        private final static int FINISHED_MSG = 1;

        @Override
        public void run() {

            if (mHandler == null) {
                synchronized (HttpThread.class) {
                    if (mHandler == null) {
                        mHandler = new Handler(Looper.getMainLooper()) {
                            public void handleMessage(Message msg) {

                                switch (msg.what) {

                                case UPDATEED_MSG:
                                    try {
                                        mListener.onProgress(NmsHttpTask.this, msg.arg1);
                                    } catch (Exception e) {
                                        NmsLog.nmsPrintStackTraceByTag(LOG_TAG, e);
                                    }
                                    break;

                                case FINISHED_MSG:
                                    try {
                                        mListener.onResult(NmsHttpTask.this);
                                    } catch (Exception e) {
                                        NmsLog.nmsPrintStackTraceByTag(LOG_TAG, e);
                                    }

                                    break;

                                default:
                                    NmsLog.error(LOG_TAG, "http thread got error msg id: "
                                            + msg.what);
                                    break;
                                }
                            }
                        };
                    }
                }
            }

            try {
                sleep(SLEEP_TIME_BEFORE_RUN_TASK);
            } catch (Exception e) {
                NmsLog.nmsPrintStackTraceByTag(LOG_TAG, e);
            }

            if (TextUtils.isEmpty(getDownLoadedFileName()))
                doHttpFetch();
            else
                doHttpFetchFile();

            if (mListener != null)
                mHandler.sendEmptyMessage(FINISHED_MSG);
        }

        @Override
        public boolean startTask() {
            try {
                start();
                return true;
            } catch (Exception e) {
                NmsLog.nmsPrintStackTraceByTag(LOG_TAG, e);
                return false;
            }
        }

        @Override
        public void update(int bytes) {
            if (mListener != null) {
                Message msg = new Message();
                msg.what = UPDATEED_MSG;
                msg.arg1 = bytes;
                mHandler.sendMessage(msg);
            }
        }

        @Override
        public boolean stopTask() {
            try {
                interrupt();
                return true;
            } catch (Exception e) {
                NmsLog.nmsPrintStackTraceByTag(LOG_TAG, e);
                return false;
            }
        }
    }

    public NmsHttpTask(HttpTaskListener listener, int tryCount, String url, String postData,
            String downloadedFileName) {
        mListener = listener;
        mUrl = url;
        mPostData = postData;
        mDownloadedFileName = downloadedFileName;
        mTryCount = tryCount;

        if (!url.toLowerCase().startsWith("http")) {
            mUrl = "http://" + url;
        }

        if (mTryCount <= 0) {
            NmsLog.warn(LOG_TAG, String.format("tryCount: %d is <= 0", mTryCount));
            mTryCount = 1;
        }
    }

    /*
     * // global thread safe http client public static synchronized HttpClient
     * createHttpClient() {
     * 
     * if (mHttpClient != null) return mHttpClient ;
     * 
     * HttpParams params = new BasicHttpParams();
     * HttpProtocolParams.setVersion(params, HttpVersion.HTTP_1_1);
     * HttpProtocolParams.setContentCharset(params,
     * HTTP.DEFAULT_CONTENT_CHARSET);
     * HttpProtocolParams.setUseExpectContinue(params, true);
     * HttpProtocolParams.setUserAgent(params, "Android_ISMS");
     * 
     * ConnManagerParams.setTimeout(params, 5000);
     * HttpConnectionParams.setConnectionTimeout(params, 5000);
     * HttpConnectionParams.setSoTimeout(params, 5000);
     * 
     * SchemeRegistry schReg = new SchemeRegistry(); schReg.register(new
     * Scheme("http", PlainSocketFactory.getSocketFactory(), 80));
     * schReg.register(new Scheme("https", SSLSocketFactory.getSocketFactory(),
     * 443)); ClientConnectionManager conMgr = new
     * ThreadSafeClientConnManager(params,schReg);
     * 
     * mHttpClient = new DefaultHttpClient(conMgr, params); return mHttpClient ;
     * }
     */

    private void doHttpFetch() {

        for (int i = 0; i < mTryCount; i++) {

            try {

                NmsLog.trace(LOG_TAG,
                        String.format("fetch task is started, url: %s, tryTime: %d", getUrl(), i));

                if (isTaskCanceled()) {
                    NmsLog.trace(LOG_TAG, String.format(
                            "fetch task is cancel before request, url: %s, tryTime: %d", getUrl(),
                            i));
                    return;
                }

                HttpUriRequest httpReq = null;

                if (mPostData == null) {
                    httpReq = new HttpGet(getUrl());
                } else {
                    HttpPost httpPost = new HttpPost(getUrl());
                    StringEntity entity = new StringEntity(mPostData, HTTP.UTF_8);
                    httpPost.setEntity(entity);
                    httpReq = httpPost;
                }

                HttpClient httpClient = new DefaultHttpClient();
                httpClient.getParams().setParameter(CoreConnectionPNames.CONNECTION_TIMEOUT,
                        mConnectTimeout);
                httpClient.getParams().setParameter(CoreConnectionPNames.SO_TIMEOUT,
                        mTransferTimeout);

                HttpResponse httpResp = httpClient.execute(httpReq);

                if (isTaskCanceled()) {
                    NmsLog.trace(
                            LOG_TAG,
                            String.format(
                                    "fetch task is cancel after request, url: %s, tryTime: %d, statuCode: %d",
                                    getUrl(), i, httpResp.getStatusLine().getStatusCode()));
                    return;
                }

                mResultCode = httpResp.getStatusLine().getStatusCode();

                if (isTaskSucceed()) {
                    mResultString = EntityUtils.toString(httpResp.getEntity(), HTTP.UTF_8);
                    if (mResultString == null)
                        mResultString = "";
                    mResultLength = mResultString.length();
                    mHttpWorker.update(mResultLength);
                    break;
                }

                NmsLog.error(LOG_TAG, String.format(
                        "fetch task is failed, url: %s, tryTime: %d, statuCode: %d", getUrl(), i,
                        mResultCode));
            } catch (Exception e) {
                mResultCode = GENERAL_ERROR;
                NmsLog.nmsPrintStackTraceByTag(LOG_TAG, e);
            }
        }
    }

    private void doHttpFetchFile() {
        for (int i = 0; i < mTryCount; i++) {

            try {
                NmsLog.trace(LOG_TAG, String.format(
                        "file task is started, url: %s, file: %s, tryTime: %d", getUrl(),
                        getDownLoadedFileName(), i));

                if (isTaskCanceled()) {
                    NmsLog.trace(LOG_TAG, String.format(
                            "file task is cancel before request, url: %s, file: %s, tryTime: %d",
                            getUrl(), getDownLoadedFileName(), i));
                    return;
                }

                HttpURLConnection httpConn = null;
                URL url = new URL(getUrl());
                httpConn = (HttpURLConnection) url.openConnection();

                httpConn.setReadTimeout(mConnectTimeout);
                httpConn.setConnectTimeout(mTransferTimeout);
                httpConn.setInstanceFollowRedirects(true);
                httpConn.setUseCaches(false);

                File file = new File(getDownLoadedFileName());
                int curLen = 0;

                if (mIsSupportResumeDL && file.exists()) {
                    curLen = (int) file.length();
                    if (curLen > 0) {
                        String start = "bytes=" + curLen + "-";
                        httpConn.setRequestProperty("Range", start);
                    }
                } else {
                    if (file.exists()) {
                        if (!file.delete()) {
                            mResultCode = FILE_OPER_ERROR;
                            NmsLog.error(LOG_TAG, String.format(
                                    "file task is failed for delete file error, url: %s, file: %s",
                                    getUrl(), getDownLoadedFileName()));
                            return;
                        }
                    } else {
                        if (!file.createNewFile()) {
                            mResultCode = FILE_OPER_ERROR;
                            NmsLog.error(LOG_TAG, String.format(
                                    "file task is failed for create file error, url: %s, file: %s",
                                    getUrl(), getDownLoadedFileName()));
                            return;
                        }
                    }
                }

                if (mPostData == null) {
                    httpConn.setRequestMethod("GET");
                    if (!mIsGZipDownload)
                        httpConn.setRequestProperty("Accept-Encoding", "identity");
                } else {
                    httpConn.setDoOutput(true);
                    httpConn.setRequestMethod("POST");
                    httpConn.setRequestProperty("Content-Type", "text/plain");
                    if (!mIsGZipDownload)
                        httpConn.setRequestProperty("Accept-Encoding", "identity");
                    OutputStream outputStream = httpConn.getOutputStream();
                    outputStream.write(mPostData.getBytes("UTF-8"));
                    outputStream.flush();
                    outputStream.close();
                }

                if (isTaskCanceled()) {
                    NmsLog.trace(LOG_TAG, String.format(
                            "file task is cancel after request, url: %s, file: %s, tryTime: %d",
                            getUrl(), getDownLoadedFileName(), i));
                    return;
                }

                mResultCode = httpConn.getResponseCode();

                if (isTaskSucceed()) {
                    InputStream inStream = httpConn.getInputStream();
                    RandomAccessFile randomFile = new RandomAccessFile(file, "rw");
                    BufferedInputStream bufferStream = new BufferedInputStream(inStream);

                    randomFile.seek(curLen);

                    int httpContentLen = httpConn.getContentLength();
                    int recvFileLen = curLen;

                    NmsLog.trace(LOG_TAG,
                            String.format("file task get content length: %d", httpContentLen));

                    if (httpContentLen != -1)
                        mResultLength = curLen + httpContentLen;
                    else
                        mResultLength = -1;

                    if (recvFileLen > 0)
                        mHttpWorker.update(recvFileLen);

                    byte[] buffer = new byte[1024 * 20];
                    int len = -1;
                    while ((len = bufferStream.read(buffer)) != -1) {
                        randomFile.write(buffer, 0, len);
                        recvFileLen += len;
                        mHttpWorker.update(recvFileLen);
                    }

                    randomFile.close();
                    bufferStream.close();
                    inStream.close();

                    mResultLength = (int) file.length();

                    if (mResultLength != recvFileLen) {
                        mResultCode = GENERAL_ERROR;
                        NmsLog.error(
                                LOG_TAG,
                                String.format(
                                        "file task is failed for file leng is not match, url: %s, file: %s, fileLen: %d, recvFileLen: %d",
                                        getUrl(), getDownLoadedFileName(), mResultCode, recvFileLen));
                    }
                }

                httpConn.disconnect();

                if (isTaskSucceed())
                    return;

            } catch (Exception e) {
                mResultCode = GENERAL_ERROR;
                NmsLog.nmsPrintStackTraceByTag(LOG_TAG, e);
            }
        }
    }

    public boolean startTask(boolean runAsAsyncTask) {

        if (TextUtils.isEmpty(getUrl())) {
            NmsLog.error(LOG_TAG, "url is invalid");
            return false;
        }

        if (runAsAsyncTask)
            mHttpWorker = new HttpAsyncTask();
        else
            mHttpWorker = new HttpThread();

        return mHttpWorker.startTask();
    }

    public boolean stopTask() {
        mResultCode = TASK_CANCELED;
        return mHttpWorker.stopTask();
    }

    public String getUrl() {
        return mUrl;
    }

    public String getDownLoadedFileName() {
        return mDownloadedFileName;
    }

    public void setConnectTimeout(int millseconds) {
        if (millseconds > 0)
            mConnectTimeout = millseconds;
    }

    public int getConnectTimeout() {
        return mConnectTimeout;
    }

    public void setTransferTimeout(int millseconds) {
        if (millseconds > 0)
            mTransferTimeout = millseconds;
    }

    public int getTransferTimeout() {
        return mTransferTimeout;
    }

    public void setClientParam(Object saveParam) {
        mSaveClientParam = saveParam;
    }

    public Object getClientParam() {
        return mSaveClientParam;
    }

    public void setSupportResumeDownload(boolean isSupport) {
        mIsSupportResumeDL = isSupport;
    }

    public void setIsGZipDownload(boolean isGZipDownload) {
        mIsGZipDownload = isGZipDownload;
    }

    public boolean isTaskSucceed() {
        boolean ret = (mResultCode == 200) || (mResultCode == 204) || (mResultCode == 206);
        if (!ret)
            NmsLog.error(LOG_TAG, "got error code: " + mResultCode);

        return ret;
    }

    public boolean isTaskCanceled() {
        return (mResultCode == TASK_CANCELED);
    }

    public String getResultString() {
        return mResultString;
    }

    public int getResultLength() {
        return mResultLength;
    }

}
