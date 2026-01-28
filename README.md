# Smart Card Browser Extension

This smart card browser extension is an attempt to provide a bridge between the JavaScript world that a Web developer is familiar with, and the native smart card subsystems available in desktop platforms. It works in Windows 10 and 11, macOS and Linux.

The idea is that a Web developer who wants to have low level access to smart cards in the machine where the browser is running would add a `webcard.js` file to the page. That script would in turn check if the extension is installed and if not prompt the user to install it.

The extension adds a `webcard` object to `navigator` and through it `navigator.webcard.readers` provides a list of the smart card `reader` objects available in the machine.
Each `reader` has a `name` and an `atr` if there is a card inserted on it.

`reader` has three methods:
- `connect` to establish a connection with the inserted card. It receives an optional argument to indicate if the connection should be exclusive or not (shared) the default is `true`
- `transcieve` that sends the APDU passed as a hexidecimal string and returs the response also as a hexadecimal string
- `disconnect` closes the connection

## Quick Start

### Step 1: Install the Extension

Download and run the installer for your platform:
- **Windows:** [webcard.msi](https://webcard.cardid.org/webcard.msi)
- **macOS:** [webcard.pkg](https://webcard.cardid.org/webcard.pkg)

### Step 2: Add WebCard to Your Page

**Option A: CDN (recommended)**
```html
<script src="https://cdn.jsdelivr.net/npm/@cardid/webcard/dist/webcard.min.js"></script>
```

**Option B: ES Module**
```html
<script type="module">
  import 'https://esm.run/@cardid/webcard';
  // navigator.webcard is now available
</script>
```

**Option C: npm**
```bash
npm install @cardid/webcard
```
```javascript
import '@cardid/webcard';
```

### Step 3: List Readers
```javascript
const readers = await navigator.webcard.readers();
console.log(readers); // [{name: "Reader Name", atr: "", ...}, ...]
```

### Step 4: Detect Card Events
```javascript
navigator.webcard.cardInserted = (reader) => {
  console.log(`Card in ${reader.name}, ATR: ${reader.atr}`);
};

navigator.webcard.cardRemoved = (reader) => {
  console.log(`Card removed from ${reader.name}`);
};
```

### Step 5: Communicate with Card
```javascript
// Connect (true = shared mode, false = exclusive)
await reader.connect(true);

// Send APDU command (hex string)
const response = await reader.transceive('FFCA000000');
console.log(response); // "9000" or data + status

// Disconnect
await reader.disconnect();
```

### Reader Events
```javascript
navigator.webcard.readersConnected = (count) => { /* USB reader plugged in */ };
navigator.webcard.readersDisconnected = (count) => { /* USB reader unplugged */ };
```

## Installing (Development)

Currently this development version requires manual installation of the unpacked extension:

1. Build the native client for your platfrom, running either `make` (macOS) or `MSBuild webcard.vcxproj` (Windows) in the `native` folder
2. Install the native client running `install.sh` (macOS) or `install.bat` (Windows) in the `native` folder
3. Open Chrome -> Extensions, enable developer mode and add the unpacked extension from the `extension` folder.
4. Open the example page, for example running `python3 -m http.server` from the `example` folder and then navigate to http://localhost:8000

## Native Messages

Messages to native:
```
{i: 'string', c: integer, r: integer, a: 'string', p: integer}
```
i: unique message identifier
c: command 1-list readers, 2-connect, 3-disconnect, 4-transcieve
r: index of reader in reader list
a: hex cAPDU to send to the card, sent only for c: 4
p: parameter, share mode for connect, sent only for c: 2

Messages from native:
```
{i: 'string', e: integer, r: integer, d: [array]|'string'}
```
i: unique message identifier, to link the response. Empty string on reader events
e: reader event 1-card insert, 2-card remove. Sent only for reader events
r: reader index for reader events
d: data 1-string array with list of readers, 2-card atr, 4-hex rAPDU

## Alternatives

There are apparently many options for [smart card extensions](https://chrome.google.com/webstore/search/smart%20card?hl=en-US&_category=extensions) in the Chrome store. But I could not figure out how to use them for raw APDU exchange. Maybe one of those is more suitable for your project.
