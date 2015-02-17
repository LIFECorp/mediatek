

#define LOG_TAG "UibcMessage"
#include <utils/Log.h>

#include "UibcMessage.h"
#include "WifiDisplayUibcType.h"

#include <media/IRemoteDisplayClient.h>

#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/MediaErrors.h>
#include <media/stagefright/foundation/hexdump.h>


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <netinet/in.h>

#include <unistd.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <KeycodeLabels.h>

#include <ui/DisplayInfo.h>
#include <gui/ISurfaceComposer.h>
#include <gui/SurfaceComposerClient.h>

namespace android {

static const scanCodeBuild_t scanCode_DefaultMap[] = {
    {0x00}, //0x00	#	NULL
    {0x00}, //0x01	#	START OF HEADING
    {0x00}, //0x02	#	START OF TEXT
    {0x00}, //0x03	#	END OF TEXT
    {0x00}, //0x04	#	END OF TRANSMISSION
    {0x00}, //0x05	#	ENQUIRY
    {0x00}, //0x06	#	ACKNOWLEDGE
    {0x00}, //0x07	#	BELL
    {KEY_BACKSPACE}, //0x08	#	BACKSPACE
    {KEY_TAB}, //0x09	#	HORIZONTAL TABULATION
    {KEY_LINEFEED}, //0x0A	#	LINE FEED
    {KEY_TAB}, //0x0B	#	VERTICAL TABULATION
    {KEY_PAGEDOWN}, //0x0C	#	FORM FEED
    {KEY_ENTER}, //0x0D	#	CARRIAGE RETURN
    {0x00}, //0x0E	#	SHIFT OUT
    {KEY_LEFTSHIFT}, //0x0F	#	SHIFT IN
    {0x00}, //0x10	#	DATA LINK ESCAPE
    {KEY_LEFTCTRL}, //0x11	#	DEVICE CONTROL ONE
    {KEY_LEFTCTRL}, //0x12	#	DEVICE CONTROL TWO
    {KEY_LEFTCTRL}, //0x13	#	DEVICE CONTROL THREE
    {KEY_LEFTCTRL}, //0x14	#	DEVICE CONTROL FOUR
    {0x00}, //0x15	#	NEGATIVE ACKNOWLEDGE
    {0x00}, //0x16	#	SYNCHRONOUS IDLE
    {0x00}, //0x17	#	END OF TRANSMISSION BLOCK
    {KEY_CANCEL}, //0x18	#	CANCEL
    {0x00}, //0x19	#	END OF MEDIUM
    {0x00}, //0x1A	#	SUBSTITUTE
    {KEY_BACK}, //0x1B	#	ESCAPE
    {0x00}, //0x1C	#	FILE SEPARATOR
    {0x00}, //0x1D	#	GROUP SEPARATOR
    {0x00}, //0x1E	#	RECORD SEPARATOR
    {0x00}, //0x1F	#	UNIT SEPARATOR
};

#define UIBC_KEYCODE_UNKNOWN KEY_UNKNOWN

UibcMessage::UibcMessage(UibcMessage::MessageType type,
                         const char* inEventDesc,
                         double widthRatio,
                         double heightRatio)
    : m_PacketData(NULL),
      m_PacketDataLen(0),
      m_DataValid(false) {

    switch (type) {
    case GENERIC_TOUCH_DOWN:
    case GENERIC_TOUCH_UP:
    case GENERIC_TOUCH_MOVE:
        m_PacketDataLen = getUIBCGenericTouchPacket(inEventDesc,
                          &m_PacketData,
                          widthRatio,
                          heightRatio);
        break;

    case GENERIC_KEY_DOWN:
    case GENERIC_KEY_UP:
        m_PacketDataLen = getUIBCGenericKeyPacket(inEventDesc, &m_PacketData);
        break;

    case GENERIC_ZOOM:
        m_PacketDataLen = getUIBCGenericZoomPacket(inEventDesc, &m_PacketData);
        break;

    case GENERIC_VERTICAL_SCROLL:
    case GENERIC_HORIZONTAL_SCROLL:
        m_PacketDataLen = getUIBCGenericScalePacket(inEventDesc, &m_PacketData);
        break;

    case GENERIC_ROTATE:
        m_PacketDataLen = getUIBCGenericRotatePacket(inEventDesc, &m_PacketData);
        break;
    };
}

UibcMessage::~UibcMessage() {
    if (m_PacketData != NULL) {
        free(m_PacketData);
        m_PacketData = NULL;
        m_DataValid = false;
    }
}

status_t UibcMessage::init() {
    int version;
    return OK;
}

status_t UibcMessage::destroy() {

    return OK;
}

char* UibcMessage::getPacketData() {
    return m_PacketData;
}

int UibcMessage::getPacketDataLen() {
    return m_PacketDataLen;
}

bool UibcMessage::isDataValid() {
    return m_DataValid;
}


// format: "typeId, number of pointers, pointer Id1, X coordnate, Y coordnate, , pointer Id2, X coordnate, Y coordnate,..."
int32_t UibcMessage::getUIBCGenericTouchPacket(const char *inEventDesc,
        char** outData,
        double widthRatio,
        double heightRatio) {
    ALOGD("getUIBCGenericTouchPacket (%s)", inEventDesc);
    int32_t typeId = 0, numberOfPointers;
    int32_t uibcBodyLen = 0, genericPacketLen;
    int32_t pointerCounter, eventField;
    int32_t temp;

    char** splitedStr = UibcMessage::str_split((char*)inEventDesc, ',');

    if (splitedStr) {
        int i;
        for (i = 0; * (splitedStr + i); i++) {
            //ALOGD("getUIBCGenericTouchPacket splitedStr tokens=[%s]\n", *(splitedStr + i));

            switch (i) {
            case 0: {
                typeId = atoi(*(splitedStr + i));
                //ALOGD("getUIBCGenericTouchPacket typeId=[%d]\n", typeId);
                break;
            }
            case 1: {
                numberOfPointers = atoi(*(splitedStr + i));
                //ALOGD("getUIBCGenericTouchPacket numberOfPointers=[%d]\n", numberOfPointers);
                genericPacketLen = numberOfPointers * 5 + 1;
                uibcBodyLen = genericPacketLen + 7; // Generic herder leh = 7
                (*outData) = (char*)malloc(uibcBodyLen + 1);
                // UIBC header
                (*outData)[0] = 0x00; //Version (3 bits),T (1 bit),Reserved(8 bits),Input Category (4 bits)
                (*outData)[1] = 0x00; //Version (3 bits),T (1 bit),Reserved(8 bits),Input Category (4 bits)
                (*outData)[2] = (uibcBodyLen >> 8) & 0xFF; //Length(16 bits)
                (*outData)[3] = uibcBodyLen & 0xFF; //Length(16 bits)
                //Generic Input Body Format
                (*outData)[4] = typeId & 0xFF; // Tyoe ID, 1 octet
                (*outData)[5] = (genericPacketLen >> 8) & 0xFF; // Length, 2 octets
                (*outData)[6] = genericPacketLen & 0xFF; // Length, 2 octets
                (*outData)[7] = numberOfPointers & 0xFF; // Number of pointers, 1 octet
                break;
            }
            default: {
                pointerCounter = (i - 2) / 3; // (curIndex - 2 fields, id+numPointers) /  (3 fields, id+x+y)
                eventField = (i - 2) % 3; // 0 = id, 1 = x, 2 = y
                temp = atoi(*(splitedStr + i));
                switch (eventField) {
                case 0: { // Pointer ID, 1 octet
                    //ALOGD("getUIBCGenericTouchPacket Pointer ID=[%d]\n", temp);
                    (*outData)[8 + pointerCounter * 5 + eventField] = temp & 0xFF;
                    break;
                }
                case 1: { // X-coordinate, 2 octets
                    temp = (int32_t)((double)temp * widthRatio);
                    //ALOGD("getUIBCGenericTouchPacket X-coordinate=[%d]\n", temp);
                    (*outData)[8 + pointerCounter * 5 + eventField] = (temp >> 8) & 0xFF;
                    (*outData)[8 + pointerCounter * 5 + eventField + 1] = temp & 0xFF;
                    break;
                }
                case 2: { // Y-coordinate, 2 octets
                    temp = (int32_t)((double)temp * heightRatio);
                    //ALOGD("getUIBCGenericTouchPacket Y-coordinate=[%d]\n", temp);
                    (*outData)[8 + pointerCounter * 5 + eventField + 1] = (temp >> 8) & 0xFF;
                    (*outData)[8 + pointerCounter * 5 + eventField + 2] = temp & 0xFF;
                    break;
                }
                }
                break;
            }
            }

            free(*(splitedStr + i));
        }
        free(splitedStr);
    }
    hexdump((*outData), uibcBodyLen);
    m_DataValid = true;
    return uibcBodyLen;
}

// format: "typeId, Key code 1(0x00), Key code 2(0x00)"
int32_t UibcMessage::getUIBCGenericKeyPacket(const char *inEventDesc,
        char** outData) {
    ALOGD("getUIBCGenericKeyPacket (%s)", inEventDesc);
    int32_t typeId;
    int32_t uibcBodyLen, genericPacketLen;
    int32_t temp;

    char** splitedStr = UibcMessage::str_split((char*)inEventDesc, ',');

    if (splitedStr) {
        int i;
        for (i = 0; * (splitedStr + i); i++) {
            //ALOGD("getUIBCGenericKeyPacket splitedStr tokens=[%s]\n", *(splitedStr + i));

            switch (i) {
            case 0: {
                typeId = atoi(*(splitedStr + i));
                //ALOGD("getUIBCGenericKeyPacket typeId=[%d]\n", typeId);
                genericPacketLen = 5;
                uibcBodyLen = genericPacketLen + 7; // Generic herder leh = 7
                (*outData) = (char*)malloc(uibcBodyLen + 1);
                // UIBC header
                (*outData)[0] = 0x00; //Version (3 bits),T (1 bit),Reserved(8 bits),Input Category (4 bits)
                (*outData)[1] = 0x00; //Version (3 bits),T (1 bit),Reserved(8 bits),Input Category (4 bits)
                (*outData)[2] = (uibcBodyLen >> 8) & 0xFF; //Length(16 bits)
                (*outData)[3] = uibcBodyLen & 0xFF; //Length(16 bits)
                //Generic Input Body Format
                (*outData)[4] = typeId & 0xFF; // Tyoe ID, 1 octet
                (*outData)[5] = (genericPacketLen >> 8) & 0xFF; // Length, 2 octets
                (*outData)[6] = genericPacketLen & 0xFF; // Length, 2 octets
                (*outData)[7] = 0x00; // resvered
                break;
            }
            case 1: {
                sscanf(*(splitedStr + i), " 0x%04X", &temp);
                if (temp == 0) {
                    uibcBodyLen = 0;
                    (*outData)[8] = 0x00;
                    (*outData)[9] = 0x00;
                }
                //ALOGD("getUIBCGenericKeyPacket key code 1=[%d]\n", temp);
                (*outData)[8] = (temp >> 8) & 0xFF;
                (*outData)[9] = temp & 0xFF;

                break;
            }
            case 2: {
                sscanf(*(splitedStr + i), " 0x%04X", &temp);
                if (temp == 0) {
                    (*outData)[10] = 0x00;
                    (*outData)[11] = 0x00;
                }
                //ALOGD("getUIBCGenericKeyPacket key code 2=[%d]\n", temp);
                (*outData)[10] = (temp >> 8) & 0xFF;
                (*outData)[11] = temp & 0xFF;

                break;
            }
            default: {
            }
            break;
            }
        }

        free(*(splitedStr + i));
    }
    free(splitedStr);
    hexdump((*outData), uibcBodyLen);
    m_DataValid = true;
    return uibcBodyLen;
}

// format: "typeId,  X coordnate, Y coordnate, integer part, fraction part"
int32_t UibcMessage::getUIBCGenericZoomPacket(const char *inEventDesc, char** outData) {
    ALOGD("getUIBCGenericZoomPacket (%s)", inEventDesc);
    int32_t typeId;
    int32_t uibcBodyLen, genericPacketLen;
    int32_t eventField;
    int32_t xCoord, yCoord, integerPart, FractionPart;

    char** splitedStr = UibcMessage::str_split((char*)inEventDesc, ',');

    if (splitedStr) {
        int i;
        for (i = 0; * (splitedStr + i); i++) {
            //ALOGD("getUIBCGenericZoomPacket splitedStr tokens=[%s]\n", *(splitedStr + i));

            switch (i) {
            case 0: {
                typeId = atoi(*(splitedStr + i));
                //ALOGD("getUIBCGenericZoomPacket typeId=[%d]\n", typeId);

                genericPacketLen = 6;
                uibcBodyLen = genericPacketLen + 7; // Generic herder leh = 7
                (*outData) = (char*)malloc(uibcBodyLen + 1);
                // UIBC header
                (*outData)[0] = 0x00; //Version (3 bits),T (1 bit),Reserved(8 bits),Input Category (4 bits)
                (*outData)[1] = 0x00; //Version (3 bits),T (1 bit),Reserved(8 bits),Input Category (4 bits)
                (*outData)[2] = (uibcBodyLen >> 8) & 0xFF; //Length(16 bits)
                (*outData)[3] = uibcBodyLen & 0xFF; //Length(16 bits)
                //Generic Input Body Format
                (*outData)[4] = typeId & 0xFF; // Tyoe ID, 1 octet
                (*outData)[5] = (genericPacketLen >> 8) & 0xFF; // Length, 2 octets
                (*outData)[6] = genericPacketLen & 0xFF; // Length, 2 octets
                break;
            }

            case 1: {
                xCoord = atoi(*(splitedStr + i));
                (*outData)[7] = (xCoord >> 8) & 0xFF;
                (*outData)[8] = xCoord & 0xFF;
                //ALOGD("getUIBCGenericZoomPacket xCoord=[%d]\n", xCoord);
                break;
            }
            case 2: {
                yCoord = atoi(*(splitedStr + i));
                //ALOGD("getUIBCGenericZoomPacket yCoord=[%d]\n", yCoord);
                break;
            }
            case 3: {
                integerPart = atoi(*(splitedStr + i));
                (*outData)[11] = integerPart & 0xFF;
                //ALOGD("getUIBCGenericZoomPacket integerPart=[%d]\n", integerPart);
                break;
            }
            case 4: {
                FractionPart = atoi(*(splitedStr + i));
                (*outData)[12] = FractionPart & 0xFF;
                //ALOGD("getUIBCGenericZoomPacket FractionPart=[%d]\n", FractionPart);

                break;
            }
            default: {
                break;
            }
            }

            free(*(splitedStr + i));
        }
        free(splitedStr);
    }
    hexdump((*outData), uibcBodyLen);
    m_DataValid = true;
    return uibcBodyLen;
}

// format: "typeId,  unit, direction, amount to scroll"
int32_t UibcMessage::getUIBCGenericScalePacket(const char *inEventDesc, char** outData) {
    ALOGD("getUIBCGenericScalePacket (%s)", inEventDesc);
    int32_t typeId;
    int32_t uibcBodyLen, genericPacketLen;
    int32_t temp;

    char** splitedStr = UibcMessage::str_split((char*)inEventDesc, ',');

    if (splitedStr) {
        int i;
        for (i = 0; * (splitedStr + i); i++) {
            //ALOGD("getUIBCGenericScalePacket splitedStr tokens=[%s]\n", *(splitedStr + i));

            switch (i) {
            case 0: {
                typeId = atoi(*(splitedStr + i));
                //ALOGD("getUIBCGenericScalePacket typeId=[%d]\n", typeId);
                genericPacketLen = 2;
                uibcBodyLen = genericPacketLen + 7; // Generic herder leh = 7
                (*outData) = (char*)malloc(uibcBodyLen + 1);
                // UIBC header
                (*outData)[0] = 0x00; //Version (3 bits),T (1 bit),Reserved(8 bits),Input Category (4 bits)
                (*outData)[1] = 0x00; //Version (3 bits),T (1 bit),Reserved(8 bits),Input Category (4 bits)
                (*outData)[2] = (uibcBodyLen >> 8) & 0xFF; //Length(16 bits)
                (*outData)[3] = uibcBodyLen & 0xFF; //Length(16 bits)
                //Generic Input Body Format
                (*outData)[4] = typeId & 0xFF; // Tyoe ID, 1 octet
                (*outData)[5] = (genericPacketLen >> 8) & 0xFF; // Length, 2 octets
                (*outData)[6] = genericPacketLen & 0xFF; // Length, 2 octets
                (*outData)[7] = 0x00; // Clear the byte
                (*outData)[8] = 0x00; // Clear the byte
                /*
                B15B14; Scroll Unit Indication bits.
                0b00; the unit is a pixel (normalized with respect to the WFD Source display resolution that is conveyed in an RTSP M4 request message).
                0b01; the unit is a mouse notch (where the application is responsible for representing the number of pixels per notch).
                0b10-0b11; Reserved.

                B13; Scroll Direction Indication bit.
                0b0; Scrolling to the right. Scrolling to the right means the displayed content being shifted to the left from a user perspective.
                0b1; Scrolling to the left. Scrolling to the left means the displayed content being shifted to the right from a user perspective.

                B12:B0; Number of Scroll bits.
                Number of units for a Horizontal scroll.
                */
                break;
            }
            case 1: {
                temp = atoi(*(splitedStr + i));
                //ALOGD("getUIBCGenericScalePacket unit=[%d]\n", temp);
                (*outData)[7] = (temp >> 8) & 0xFF;
                break;
            }
            case 2: {
                temp = atoi(*(splitedStr + i));
                //ALOGD("getUIBCGenericScalePacket direction=[%d]\n", temp);
                (*outData)[7] |= ((temp >> 10) & 0xFF);
                break;

            }
            case 3: {
                temp = atoi(*(splitedStr + i));
                //ALOGD("getUIBCGenericScalePacket amount to scroll=[%d]\n", temp);
                (*outData)[7] |= ((temp >> 12) & 0xFF);
                (*outData)[8] = temp & 0xFF;

                break;
            }
            default: {
                break;
            }
            }

            free(*(splitedStr + i));
        }

        free(splitedStr);
    }
    hexdump((*outData), uibcBodyLen);
    m_DataValid = true;
    return uibcBodyLen;
}

// format: "typeId,  integer part, fraction part"
int32_t UibcMessage::getUIBCGenericRotatePacket(const char * inEventDesc, char** outData) {
    ALOGD("getUIBCGenericRotatePacket (%s)", inEventDesc);
    int32_t typeId;
    int32_t uibcBodyLen, genericPacketLen;
    int32_t temp;
    int32_t integerPart, FractionPart;

    char** splitedStr = UibcMessage::str_split((char*)inEventDesc, ',');

    if (splitedStr) {
        int i;
        for (i = 0; * (splitedStr + i); i++) {
            //ALOGD("getUIBCGenericRotatePacket splitedStr tokens=[%s]\n", *(splitedStr + i));

            switch (i) {
            case 0: {
                typeId = atoi(*(splitedStr + i));
                //ALOGD("getUIBCGenericRotatePacket typeId=[%d]\n", typeId);
                genericPacketLen = 2;
                uibcBodyLen = genericPacketLen + 7; // Generic herder leh = 7
                (*outData) = (char*)malloc(uibcBodyLen + 1);
                // UIBC header
                (*outData)[0] = 0x00; //Version (3 bits),T (1 bit),Reserved(8 bits),Input Category (4 bits)
                (*outData)[1] = 0x00; //Version (3 bits),T (1 bit),Reserved(8 bits),Input Category (4 bits)
                (*outData)[2] = (uibcBodyLen >> 8) & 0xFF; //Length(16 bits)
                (*outData)[3] = uibcBodyLen & 0xFF; //Length(16 bits)
                //Generic Input Body Format
                (*outData)[4] = typeId & 0xFF; // Tyoe ID, 1 octet
                (*outData)[5] = (genericPacketLen >> 8) & 0xFF; // Length, 2 octets
                (*outData)[6] = genericPacketLen & 0xFF; // Length, 2 octets
                break;
            }
            case 1: {
                integerPart = atoi(*(splitedStr + i));
                (*outData)[7] = integerPart & 0xFF;
                //ALOGD("getUIBCGenericRotatePacket integerPart=[%d]\n", integerPart);
                break;
            }
            case 2: {
                FractionPart = atoi(*(splitedStr + i));
                (*outData)[8] = FractionPart & 0xFF;
                //ALOGD("getUIBCGenericRotatePacket FractionPart=[%d]\n", FractionPart);

                break;
            }
            default: {
                break;
            }
            }
            free(*(splitedStr + i));
        }

        free(splitedStr);
    }
    hexdump((*outData), uibcBodyLen);
    m_DataValid = true;
    return uibcBodyLen;
}


//static
scanCodeBuild_t UibcMessage::asciiToScancodeBuild(UINT16 asciiCode) {
    scanCodeBuild_t ret = scanCode_DefaultMap[0];

    ALOGD("asciiCode: %d", asciiCode);

    ret = scanCode_DefaultMap[asciiCode];

    ALOGD("scanCode: %d", ret.scanCode);
    return ret;
}

//static
short UibcMessage::scancodeToAcsii(UINT8 scanCode) {
    short ret = UIBC_KEYCODE_UNKNOWN;

    ALOGD("scanCode : %d", scanCode);

    for (unsigned int i = 0; i < (sizeof(scanCode_DefaultMap) / sizeof(scanCode_DefaultMap[0])); i++) {
        if (scanCode == scanCode_DefaultMap[i].scanCode) {
            ret = i;
            break;
        }
    }

    ALOGD("asciiCode: %d", ret);
    return ret;
}

//static
void UibcMessage::getScreenResolution(int* x, int* y) {
    sp<IBinder> display = SurfaceComposerClient::getBuiltInDisplay(
                              ISurfaceComposer::eDisplayIdMain);
    DisplayInfo info;
    SurfaceComposerClient::getDisplayInfo(display, &info);
    *x = info.w;
    *y = info.h;
}

char** UibcMessage::str_split(char * a_str, const char a_delim) {
    char** result    = 0;
    size_t count     = 0;
    char* tmp        = a_str;
    char* last_comma = 0;

    /* Count how many elements will be extracted. */
    while (*tmp) {
        if (a_delim == *tmp) {
            count++;
            last_comma = tmp;
        }
        tmp++;
    }

    /* Add space for trailing token. */
    count += last_comma < (a_str + strlen(a_str) - 1);

    /* Add space for terminating null string so caller
       knows where the list of returned strings ends. */
    count++;

    result = (char**)malloc(sizeof(char*) * count);

    if (result) {
        size_t idx  = 0;
        char* token = strtok(a_str, ",");

        while (token) {
            CHECK_LT(idx , count);
            *(result + idx++) = strdup(token);
            token = strtok(0, ",");
        }
        CHECK_EQ(idx , count - 1);
        *(result + idx) = 0;
    }

    return result;
}

}
