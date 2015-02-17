package com.mediatek.op01.tests;

import android.app.Instrumentation;

import android.content.ContentValues;
import android.content.Context;
import android.net.Uri;
import android.provider.Telephony.Sms.Inbox;
import android.telephony.SmsMessage;
//import android.telephony.gemini.GeminiSmsMessage;
import com.android.internal.telephony.GeminiSmsMessage;
import android.test.InstrumentationTestCase;
import com.android.internal.telephony.SmsHeader;

import com.mediatek.mms.op01.Op01SmsReceiverExt;
import com.mediatek.pluginmanager.PluginManager;
import com.mediatek.xlog.Xlog;

public class Op01SmsReceiverExtTest extends InstrumentationTestCase {
    private static final String TAG = "SmsReceiver test";
    
    private Context mContext = null;
    private Instrumentation mInst = null;
    private Op01SmsReceiverExt mPlugin = null;
    private Uri mUri = null;
    
    @Override
    protected void setUp() throws Exception {
        Xlog.d(TAG, "setUp");
        super.setUp();
        
        mContext = this.getInstrumentation().getContext();
        mInst = this.getInstrumentation();
        mPlugin = 
            (Op01SmsReceiverExt)PluginManager.createPluginObject(mContext, "com.mediatek.mms.ext.ISmsReceiver");

        if (mPlugin == null) {
            Xlog.d(TAG, "get plugin failed");
        }

        //genTestSms();
    }
    
    @Override    
    protected void tearDown() throws Exception {
        Xlog.d(TAG, "tearDown");
        super.tearDown();
        //clearTestSms();
    }

    public void testNormalSMS() {
        Xlog.d(TAG, "testNormalSMS");
        SmsMessage[] msgs = genNormalSMS();
        SmsMessage sms = msgs[0];
        ContentValues values = new ContentValues(1);
        
        mPlugin.extractSmsBody(msgs, sms, values);

        String body = values.getAsString(Inbox.BODY);
        if (body != null) {
            Xlog.d(TAG, "body =" + body);
        }
        
        assertEquals(true, (body != null));
    }

    private SmsMessage[] genNormalSMS() {
        String pduStr = "0891683108707515F0240D91685106037200F300002111021" + 
            "11582233C31D98C56B3DD7039584C36A3D56C375C0E1693CD6835DB0D9783" + 
            "C564335ACD76C3E56031D98C56B3DD7039584C36A3D56C375C0E06";
        byte[] pdu = hexStringToByteArray(pduStr);
        String format = "3gpp";
        int simId = 1;
        SmsMessage[] msgs = new SmsMessage[1];
        msgs[0] = GeminiSmsMessage.createFromPdu(pdu, format, simId);
        return msgs;
    }

    public void testLongSMS() {
        Xlog.d(TAG, "testLongSMS");
        SmsMessage[] msgs = genLongSMS();
        SmsMessage sms = msgs[0];
        ContentValues values = new ContentValues(1);
        
        mPlugin.extractSmsBody(msgs, sms, values);

        String body = values.getAsString(Inbox.BODY);
        if (body != null) {
            Xlog.d(TAG, "body =" + body);
        }
        
        assertEquals(true, (body != null));
    }

    private SmsMessage[] genLongSMS() {
        String pduStr0 = "0891683108707515F0640D91685106037200F30000211102" + 
            "11157023A005000307020162B219AD66BBE172B0986C46ABD96EB81C2C269BD16AB61" + 
            "B2E078BC966B49AED86CBC162B219AD66BBE172B0986C46ABD96EB81C2C269BD16A8B" + 
            "C966B49AED86CBC162B219AD66BBE172B0986C46ABD96EB81C2C269BD16AB61B2E078" + 
            "BED86CBC162B219AD66BBE172ABD96EB81C2C269BD16AB61B2E078BC966";
        String pduStr1 = "0891683108707515F0640D91685106037200F30000211102" + 
            "11156123220500030702026835DB0D9783C564335ACD76C3E56031D98C56B3DD703918";
        byte[] pdu0 = hexStringToByteArray(pduStr0);
        byte[] pdu1 = hexStringToByteArray(pduStr1);
        String format = "3gpp";
        int simId = 1;
        SmsMessage[] msgs = new SmsMessage[2];
        msgs[0] = GeminiSmsMessage.createFromPdu(pdu0, format, simId);
        msgs[1] = GeminiSmsMessage.createFromPdu(pdu1, format, simId);
        return msgs;
    }

