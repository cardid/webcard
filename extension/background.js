var nativePort = null;  // Connection with native app
var contentPorts = new Map();

chrome.runtime.onConnect.addListener(function(port) {
  console.assert(port.name == "webcard");
  contentPorts.set(port.sender.tab.id, port);
  console.log("Connected from tab: " + port.sender.tab.id);
  if (!nativePort)
    connectNative();
  port.onMessage.addListener(function(msg) {
    // Add tab id to the message UID
    msg.i = port.sender.tab.id + '.' + msg.i;
    console.log(">> " + JSON.stringify(msg));
    nativePort.postMessage(msg);
  });
});

function onNativeMessage(msg) {
  console.log("<< " + JSON.stringify(msg));
  // Events get broadcasted to all tabs
  if (msg.i === undefined) {
    for (let port of contentPorts.values()) {
      port.postMessage(msg);
    }
  } else {
    // Extract tab id from identifier and restore the original correlation
    let destination = msg.i.match(/(\d+)\.(.+)/);
    let port = contentPorts.get(parseInt(destination[1]));
    msg.i = destination[2];
    port.postMessage(msg);
  }
}

function onDisconnected() {
  console.log("Native client disconnected: " + chrome.runtime.lastError.message);
  nativePort = null;
}

function connectNative() {
  var hostName = "org.cardid.webcard.native";
  nativePort = chrome.runtime.connectNative(hostName);
  console.log("Connected to native messaging host");
  nativePort.onMessage.addListener(onNativeMessage);
  nativePort.onDisconnect.addListener(onDisconnected);
}
