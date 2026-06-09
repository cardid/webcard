/******************************************************************************/
// WebCard - Smart Card Browser Extension Library
// https://webcard.cardid.org
/******************************************************************************/
'use strict';

const WEBCARD_VERSION = '0.4.1';
const WEBCARD_HOMEPAGE = 'https://webcard.cardid.org';

/******************************************************************************/
// `Reader` class - represents a smart card reader
function Reader(index, name, atr) {
    let self = this;

    self.index = index;
    self.name = name;
    self.atr = atr;
    self.connected = undefined;
    self.connectStartTime = null;

    self.connect = (shared) => {
        self.connectStartTime = Date.now();
        return navigator.webcard.send(2, { r: self.index, p: shared ? 2 : 1 });
    };

    self.disconnect = () => {
        let p = navigator.webcard.send(3, { r: self.index });
        p.then(() => {
            if (self.connectStartTime) {
                let duration = Date.now() - self.connectStartTime;
                if (navigator.webcard && navigator.webcard.sendTelemetry) {
                    navigator.webcard.sendTelemetry('transaction', {
                        readerName: self.name,
                        duration: duration
                    });
                }
                self.connectStartTime = null;
            }
        }).catch(() => {});
        return p;
    };

    self.transceive = (apdu) =>
        navigator.webcard.send(4, { r: self.index, a: apdu });
}

