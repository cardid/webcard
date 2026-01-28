# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

WebCard is a cross-platform browser extension that enables web applications to communicate with local smart card readers via APDU commands. It consists of three components:

1. **Browser Extension** (`/extension/`) - Manifest V3 Chrome/Edge extension using native messaging
2. **Native Application** (`/native/`) - C application that interfaces with OS smart card subsystems
3. **Client Library** (`/src/webcard.js`) - JavaScript library for web pages to access `navigator.webcard`

## Build Commands

### JavaScript Library (npm)

```bash
npm install        # Install dependencies (esbuild)
npm run build      # Build all dist files
```

Output files in `dist/`:
- `webcard.js` - ES module (unminified, for debugging)
- `webcard.min.js` - ES module (minified, primary CDN target)
- `webcard.iife.js` - Browser global version (backward compatible)
- `webcard.d.ts` - TypeScript declarations

### Native Application (C)

```bash
cd native
make release    # Optimized build (-O3, stripped)
make debug      # Debug build with symbols and -D_DEBUG
```

Output binaries are placed in platform-specific directories:
- macOS: `native/out/macos/webcard_arm`
- Linux 64-bit: `native/out/linux64/webcard`
- Windows 64-bit: `native/out/win64/webcard.exe`

### Installation

```bash
# macOS/Linux
cd native && make && ./install.sh

# Windows (from native folder)
MSBuild webcard.vcxproj
install.bat
```

### Testing

No automated test suite. Manual testing via the example page:

```bash
npm run build                    # Build the library first
cd example
python3 -m http.server
# Open http://localhost:8000 in Chrome/Edge with extension loaded
```

The extension must be loaded unpacked from `/extension` via `chrome://extensions` with Developer Mode enabled.

## Architecture

### Message Flow

```
Web Page (webcard.js) → Content Script → Background Service Worker → Native App → PC/SC → Smart Card
```

Messages use JSON with request/response correlation via `i` (message ID) field.

### Native Code Structure (`/native/src/`)

- `webcard_main.c` - Entry point, Chrome native messaging I/O
- `json/` - Custom JSON parser (no external dependencies)
- `smart_cards/` - PC/SC wrapper: `sc_webcard.c` (commands), `sc_conn.c` (connections), `sc_db.c` (reader tracking)
- `os_specific/` - Platform abstraction (Windows winscard, Linux pcsclite, macOS PCSC framework)
- `utf/` - UTF-8/UTF-16 conversion utilities

### Extension Structure (`/extension/`)

- `scripts/background.js` - Service worker managing native messaging port
- `scripts/content.js` - Injects webcard.js, relays messages between page and background
- `manifest.json` - Extension configuration (Manifest V3)

### Protocol Commands

Commands (`c` field): 1=list readers, 2=connect, 3=disconnect, 4=transceive, 10=get version

Events (`e` field): 1=card insert, 2=card remove, 3=readers connected, 4=readers disconnected

## Key Files

- `native/org.cardid.webcard.native.json` - Native messaging manifest (allowed extension IDs)
- `src/webcard.js` - Client library implementing `navigator.webcard` API
- `extension/manifest.json` - Browser extension manifest

## CI/CD

GitHub Actions runs CodeQL security analysis on push/PR to main for C++ and JavaScript code.
