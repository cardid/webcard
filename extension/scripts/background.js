/******************************************************************************/
// "Smart Card Extension" (WebCard): "background.js"

const hostName = 'org.cardid.webcard.native';

let nativePort;

// All the tabs that use "WebCard" extension.
let contentPorts = new Map();

/******************************************************************************/
// Combined WebCard UID:
// (senderId, requestId) => (jsonResponseId)
const packMessageId = (senderId, requestId) => `${senderId}.${requestId}`;

// Restoring original correlation:
// (jsonResponseId) => (senderId, requestId)
const unpackMessageId = (jsonResponseId) =>
{
    let msg = jsonResponseId.match(/(\d+)\.(.+)/);
    return [msg[1], msg[2]];
}

/******************************************************************************/
// Called when the [Native App] sent any [JSON message].
function nativePortCallback(msg)
{
    console.log(`<< ${JSON.stringify(msg)}`);

    if (msg.i)
    {
        // Received a response to a specific request.
        let [senderId, requestId] = unpackMessageId(msg.i);

        // Forward the response to a specific content port.
        let contentPort = contentPorts.get(senderId);
        if (contentPort)
        {
            msg.i = requestId;
            contentPort.postMessage(msg);
        }
    }
    else if (msg.e)
    {
        // Message originating from the [Native App].
        // Broadcast to all content ports.
        contentPorts.forEach((port, senderId) =>
        {
            try
            {
                port.postMessage(msg);
            }
            catch (error)
            {
                // Assuming that given content port is disconnected:
                // "Error: Attempting to use a disconnected port object"
                contentPorts.delete(senderId);
            }
        });
    }
}

/******************************************************************************/
function connectWithNativeApp()
{
    nativePort = chrome.runtime.connectNative(hostName);

    nativePort.onDisconnect.addListener(() =>
    {
        // Invalidate the port to [Native App]
        // (requiring a reconnection later).
        nativePort = null;

        let info = `NativeApp disconnected: ${chrome.runtime.lastError.message}`;
        console.error(info);

        contentPorts.forEach((port) =>
        {
            // [content script's tab] will alert the user
            // and all pending requests will be rejected.
            port.postMessage({ webcard: 'alert', info: info, e: (-1) });
        });
    });

    nativePort.onMessage.addListener(nativePortCallback);
}

/******************************************************************************/
chrome.runtime.onConnect.addListener((contentPort) =>
{
    // Accept `chrome.runtime.connect()` only from
    // the "Smart Card Extension" (WebCard) content scripts.
    if (contentPort.name !== 'webcard')
    {
        return;
    }

    // Register this [content port] once.
    let senderId = contentPort.sender.tab.id.toString();

    // Called when [content script's tab] sends a [webcard request].
    contentPort.onMessage.addListener((msg) =>
    {
        if (msg && (typeof msg.i === 'string'))
        {
            if (!contentPorts.has(senderId))
            {
                contentPorts.set(senderId, contentPort);
            }

            let requestId = msg.i;
            msg.i = packMessageId(senderId, requestId);
            console.log(`>> ${JSON.stringify(msg)}`);

            if (!nativePort)
            {
                // Connecting for the first time
                // (or reconnecting) with the [Native App].
                connectWithNativeApp();
            }

            nativePort.postMessage(msg);
        }
    });
});

/******************************************************************************/
