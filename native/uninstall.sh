#!/bin/sh
# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

set -e

EXTENSION_ID="icpgdjoejngfekheifhhaceealdnipfb"
if [ "$(uname -s)" == "Darwin" ]; then
  if [ "$(whoami)" == "root" ]; then
    TARGET_DIR="/Library/Google/Chrome/NativeMessagingHosts"
    EXTENSION_DIR="/Library/Application Support/Google/Chrome/External Extensions"
  else
    TARGET_DIR="$HOME/Library/Application Support/Google/Chrome/NativeMessagingHosts"
    EXTENSION_DIR="$HOME/Library/Application Support/Google/Chrome/External Extensions"
  fi
else
  if [ "$(whoami)" == "root" ]; then
    TARGET_DIR="/etc/opt/chrome/native-messaging-hosts"
    EXTENSION_DIR="/opt/google/chrome/extensions/"
  else
    TARGET_DIR="$HOME/.config/google-chrome/NativeMessagingHosts"
    EXTENSION_DIR="/usr/share/google-chrome/extensions/"
  fi
fi

HOST_NAME=org.cardid.webcard.native
EXTENSION_ID="icpgdjoejngfekheifhhaceealdnipfb"
rm "$TARGET_DIR/$HOST_NAME.json"
rm "$EXTENSION_DIR/$EXTENSION_ID.json"
echo "Native messaging helper $HOST_NAME has been uninstalled."
