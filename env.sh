#!/bin/bash
export ANDROID_SDK=/home/opt/android-sdk-linux
export ANDROID_NDK=/home/opt/android-ndk-r13b
#export MY_GRADLE=/home/opt/gradle-2.14.1/bin
export ANDROID_HOME=${ANDROID_SDK}

#export JAVA_HOME=/home/ruziniu/hongjian.huang/ijkplayer/jdk1.6.0_45
#export JAVA_HOME=/home/opt/jdk1.7.0_79
#export JRE_HOME=$JAVA_HOME/jre  
#export CLASSPATH=$JAVA_HOME/lib:$JRE_HOME/lib:$CLASSPATH  

#export PATH=$PATH:$ANDROID_SDK/tools:$ANDROID_SDK/platform-tools:$ANDROID_NDK:$JAVA_HOME/bin:$PATH:$JRE_HOME/bin:$MY_GRADLE
export PATH=$ANDROID_SDK/tools:$ANDROID_SDK/platform-tools:$ANDROID_NDK:$PATH


#$JAVA_HOME/bin:$PATH:$JRE_HOME/bin:$MY_GRADLE

export DEVICE=HI3798
