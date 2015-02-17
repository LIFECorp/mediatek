#!/bin/sh
java -Dcheckstyle.suppressions.file=.checkstyle/suppressions.xml -jar ~/local/checkstyle/checkstyle-5.3-all.jar -c .checkstyle/checks.xml -r src
