#!/bin/sh
# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

set -e

if [ "$(uname -s)" == "Darwin" ]; then
  if [ "$(whoami)" == "root" ]; then
    CHROME_TARGET_DIR="/Library/Google/Chrome/NativeMessagingHosts"
    CHROME_EXTENSION_DIR="/Library/Application Support/Google/Chrome/External Extensions"
    EDGE_TARGET_DIR="/Library/Microsoft/Edge/NativeMessagingHosts"
    EDGE_EXTENSION_DIR="/Library/Application Support/Microsoft/Edge/External Extensions"
  else
    CHROME_TARGET_DIR="$HOME/Library/Application Support/Google/Chrome/NativeMessagingHosts"
    CHROME_EXTENSION_DIR="$HOME/Library/Application Support/Google/Chrome/External Extensions"
    EDGE_TARGET_DIR="$HOME/Library/Application Support/Microsoft Edge/NativeMessagingHosts"
    EDGE_EXTENSION_DIR="$HOME/Library/Application Support/Microsoft Edge/External Extensions"
  fi
else
  if [ "$(whoami)" == "root" ]; then
    CHROME_TARGET_DIR="/etc/opt/chrome/native-messaging-hosts"
    CHROME_EXTENSION_DIR="/opt/google/chrome/extensions/"
    EDGE_TARGET_DIR="/etc/opt/edge/native-messaging-hosts"
    EDGE_EXTENSION_DIR="/usr/share/microsoft-edge/extensions/"
  else
    CHROME_TARGET_DIR="$HOME/.config/google-chrome/NativeMessagingHosts"
    CHROME_EXTENSION_DIR="/usr/share/google-chrome/extensions/"
    EDGE_TARGET_DIR="$HOME/.config/microsoft-edge/NativeMessagingHosts"
    EDGE_EXTENSION_DIR="/.config/microsoft-edge/External Extensions/"
  fi
fi

HOST_NAME=org.cardid.webcard.native
CHROME_EXTENSION_ID="icpgdjoejngfekheifhhaceealdnipfb"
EDGE_EXTENSION_ID="gfahiodabpingeopmhmnlilmbalpokoa"
rm "$CHROME_TARGET_DIR/$HOST_NAME.json"
rm "$CHROME_EXTENSION_DIR/$CHROME_EXTENSION_ID.json"
rm "$EDGE_TARGET_DIR/$HOST_NAME.json"
rm "$EDGE_EXTENSION_DIR/$EDGE_EXTENSION_ID.json"
echo "Native messaging helper $HOST_NAME has been uninstalled."
