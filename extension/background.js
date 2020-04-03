var tab = null;
var nativePort = null;
var contentPort = null;

chrome.runtime.onConnect.addListener(function(port) {
  console.assert(port.name == "webcard");
  if (!contentPort)
    contentPort = port;
  console.log("Connected from tab: " + port.sender.tab.id);
  connectNative();
  port.onMessage.addListener(function(msg) {
    console.log("received " + msg + " (tab " + port.sender.tab.id + ")");
    nativePort.postMessage(msg);
  });
});

function onNativeMessage(message) {
  console.log("Received native message: " + JSON.stringify(message));
  contentPort.postMessage(message);
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
