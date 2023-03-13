/******************************************************************************/
// "webcard.js"
'use strict';

// IIFE BEGIN
(() =>
{
    const WEBCARD_VERSION = '0.4.0';
    const WEBCARD_HOMEPAGE = 'https://webcard.cardid.org';

    /**************************************************************************/
    // `Reader` class.
    function Reader(index, name, atr)
    {
        let self = this;

        self.index     = index;
        self.name      = name;
        self.atr       = atr;
        self.connected = undefined;

        self.connect = (shared) =>
            navigator.webcard.send(2, { r: self.index, p: shared ? 2 : 1 });

        self.disconnect = () =>
            navigator.webcard.send(3, { r: self.index });

        self.transceive = (apdu) =>
            navigator.webcard.send(4, { r: self.index, a: apdu });
    }

    /**************************************************************************/
    // `WebCard` class.
    function WebCard()
    {
        let self = this;
        let _readerList;

        console.info('Starting WebCard...');

        // Install check.
        self.isReady = (null != document.getElementById('webcard-install-banner'));

        if (navigator.userAgent.includes('Windows NT'))
        {
            self.installerUrl = `${WEBCARD_HOMEPAGE}/webcard.msi`;
        }
        else
        {
            self.installerUrl = `${WEBCARD_HOMEPAGE}/webcard.dmg`;
        }

        if (!self.isReady)
        {
          let banner = document.createElement('div');
          banner.id = "webcard-install-banner";
          banner.style = "width:100%;top:0;left:0;position:fixed;"
            + "background-color:rgba(250,250,210,0.85);border-bottom:1px solid gold;z-index:1030;"
          banner.innerHTML = `
            <p style="margin:8pt;font:11pt Helvetica;color:black;">
              This page needs you to install the 
              <a href="${self.installerUrl}">Smart Card Extension</a>
              to access your smart card readers.
            </p>
          `;
          document.body.appendChild(banner);
          console.log('added banner');
        }

        // Remember all pending JavaScript Promises.
        self.pendingRequests = new Map();

        // Command-sending wrapper method.
        self.send = (cmdIdx, otherParams) =>
        {
            if (!self.isReady)
            {
                return new Promise((_, reject) =>
                    reject());
            }

            return new Promise((resolve, reject) =>
            {
                let uid = window.crypto.randomUUID();

                self.pendingRequests.set(
                    uid,
                    { c: cmdIdx, resolve: resolve, reject: reject });

                try
                {
                    window.postMessage(
                        {
                            webcard: 'request',
                            i: uid,
                            c: cmdIdx,
                            ...otherParams
                        },
                        window.location.origin);
                }
                catch (error)
                {
                    self.pendingRequests.delete(uid);
                    reject();
                }
            });
        }

        // Command-sending wrapper method.
        self.sendEx = (cmdIdx, otherParams) =>
        {
            if (!self.isReady)
            {
                let promiseWrapper = () =>
                    new Promise((_, reject) =>
                        reject());

                return {promise: promiseWrapper(), uid: undefined};
            }

            let uid = self.randomUid();

            let promiseWrapper = () =>
                new Promise((resolve, reject) =>
                {
                    self.pendingRequests.set(
                        uid,
                        { c: cmdIdx, resolve: resolve, reject: reject });

                    try
                    {
                        window.postMessage(
                            {
                                webcard: 'request',
                                i: uid,
                                c: cmdIdx,
                                ...otherParams
                            },
                            window.location.origin);
                    }
                    catch (error)
                    {
                        self.pendingRequests.delete(uid);
                        reject();
                    }
                });

            return {promise: promiseWrapper(), uid: uid};
        }

        // Is the latest version of WebCard installed?
        self.getVersions = async() =>
        {
            // Timeout for 2 seconds.
            const timeout = () =>
                new Promise((resolve, _) =>
                    setTimeout(() => resolve(), 2000));

            let pendingVersionCheck;

            // [Get Version] command supported only since "WebCard 0.4.0".
            const checkVersion = () =>
            {
                pendingVersionCheck = self.sendEx(10);
                return pendingVersionCheck.promise;
            }

            // Promises are rejected if the extension is not installed,
            // or the Native App has not been found / was disconnected.
            const versions = await Promise.race([timeout(), checkVersion()])
                .then(msg => [msg?.verExt ?? '?', msg?.verNat ?? '?'])
                .catch(() => ['EXTENSION DISABLED', 'EXTENSION DISABLED']);

            // In case of timeout, remove the [Get version] request.
            if ((typeof pendingVersionCheck !== 'undefined') &&
                (typeof pendingVersionCheck.uid !== 'undefined'))
            {
                self.pendingRequests.delete(pendingVersionCheck.uid);
            }

            return {
                addon: versions[0],
                app: versions[1],
                latest: WEBCARD_VERSION
            };
        }

        // Fetches list of SmartCard readers connected to the PC.
        self.readers = () =>
            self.send(1);

        // Handling content script (Native App) responses.
        self.responseCallback = (msg) =>
        {
            if (typeof msg !== 'object')
            {
                return;
            }

            if (msg.e)
            {
                switch (msg.e)
                {
                    // [Reject any pending promises]
                    case (-1):
                    {
                        self.pendingRequests.forEach((request) =>
                        {
                            request.reject();
                        });

                        self.pendingRequests.clear();

                        break;
                    }

                    // [Card inserted]
                    case 1:
                    {
                        _readerList[msg.r].atr = msg.d;
                        self.cardInserted?.(_readerList[msg.r]);
                        break;
                    }

                    // [Card removed]
                    case 2:
                    {
                        self.cardRemoved?.(_readerList[msg.r]);
                        break;
                    }

                    // [New readers connected]
                    case 3:
                    {
                        self.readersConnected?.(msg.n);
                        break;
                    }

                    // [Any reader unplugged]
                    case 4:
                    {
                        self.readersDisconnected?.(msg.n);
                        break;
                    }
                }

                return;
            }

            if (!msg.i)
            {
                // Impossible to resolve!
                return;
            }

            let request = self.pendingRequests.get(msg.i);
            if (!request)
            {
                // No match for given UID!
                return;
            }

            if (msg.incomplete)
            {
                // Response marked as incomplete
                // (error on the Native App's side).
                request.reject();
            }
            else switch (request.c)
            {
                // [List readers]
                case 1:
                {
                    if (msg.d)
                    {
                        _readerList = [];

                        msg.d.forEach((element, index) =>
                        {
                            _readerList.push(
                                new Reader(index, element.n, element.a));
                        });

                        request.resolve(_readerList);
                    }
                    else
                    {
                        request.reject();
                    }

                    break;
                }

                // [Connect] and [Transceive]
                case 2: case 4:
                {
                    if (msg.d)
                    {
                        request.resolve(msg.d);
                    }
                    else
                    {
                        request.reject();
                    }

                    break;
                }

                // [Get Version]
                case 10:
                {
                    request.resolve(msg);
                    break;
                }

                // [Disconnect] or an unknown command (possibly just a ping)
                default:
                {
                    request.resolve();
                }
            }

            self.pendingRequests.delete(msg.i);
        }
    };

    /**************************************************************************/
    // Called after any `window.postMessage()`
    // (originating either from the content script or the page scripts).
    function windowMessageCallback(event)
    {
        // We want to only accept messages from ourselves
        // (content script => page script).
        if ((event.source !== window) ||
            (event.origin !== window.location.origin) ||
            (typeof event?.data?.webcard !== 'string'))
        {
            return;
        }

        if (event.data.webcard === 'response')
        {
            navigator.webcard.responseCallback(event.data);
        }
    }

    window.addEventListener('message', windowMessageCallback, false);

    /**************************************************************************/
    // WebCard initialization.
    if (typeof navigator.webcard === 'undefined')
    {
        navigator.webcard = new WebCard();
    }

// IIFE END
})();

/******************************************************************************/