LINUX_KERNEL_VERSION = kernel-3.4
MTK_CPU=arm_cortexa7
TARGET_ARCH_VARIANT=armv7-a-neon
MTK_FIRST_MD=1
MTK_DP_FRAMEWORK=yes
MTK_ION_SUPPORT=yes
MTK_ENABLE_MD1=yes
CUSTOM_KERNEL_SSW=ssw_single
MTK_SWIP_VORBIS=yes
MTK_PRODUCT_INFO_SUPPORT=yes
DMNR_TUNNING_AT_MODEMSIDE=no
MTK_FENCE_SUPPORT=yes
MTK_PQ_SUPPORT=PQ_HW_VER_2
MTK_FM_RX_AUDIO=FM_DIGITAL_INPUT
MTK_BWC_SUPPORT=yes

# common part
MTK_COMBO_SUPPORT=yes
CUSTOM_HAL_COMBO=mt6571
CUSTOM_HAL_ANT=mt6571_ant_m1
MTK_COMBO_CHIP=CONSYS_6571


# Bluetooth chip type
MTK_BT_CHIP=MTK_CONSYS_MT6571
MTK_LCEEFT_SUPPORT=yes

MTK_SHARE_MODEM_SUPPORT=2
MTK_MD_SHUT_DOWN_NT=yes
MTK_SEC_SECRO_AC_SUPPORT=yes

# Enable Scatter file as YAML format
MTK_YAML_SCATTER_FILE_SUPPORT=yes

MTK_GPS_CHIP=MTK_GPS_MT6571
MTK_UART_USB_SWITCH=yes
MTK_DATAUSAGE_SUPPORT=no
MTK_SUPPORT_MJPEG=yes
MTK_MEM_PRESERVED_MODE_ENABLE=no

# For audio kernel driver's speaker customization folder definition.
CUSTOM_KERNEL_SOUND=amp_6323pmic_spk

# User space flashlight driver:
CUSTOM_HAL_FLASHLIGHT=constant_flashlight
# Kernel space flashlight driver
CUSTOM_KERNEL_FLASHLIGHT=constant_flashlight

# The battery feature is the MUST include feature and can not be disable.This feature provides battery monitor and charging. The system can not boot up without battery.
CUSTOM_KERNEL_BATTERY=battery

# Add these variables to define the default input method and default input method languages.
DEFAULT_LATIN_IME_LANGUAGES=en-US fr ru

# In Audio record,  Enable/disable AWB encode, yes: enable
# no:disable
HAVE_AWBENCODE_FEATURE=no

# Originally designed for ESD(Electrostatic discharge) test. For internal use only.
MTK_ANDROIDFACTORYMODE_APP=no

# It can capture panorama picture. We will take maximum 9 pictures merge to one panorama image
MTK_AUTORAMA_SUPPORT=no
MTK_BICR_SUPPORT=yes

# To control whether enable or disable LCD controller dithering feature. If choose yes, LCD controller would do dithering to avoid contour effect, but side effect is that dithering mechanism will make some noises
MTK_DITHERING_SUPPORT=no

MTK_FASTBOOT_SUPPORT=yes

# Used in FM driver and Native lib makefiles to decide which chip driver will be build
MTK_FM_CHIP=MT6627_FM

# FM short antenna feature option: For FM Radio, when there's no headset,  if short antenna is available,  FM app will switch to short antenna automaticlly
#  For FM Transmitter, short antenna is the default transmit antenna. If target PCB provides short antenna, please set this compile option to "yes", or else set to "no"
MTK_FM_SHORT_ANTENNA_SUPPORT=no

# This feature option is used to distinguish the version of GPU. Different GPU has different shader compiler. We can use correctly shader compiler to compiler our shaders by this option
MTK_GPU_SUPPORT=yes

# it has two values - 1 or 2. 1 stands for google default lockscreen, which we drag the lock bar to right, then the phone can be unlocked. 2 stands for SlideLockScreen,  which we slide the screen up, then the phone can be unlocked.
MTK_LOCKSCREEN_TYPE=1

