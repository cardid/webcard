var port;

var banner = document.createElement('div');
banner.id = "webcard-install-banner";
document.documentElement.appendChild(banner);

window.addEventListener("message", function(event) {
  // We only accept messages from ourselves
  if (!Object.is(event.source, this.window) || !window.location.href.startsWith(event.origin))
    return;

  if (event.data.webcard && (event.data.webcard == "request")) {
    if (!port)
    {
      port = chrome.runtime.connect({name: "webcard"});
      port.onMessage.addListener(function(msg) {
        // Got a message from background, bubble it up to the page
        msg.webcard = 'response';
        window.postMessage(msg, window.location.href);
      });
    }
    
    delete event.data.webcard;
    // Received a message from the page, pass it ot to background
    port.postMessage(event.data);
  }
}, false);