    public void testMissSegments() {
        Xlog.d(TAG, "testMissSegments");

        SmsMessage[] msgs = genMissSegmentsSMS();
        SmsMessage sms = msgs[0];
        ContentValues values = new ContentValues(1);
        
        mPlugin.extractSmsBody(msgs, sms, values);

        String body = values.getAsString(Inbox.BODY);
        if (body != null) {
            Xlog.d(TAG, "body =" + body);
        }
        
        assertEquals(true, (body != null));
    }

    private SmsMessage[] genMissSegmentsSMS() {
        String pduStr0 = "0891683108707515F0600D91685106037200F30000211102" + 
            "61337023A005000312030162B219AD66BBE172B0986C46ABD96EB81C2C269" + 
            "BD16AB61B2E078BC966B49AED86CBC162B219AD66BBE172B0986C46ABD96E" + 
            "B81C2C269BD16AB61B2E078BC966B49AED86CBC162B219AD66BBE172B0986" + 
            "C46ABD96EB81C2C269BD16AB61B2E078BC966B49AED86CBC162B219AD66BB" + 
            "E172B0986C46ABD96EB81C2C269BD16AB61B2E078BC966";
        String pduStr1 = "0891683108707515F0600D91685106037200F300002111" + 
            "0261333123A00500031203026835DB0D9783C564335ACD76C3E56031D98" + 
            "C56B3DD7039584C36A3D56C375C0E1693CD6835DB0D9783C564335ACD76" + 
            "C3E56031D98C56B3DD7039584C36A3D56C375C0E1693CD6835DB0D9783C" + 
            "564335ACD76C3E56031D98C56B3DD7039584C36A3D56C375C0E1693CD68" + 
            "35DB0D9783C564335ACD76C3E56031D98C56B3DD7039584C36A3D56C";
        byte[] pdu0 = hexStringToByteArray(pduStr0);
        byte[] pdu1 = hexStringToByteArray(pduStr1);
        String format = "3gpp";
        int simId = 1;
        SmsMessage[] msgs = new SmsMessage[2];
        msgs[0] = GeminiSmsMessage.createFromPdu(pdu0, format, simId);
        msgs[1] = GeminiSmsMessage.createFromPdu(pdu1, format, simId);
        return msgs;
    }

    private static final char[] HEX_DIGITS = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
            'A', 'B', 'C', 'D', 'E', 'F' };
    
    private static String toHexString(byte[] array) {
        int length = array.length;
        char[] buf = new char[length * 2];

        int bufIndex = 0;
        for (int i = 0 ; i < length; i++) {
            byte b = array[i];
            buf[bufIndex++] = HEX_DIGITS[(b >>> 4) & 0x0F];
            buf[bufIndex++] = HEX_DIGITS[b & 0x0F];
        }

        return new String(buf);
    }

    private static int toByte(char c) {
        if (c >= '0' && c <= '9') {
            return (c - '0');
        }
        if (c >= 'A' && c <= 'F') {
            return (c - 'A' + 10);
        }
        if (c >= 'a' && c <= 'f') {
            return (c - 'a' + 10);
        }

        throw new RuntimeException("Invalid hex char '" + c + "'");
    }

    private static byte[] hexStringToByteArray(String hexString) {
        int length = hexString.length();
        byte[] buffer = new byte[length / 2];

        for (int i = 0 ; i < length ; i += 2) {
            buffer[i / 2] =
                (byte)((toByte(hexString.charAt(i)) << 4) | toByte(hexString.charAt(i + 1)));
        }

        return buffer;
    }

    private class SmsMessageTest extends SmsMessage {
        private SmsHeader mTestHeader;
        
        public SmsMessageTest() {
            super();
            mTestHeader = new SmsHeader();
            mTestHeader.concatRef = new SmsHeader.ConcatRef();
            mTestHeader.concatRef.msgCount = 0;
        }
        public SmsHeader getUserDataHeader() {
            return mTestHeader;
        }
    }
}