/******************************************************************************/
// `WebCard` class - main API interface
function WebCard(options = {}) {
    let self = this;
    let _readerList;

    console.info('Starting WebCard...');

    // Environment and Capability checks
    const ua = navigator.userAgent;
    const isWindows = ua.includes('Windows NT');
    const isMac = ua.includes('Mac OS X') || ua.includes('Macintosh');
    const isLinux = ua.includes('Linux') && !ua.includes('Android');
    const isDesktop = isWindows || isMac || isLinux;
    const isChromium = ua.includes('Chrome') || ua.includes('Edg') || ua.includes('Brave');
    
    self.isSupported = isDesktop && isChromium;

    // Determine OS and Browser for telemetry
    let os = 'Other';
    if (isWindows) os = 'Windows';
    else if (isMac) os = 'macOS';
    else if (isLinux) os = 'Linux';
    else if (ua.includes('Android')) os = 'Android';
    else if (ua.includes('iPhone') || ua.includes('iPad')) os = 'iOS';

    let browser = 'Other';
    if (ua.includes('Edg')) browser = 'Edge';
    else if (ua.includes('Chrome') && !ua.includes('Edg')) browser = 'Chrome';
    else if (ua.includes('Firefox')) browser = 'Firefox';
    else if (ua.includes('Safari') && !ua.includes('Chrome')) browser = 'Safari';

    // Install check.
    self.isReady = (null != document.getElementById('webcard-install-banner'));

    if (isWindows) {
        self.installerUrl = `${WEBCARD_HOMEPAGE}/webcard.msi`;
    } else {
        self.installerUrl = `${WEBCARD_HOMEPAGE}/webcard.pkg`;
    }

    // Telemetry setup
    self.telemetryDisabled = window.WEBCARD_DISABLE_TELEMETRY === true || options.disableTelemetry === true;
    
    self.sendTelemetry = (type, payload) => {
        if (self.telemetryDisabled) return;
        try {
            fetch('https://telemetry.cardid.org/track', {
                method: 'POST',
                mode: 'cors',
                keepalive: true,
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ type: type, ...payload })
            }).catch(() => {});
        } catch (e) {}
    };

    self.sendTelemetry('load', { os, browser, webcardVersion: WEBCARD_VERSION });

    // Show banner if not ready or not supported. Developer can hide it with options.hideBanner
    // or by setting window.WEBCARD_HIDE_BANNER = true before loading the script.
    if (!self.isReady && options.hideBanner !== true && window.WEBCARD_HIDE_BANNER !== true) {
        let banner = document.createElement('div');
        banner.id = "webcard-install-banner";
        Object.assign(banner.style, {
            boxSizing: 'border-box',
            width: '100%',
            top: '0',
            left: '0',
            position: 'fixed',
            backgroundColor: '#1E293B',
            color: '#F8FAFC',
            borderBottom: '1px solid #334155',
            zIndex: '9999',
            fontFamily: '-apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, Arial, sans-serif',
            display: 'flex',
            justifyContent: 'space-between',
            alignItems: 'center',
            padding: '12px 20px',
            boxShadow: '0 4px 6px -1px rgba(0, 0, 0, 0.1)'
        });

        let contentHtml = '';
        if (!self.isSupported) {
            contentHtml = `This page requires a desktop Chromium-based browser (Chrome, Edge) to access smart card readers.`;
        } else {
            contentHtml = `This page needs you to install the <a href="${self.installerUrl}" style="color: #38BDF8; font-weight: 500; text-decoration: none;">Smart Card Extension</a> to access your smart card readers.`;
        }

        banner.innerHTML = `
            <div style="font-size: 14px; line-height: 1.5; flex-grow: 1;">
                <span style="margin-right: 12px; font-size: 16px;">💳</span>
                ${contentHtml}
            </div>
            <button id="webcard-banner-dismiss" style="background: none; border: none; color: #94A3B8; cursor: pointer; padding: 4px; border-radius: 4px; display: flex; align-items: center; justify-content: center;">
                <svg width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><line x1="18" y1="6" x2="6" y2="18"></line><line x1="6" y1="6" x2="18" y2="18"></line></svg>
            </button>
        `;
        
        document.body.appendChild(banner);
        document.getElementById('webcard-banner-dismiss').addEventListener('click', () => {
            banner.style.display = 'none';
        });
        console.log(`WebCard banner added [Supported: ${self.isSupported}]`);
    }

    // Remember all pending JavaScript Promises.
    self.pendingRequests = new Map();

    // Command-sending wrapper method.
    self.send = (cmdIdx, otherParams) => {
        if (!self.isReady) {
            return new Promise((_, reject) => reject());
        }

        return new Promise((resolve, reject) => {
            let uid = window.crypto.randomUUID();

            self.pendingRequests.set(
                uid,
                { c: cmdIdx, resolve: resolve, reject: reject });

            try {
                window.postMessage(
                    {
                        webcard: 'request',
                        i: uid,
                        c: cmdIdx,
                        ...otherParams
                    },
                    window.location.origin);
            } catch (error) {
                self.pendingRequests.delete(uid);
                reject();
            }
        });
    }

    // Command-sending wrapper method (extended version with uid access).
    self.sendEx = (cmdIdx, otherParams) => {
        if (!self.isReady) {
            let promiseWrapper = () =>
                new Promise((_, reject) => reject());

            return { promise: promiseWrapper(), uid: undefined };
        }

        let uid = window.crypto.randomUUID();

        let promiseWrapper = () =>
            new Promise((resolve, reject) => {
                self.pendingRequests.set(
                    uid,
                    { c: cmdIdx, resolve: resolve, reject: reject });

                try {
                    window.postMessage(
                        {
                            webcard: 'request',
                            i: uid,
                            c: cmdIdx,
                            ...otherParams
                        },
                        window.location.origin);
                } catch (error) {
                    self.pendingRequests.delete(uid);
                    reject();
                }
            });

        return { promise: promiseWrapper(), uid: uid };
    }

    // Is the latest version of WebCard installed?
    self.getVersions = async () => {
        // Timeout for 2 seconds.
        const timeout = () =>
            new Promise((resolve, _) =>
                setTimeout(() => resolve(), 2000));

        let pendingVersionCheck;

        // [Get Version] command supported only since "WebCard 0.4.0".
        const checkVersion = () => {
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
            (typeof pendingVersionCheck.uid !== 'undefined')) {
            self.pendingRequests.delete(pendingVersionCheck.uid);
        }

        return {
            addon: versions[0],
            app: versions[1],
            latest: WEBCARD_VERSION
        };
    }

    // Fetches list of SmartCard readers connected to the PC.
    self.getReaders = () => self.send(1);
    self.readers = self.getReaders;

    // Handling content script (Native App) responses.
    self.responseCallback = (msg) => {
        if (typeof msg !== 'object') {
            return;
        }

        if (msg.e) {
            switch (msg.e) {
                // [Reject any pending promises]
                case (-1): {
                    self.pendingRequests.forEach((request) => {
                        request.reject();
                    });

                    self.pendingRequests.clear();
                    break;
                }

                // [Card inserted]
                case 1: {
                    _readerList[msg.r].atr = msg.d;
                    self.cardInserted?.(_readerList[msg.r]);
                    break;
                }

                // [Card removed]
                case 2: {
                    _readerList[msg.r].atr = "";
                    self.cardRemoved?.(_readerList[msg.r]);
                    break;
                }

                // [New readers connected]
                case 3: {
                    self.readersConnected?.(msg.n);
                    break;
                }

                // [Any reader unplugged]
                case 4: {
                    self.readersDisconnected?.(msg.n);
                    break;
                }
            }

            return;
        }

        if (!msg.i) {
            // Impossible to resolve!
            return;
        }

        let request = self.pendingRequests.get(msg.i);
        if (!request) {
            // No match for given UID!
            return;
        }

        if (msg.incomplete) {
            // Response marked as incomplete
            // (error on the Native App's side).
            request.reject();
        } else switch (request.c) {
            // [List readers]
            case 1: {
                if (msg.d) {
                    _readerList = [];

                    msg.d.forEach((element, index) => {
                        _readerList.push(
                            new Reader(index, element.n, element.a));
                    });

                    request.resolve(_readerList);
                } else {
                    request.reject();
                }

                break;
            }

            // [Connect] and [Transceive]
            case 2: case 4: {
                if (msg.d) {
                    request.resolve(msg.d);
                } else {
                    request.reject();
                }

                break;
            }

            // [Get Version]
            case 10: {
                request.resolve(msg);
                break;
            }

            // [Disconnect] or an unknown command (possibly just a ping)
            default: {
                request.resolve();
            }
        }

        self.pendingRequests.delete(msg.i);
    }
}

/******************************************************************************/
// Called after any `window.postMessage()`
// (originating either from the content script or the page scripts).
function windowMessageCallback(event) {
    // We want to only accept messages from ourselves
    // (content script => page script).
    if ((event.source !== window) ||
        (event.origin !== window.location.origin) ||
        (typeof event?.data?.webcard !== 'string')) {
        return;
    }

    if (event.data.webcard === 'response') {
        navigator.webcard.responseCallback(event.data);
    }
}

/******************************************************************************/
// Initialize WebCard - installs navigator.webcard
function initWebCard(options = {}) {
    if (typeof window === 'undefined' || typeof navigator === 'undefined') {
        return;
    }

    window.addEventListener('message', windowMessageCallback, false);

    if (typeof navigator.webcard === 'undefined') {
        navigator.webcard = new WebCard(options);
        if (navigator.webcard.isReady) {
            navigator.webcard.getReaders();
        }
    }
}

/******************************************************************************/
// Auto-initialize when loaded as script tag or ES module in browser
initWebCard();

// ES Module exports for bundler/CDN usage
export { WebCard, Reader, initWebCard, WEBCARD_VERSION, WEBCARD_HOMEPAGE };
export default initWebCard;

/******************************************************************************/
