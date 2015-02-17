LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_C_INCLUDES := \
	system/core/include/ \
	mediatek/hardware/gralloc_extra/include \

LOCAL_SRC_FILES:= \
	main.cpp \
	Frame.cpp \
	Action.cpp \
	Dump.cpp \
	TestSurface.cpp \
	App.cpp \
	Stretegy.cpp

LOCAL_SHARED_LIBRARIES := \
	libcutils \
	libutils \
	libbinder \
    libui \
    libgui

LOCAL_MODULE:= stresstool

LOCAL_MODULE_TAGS := tests

include $(BUILD_EXECUTABLE)
