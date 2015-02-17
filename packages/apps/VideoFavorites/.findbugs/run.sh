#!/bin/sh
~/local/findbugs/bin/findbugs -textui -sourcepath src -exclude .findbugs/exclude.xml -auxclasspath ../../../../out/target/common/obj/JAVA_LIBRARIES/framework_intermediates/classes.jar ../../../../out/target/common/obj/JAVA_LIBRARIES/com.mediatek.ngin3d_intermediates/classes.jar 
