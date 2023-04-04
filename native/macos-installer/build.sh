#!/bin/sh
mkdir -p ./files/Library/Application\ Support/CardID/WebCard
cp ../out/webcard ./files/Library/Application\ Support/CardID/WebCard
cp ../org.cardid.webcard.native.json ./files/Library/Application\ Support/CardID/WebCard
pkgbuild --root ./files --identifier WebCard --version 0.4.0 --install-location "/" --scripts ./scripts intermediate.pkg
productbuild --distribution distribution.xml --package-path intermediate.pkg webcard.pkg
rm intermediate.pkg
rm -rf ./files