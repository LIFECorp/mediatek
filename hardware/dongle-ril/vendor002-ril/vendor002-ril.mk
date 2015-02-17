# Copyright (C) 2011 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

PRODUCT_COPY_FILES += \
					mediatek/hardware/dongle-ril/vendor002-ril/ppp/ip-up-ppp0:system/etc/ppp/ip-up-ppp0 \
					mediatek/hardware/dongle-ril/vendor002-ril/ppp/ip-down-ppp0:system/etc/ppp/ip-down-ppp0 \
					mediatek/hardware/dongle-ril/vendor002-ril/script/init.gprs-pppd:system/bin/init.gprs-pppd \
					mediatek/hardware/dongle-ril/vendor002-ril/script/zte_ppp_dialer:system/etc/zte_ppp_dialer \
					mediatek/hardware/dongle-ril/vendor002-ril/zterilPara:system/lib/zterilPara 		
