/******************************************************************************/
// "Smart Card Extension" (WebCard): "content.js"

const webcardVersion = '0.4.0';

let backgroundPort;

let banner = document.createElement('div');
banner.id = "webcard-install-banner";
document.documentElement.appendChild(banner);

/******************************************************************************/
// Called when the [background port] processed
// any [JSON message] from the [Native App].
function backgroundPortCallback(msg)
{
    if (msg?.webcard === 'alert')
    {
        // Alert forwared from the [background script].
        window.alert(msg.info);
    }

    if (msg.e || msg.i)
    {
        if (msg.verNat)
        {
            // User requested for version check.
            msg.verExt = webcardVersion;
        }

        // Bubble-up this message (event or response)
        // to the webpage (browser tab).
        msg.webcard = 'response';
        window.postMessage(msg, window.location.origin);
    }
}

/******************************************************************************/
// Called after any `window.postMessage()`
// (originating either from the content script or the page scripts).
function windowMessageCallback(event)
{
    // We want to only accept messages from ourselves
    // (page script => content script).
    if ((event.source !== window) ||
        (event.origin !== window.location.origin) ||
        (typeof event?.data?.webcard !== 'string'))
    {
        return;
    }

    if (event.data.webcard === 'request')
    {
        if (!backgroundPort)
        {
            // This page is sending its first [webcard request].
            // Connect with the [background script] once.
            backgroundPort = chrome.runtime.connect({ name: 'webcard' });
            backgroundPort.onMessage.addListener(backgroundPortCallback);
        }

        delete event.data.webcard;
        backgroundPort.postMessage(event.data);
    }
}

window.addEventListener('message', windowMessageCallback, false);

/******************************************************************************/