# Let persist.sys.usb.config in default.prop as mass_storage.
MTK_MASS_STORAGE=yes

# Control NEON HW support or not
MTK_NEON_SUPPORT=yes

# MTK_FLIGHT_MODE_POWER_OFF_MD is used to control if modem is powered off when entering flight mode MTK_TELEPHONY_MODE is used for specify current telephony mode MTK_FIRST_MD is used to specify the high priority modem
MTK_TELEPHONY_MODE=2

# the switcher of turn on /off weather3d  widget
MTK_WEATHER3D_WIDGET=no

MTK_PLATFORM_OPTIMIZE=yes
MTK_TINY_UTIL=yes

NO_INIT_PERMISSION_CHECK=yes
MTK_CTA_SET=yes
MTK_KERNEL_POWER_OFF_CHARGING=yes
MTK_RAT_WCDMA_PREFERRED=no
MTK_TODOS_APP=yes
MTK_WLAN_CHIP=MTK_CONNSYS_MT6571
MTK_UMTS_TDD128_MODE=yes
MTK_MD1_SUPPORT=4

MTK_IME_GERMAN_SUPPORT=no
MTK_IME_HINDI_SUPPORT=no
MTK_IME_INDONESIAN_SUPPORT=no
MTK_IME_ITALIAN_SUPPORT=no
MTK_IME_MALAY_SUPPORT=no
MTK_IME_PORTUGUESE_SUPPORT=no
MTK_IME_RUSSIAN_SUPPORT=no
MTK_IME_SPANISH_SUPPORT=no
MTK_IME_THAI_SUPPORT=no

MTK_AUDIO_BLOUD_CUSTOMPARAMETER_REV=MTK_AUDIO_BLOUD_CUSTOMPARAMETER_V4
MTK_DATAUSAGELOCKSCREENCLIENT_SUPPORT=no
MTK_EAP_SIM_AKA=yes
MTK_FLV_PLAYBACK_SUPPORT=yes
MTK_FM_TX_SUPPORT=no
MTK_LAUNCH_TIME_OPTIMIZE=yes
MTK_MAV_PLAYBACK_SUPPORT=yes
MTK_SENSOR_SUPPORT=yes
MTK_USES_STAGEFRIGHT_DEFAULT_CODE=no
MTK_USES_VR_DYNAMIC_QUALITY_MECHANISM=yes
MTK_VIDEOWIDGET_APP=no
MTK_VIDEO_FAVORITES_WIDGET_APP=no
MTK_WIFI_P2P_SUPPORT=no
MTK_MEDIA3D_APP=no
MTK_FACEBEAUTY_SUPPORT=no

MTK_MAV_SUPPORT=yes
MTK_HWC_SUPPORT=yes
MTK_BEAM_PLUS_SUPPORT=no
MTK_NFC_SUPPORT=no
MTK_NFC_ADDON_SUPPORT=no
MTK_NFC_APP_SUPPORT=no
MTK_MOBILE_MANAGEMENT = yes
MTK_PERMISSION_CONTROL = yes
MTK_PROGUARD_SHRINKING = yes
MTK_HANDSFREE_DMNR_SUPPORT = no
MTK_ASR_SUPPORT = no
MTK_BESRECORD_2_0_SUPPORT = yes

MD1_SIZE = ref:chkMDSize.pl md1
MD2_SIZE = ref:chkMDSize.pl md2
MTK_SAFEMEDIA_SUPPORT=yes

MTK_SMSREG_APP = no
MTK_MDM_APP = no
MTK_GPU_CHIP=MALI400MP1
MTK_HFP1_6_SUPPORT=yes

LEGACY_DFO_GEN=yes

MTK_POWER_EXT_DETECT=yes
MTK_CAM_VSS_SUPPORT = no
MTK_PUMP_EXPRESS_SUPPORT= yes

MTK_PASSPOINT_R1_SUPPORT = no 

DMNR_COMPLEX_ARCH_SUPPORT = no