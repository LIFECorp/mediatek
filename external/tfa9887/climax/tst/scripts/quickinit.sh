echo "48 kHz I2S with coolflux in Bypass, 1.0 uF coil and other system settings, power on"
climax -r 4 -w 0x880b -r 9 -w 0x0219 -r 9 -w 0x618
climax -r0
