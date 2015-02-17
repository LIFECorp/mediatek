LOCAL_PATH:= $(call my-dir)

mcld_script_SRC_FILES := \
  AssertCmd.cpp \
  Assignment.cpp \
  BinaryOp.cpp \
  EntryCmd.cpp \
  FileToken.cpp \
  GroupCmd.cpp \
  InputSectDesc.cpp \
  InputToken.cpp \
  NameSpec.cpp \
  NullaryOp.cpp \
  Operand.cpp \
  Operator.cpp \
  OutputArchCmd.cpp \
  OutputCmd.cpp \
  OutputFormatCmd.cpp \
  OutputSectDesc.cpp \
  RpnEvaluator.cpp \
  RpnExpr.cpp \
  ScriptCommand.cpp \
  ScriptFile.cpp \
  ScriptReader.cpp \
  SearchDirCmd.cpp \
  SectionsCmd.cpp \
  StrToken.cpp \
  StringList.cpp \
  TernaryOp.cpp \
  UnaryOp.cpp \
  WildcardPattern.cpp

define build-linker-script-parser
  LOCAL_MODULE_CLASS := STATIC_LIBRARIES

  intermediates := $$(call local-intermediates-dir)

  LOCAL_GENERATED_SOURCES += $$(addprefix $$(intermediates)/, \
                                  FlexLexer.h \
                                  ScriptScanner.cpp \
                                  ScriptParser.cpp \
                                  ScriptParser.h)

$$(intermediates)/ScriptScanner.cpp: $$(LOCAL_PATH)/ScriptScanner.ll
	@mkdir -p $$(dir $$@)
	echo $$<
	@echo "MCLinker Linker Script Lexer: $$(PRIVATE_MODULE) <= $$<"
	$$(hide) $$(LEX) -o$$@ $$<

$$(intermediates)/ScriptParser.cpp: $$(LOCAL_PATH)/ScriptParser.yy
	@mkdir -p $$(dir $$@)
	echo $$<
	@echo "MCLinker Linker Script Parser: $$(PRIVATE_MODULE) <= $$<"
	$$(hide) $$(YACC) --defines=$$(@:.cpp=.h) -o $$@ $$<

$$(eval $$(call copy-one-header,$$(MCLD_ROOT_PATH)/include/mcld/Script/FlexLexer.h, \
                                $$(intermediates)/FlexLexer.h))

$$(intermediates)/ScriptParser.h: $$(intermediates)/ScriptParser.cpp
$$(intermediates)/ScriptScanner.o: $$(intermediates)/ScriptParser.h

endef

# For the host
# =====================================================
include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(mcld_script_SRC_FILES)
LOCAL_MODULE:= libmcldScript

LOCAL_MODULE_TAGS := optional
LOCAL_IS_HOST_MODULE := true

$(eval $(call build-linker-script-parser))

include $(MCLD_HOST_BUILD_MK)
include $(BUILD_HOST_STATIC_LIBRARY)

# For the device
# =====================================================
include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(mcld_script_SRC_FILES)
LOCAL_MODULE:= libmcldScript

LOCAL_MODULE_TAGS := optional

$(eval $(call build-linker-script-parser))

include $(MCLD_DEVICE_BUILD_MK)
include $(BUILD_STATIC_LIBRARY)
