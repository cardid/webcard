#!/bin/bash
# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

set -e

DIR="$( cd "$( dirname "$0" )" && pwd )"
if [ "$(uname -s)" = "Darwin" ]; then
  if [ "$(whoami)" = "root" ]; then
    TARGET_DIR="/Library/Google/Chrome/NativeMessagingHosts"
    EXTENSION_DIR="/Library/Application Support/Google/Chrome/External Extensions"
  else
    TARGET_DIR="$HOME/Library/Application Support/Google/Chrome/NativeMessagingHosts"
    EXTENSION_DIR="$HOME/Library/Application Support/Google/Chrome/External Extensions"
  fi
else
  if [ "$(whoami)" = "root" ]; then
    TARGET_DIR="/etc/opt/chrome/native-messaging-hosts"
    EXTENSION_DIR="/opt/google/chrome/extensions/"
  else
    TARGET_DIR="$HOME/.config/google-chrome/NativeMessagingHosts"
    EXTENSION_DIR="/usr/share/google-chrome/extensions/"
  fi
fi

HOST_NAME=org.cardid.webcard.native
EXTENSION_ID="mnfaoaojdjchapbhjjcofeojlnpofeam"
# Create directory to store native messaging host.
mkdir -p "$TARGET_DIR"
mkdir -p "$EXTENSION_DIR"

# Copy native messaging host manifest.
cp "$DIR/$HOST_NAME.json" "$TARGET_DIR"

# Update host path in the manifest.
HOST_PATH=$DIR/bin/webcard
ESCAPED_HOST_PATH=${HOST_PATH////\\/}
sed -i -e "s/HOST_PATH/$ESCAPED_HOST_PATH/" "$TARGET_DIR/$HOST_NAME.json"

# Write the external extension file
(echo '{'; echo '  "external_update_url": "https://clients2.google.com/service/update2/crx"'; echo '}') > "$EXTENSION_DIR/$EXTENSION_ID.json"

# Set permissions for the manifest so that all users can read it.
chmod o+r "$TARGET_DIR/$HOST_NAME.json"
chmod o+r "$EXTENSION_DIR/$EXTENSION_ID.json"

echo "Native messaging helper $HOST_NAME has been installed."
