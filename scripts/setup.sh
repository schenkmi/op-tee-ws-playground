#!/bin/bash

#******************************************************************************
#*
#* VARIABLES
#*
#******************************************************************************

#------------------------------------------------------------------------------
# BASIC VARIABLES
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# Set environement variables
#------------------------------------------------------------------------------
SCRIPTDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
export PROJECT_ROOT=`echo ${SCRIPTDIR%/*}`
export WORK_ROOT=`echo ${SCRIPTDIR%/*/*}`
export BUILD_ROOT=$PROJECT_ROOT/BUILD
export WORKING_COPY=`basename $PROJECT_ROOT`

echo "PROJECT_ROOT: $PROJECT_ROOT"
echo "WORK_ROOT   : $WORK_ROOT"
echo "BUILD_ROOT  : $BUILD_ROOT"
echo "WORKING_COPY: $WORKING_COPY"

export BR_SDK_HOME=/opt/aarch64-buildroot-linux-gnu_sdk-buildroot
export BR_SDK_HOST_BIN=$BR_SDK_HOME/bin
export BR_SDK_SYSROOT=$BR_SDK_HOME/aarch64-buildroot-linux-gnu/sysroot
export BR_SDK_PKG_CONFIG=$BR_HOST_BIN/pkg-config
export BR_SDK_PKG_CONFIG_PATH=$BR_SYSROOT/usr/lib/pkgconfig:$BR_SYSROOT/usr/share/pkgconfig:$BR_SYSROOT/lib/pkgconfig
export BR_SDK_PKG_CONFIG_LIBDIR=$BR_SYSROOT/usr/lib
export PATH=$BR_HOST_BIN:$PATH

# check if path already set if not set it
SDKDIR=`echo $BR_SDK_HOST_BIN`
echo $PATH | grep "$SDKDIR:" >/dev/null
if [ $? -ne 0 ]; then
  echo $PATH | grep "$SDKDIR$" >/dev/null
  if [ $? -ne 0 ]; then
    export PATH=$BR_SDK_HOST_BIN:$PATH
  fi
fi

#------------------------------------------------------------------------------
# Define alias based on project
#------------------------------------------------------------------------------

# General aliases
alias ll='ls -l'
alias lll='ls -al'
alias ..='cd ..'

# Project specific aliases
alias cdscripts='cd $PROJECT_ROOT/scripts'
alias cdex='cd $PROJECT_ROOT/examples'
alias cdta='cd $PROJECT_ROOT/tasks'
alias cdint='cd $PROJECT_ROOT/int'

# set group rights
umask 002

#------------------------------------------------------------------------------
# Calling ccache setup
#------------------------------------------------------------------------------
source $PROJECT_ROOT/scripts/setupccache

cdex

return 0

