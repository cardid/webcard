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
    console.log("received " + msg + " (tab " + port.sender.tab.id + ")");
    nativePort.postMessage(msg);
  });
});

function onNativeMessage(msg) {
  console.log("Received native message: " + JSON.stringify(msg));
  // Extract tab id from identifier and restore the original correlation
  // TODO "events are passed to active tab"
  let destination = msg.i.match(/(\d+)\.(.+)/);
  let port = contentPorts.get(parseInt(destination[1]));
  msg.i = destination[2];
  port.postMessage(msg);
}

function onDisconnected() {
  console.log("Failed to connect: " + chrome.runtime.lastError.message);
  nativePort = null;
}

function connectNative() {
  var hostName = "org.cardid.webcard.native";
  nativePort = chrome.runtime.connectNative(hostName);
  console.log("Connected to native messaging host");
  nativePort.onMessage.addListener(onNativeMessage);
  nativePort.onDisconnect.addListener(onDisconnected);
}
