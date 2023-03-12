#!/bin/sh
# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

set -e

DIR="$( cd "$( dirname "$0" )" && pwd )"
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
# Create directory to store native messaging host.
mkdir -p "$CHROME_TARGET_DIR"
mkdir -p "$CHROME_EXTENSION_DIR"
mkdir -p "$EDGE_TARGET_DIR"
mkdir -p "$EDGE_EXTENSION_DIR"

# Copy native messaging host manifest.
cp "$DIR/$HOST_NAME.json" "$CHROME_TARGET_DIR"
cp "$DIR/$HOST_NAME.json" "$EDGE_TARGET_DIR"

# Update host path in the manifest.
HOST_PATH=$DIR/out/webcard
ESCAPED_HOST_PATH=${HOST_PATH////\\/}
sed -i -e "s/HOST_PATH/$ESCAPED_HOST_PATH/" "$CHROME_TARGET_DIR/$HOST_NAME.json"
sed -i -e "s/HOST_PATH/$ESCAPED_HOST_PATH/" "$EDGE_TARGET_DIR/$HOST_NAME.json"

# Write the external extension file
(echo '{'; echo '  "external_update_url": "https://clients2.google.com/service/update2/crx"'; echo '}') > "$CHROME_EXTENSION_DIR/$CHROME_EXTENSION_ID.json"
(echo '{'; echo '  "external_update_url": "https://edge.microsoft.com/extensionwebstorebase/v1/crx"'; echo '}') > "$EDGE_EXTENSION_DIR/$EDGE_EXTENSION_ID.json"

# Set permissions for the manifest so that all users can read it.
chmod o+r "$CHROME_TARGET_DIR/$HOST_NAME.json"
chmod o+r "$CHROME_EXTENSION_DIR/$CHROME_EXTENSION_ID.json"
chmod o+r "$EDGE_TARGET_DIR/$HOST_NAME.json"
chmod o+r "$EDGE_EXTENSION_DIR/$EDGE_EXTENSION_ID.json"

echo "Native messaging helper $HOST_NAME has been installed."
