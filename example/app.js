/**
 * WebCard Example Application
 * Demonstrates smart card reader access from a web page
 */
'use strict';

// State
let readers = [];
let selectedReader = null;

// DOM elements — playground UI (may be null until activatePlayground() runs)
let readerList;
let apduInput;
let sendBtn;
let output;

/**
 * Inject a dismissable status notice into #status-notice.
 * type: 'unsupported' | 'not-installed'
 */
function showStatusNotice(type) {
    const container = document.getElementById('status-notice');
    if (!container) return;

    const isUnsupported = type === 'unsupported';

    const notice = document.createElement('div');
    notice.className = `status-notice status-notice--${isUnsupported ? 'warning' : 'info'}`;

    let bodyHtml;
    if (isUnsupported) {
        bodyHtml = `
            <strong>Unsupported browser.</strong>
            WebCard requires a desktop Chromium-based browser (Chrome, Edge, or Brave) to communicate
            with local smart card readers via Native Messaging. The developer documentation below is
            still available for reference.`;
    } else {
        const installerUrl = navigator.webcard?.installerUrl ?? 'https://webcard.cardid.org';
        bodyHtml = `
            <strong>Extension not installed.</strong>
            To use the playground below, download and install the
            <a href="${installerUrl}" class="notice-link">WebCard native component</a>,
            then reload this page. The developer documentation is available below while you set up.`;
    }

    notice.innerHTML = `
        <div class="notice-body">${bodyHtml}</div>
        <button class="notice-dismiss" aria-label="Dismiss">
            <svg width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor"
                 stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
                <line x1="18" y1="6" x2="6" y2="18"/>
                <line x1="6" y1="6" x2="18" y2="18"/>
            </svg>
        </button>`;

    notice.querySelector('.notice-dismiss').addEventListener('click', () => {
        notice.style.opacity = '0';
        setTimeout(() => container.removeChild(notice), 200);
    });

    container.appendChild(notice);
}

/**
 * Show a static message in the playground locked area without an active widget.
 */
function setPlaygroundLockedMsg(msg) {
    const el = document.getElementById('playground-locked-msg');
    if (el) el.textContent = msg;
}

/**
 * Switch the playground from the locked placeholder to the live UI.
 */
function activatePlayground() {
    const locked = document.getElementById('playground-locked');
    const ui = document.getElementById('playground-ui');
    if (locked) locked.remove();
    if (ui) ui.classList.remove('hidden');

    // Bind DOM elements now that the UI is visible
    readerList = document.getElementById('reader-list');
    apduInput  = document.getElementById('apdu-input');
    sendBtn    = document.getElementById('send-btn');
    output     = document.getElementById('output');

    if (sendBtn) {
        sendBtn.addEventListener('click', sendCommands);
    }

    setupEventHandlers();
    refreshReaders();
}

/**
 * Render the reader list UI.
 */
function renderReaderList() {
    if (!readerList) return;
    readerList.innerHTML = '';

    if (readers.length === 0) {
        const li = document.createElement('li');
        li.className = 'empty';
        li.textContent = 'No readers detected';
        readerList.appendChild(li);
        return;
    }

    readers.forEach((reader, index) => {
        const li = document.createElement('li');
        li.className = 'reader';

        if (reader.atr) li.classList.add('has-card');
        if (selectedReader === index) li.classList.add('selected');

        const nameDiv = document.createElement('div');
        nameDiv.className = 'reader-name';
        nameDiv.textContent = reader.name;

        const statusDiv = document.createElement('div');
        statusDiv.className = 'reader-status';

        if (reader.atr) {
            statusDiv.textContent = 'Card present';
            const atrDiv = document.createElement('div');
            atrDiv.className = 'reader-atr';
            atrDiv.textContent = reader.atr;
            li.appendChild(nameDiv);
            li.appendChild(statusDiv);
            li.appendChild(atrDiv);
        } else {
            statusDiv.textContent = 'No card';
            li.appendChild(nameDiv);
            li.appendChild(statusDiv);
        }

        li.addEventListener('click', () => selectReader(index));
        readerList.appendChild(li);
    });
}

/**
 * Select a reader for APDU communication.
 */
function selectReader(index) {
    selectedReader = index;
    renderReaderList();
    updateSendButton();
}

/**
 * Update the send button enabled state.
 */
function updateSendButton() {
    if (!sendBtn) return;
    const hasCard = selectedReader !== null && readers[selectedReader]?.atr;
    sendBtn.disabled = !hasCard;
}

/**
 * Fetch and display the list of readers.
 */
async function refreshReaders() {
    try {
        readers = await navigator.webcard.readers();
        console.log(`${readers.length} reader(s) detected`);

        if (selectedReader !== null && !readers[selectedReader]) {
            selectedReader = null;
        }

        renderReaderList();
        updateSendButton();
    } catch (err) {
        console.error('Failed to list readers:', err);
        readers = [];
        renderReaderList();
    }
}

/**
 * Send APDU commands to the selected reader.
 */
async function sendCommands() {
    if (selectedReader === null || !output || !apduInput) return;

    const reader = readers[selectedReader];
    if (!reader || !reader.atr) return;

    const commands = apduInput.value
        .split('\n')
        .map(line => line.trim())
        .filter(line => line && /^[0-9A-Fa-f]+$/.test(line));

    if (commands.length === 0) {
        output.textContent = 'Error: No valid APDU commands found.\nEnter hex strings, one per line.';
        return;
    }

    output.textContent = `Connecting to ${reader.name}...\n`;
    sendBtn.disabled = true;

    try {
        const startTime = Date.now();
        const atr = await reader.connect(true);
        output.textContent += `Connected. ATR: ${atr}\n\n`;

        for (const apdu of commands) {
            output.textContent += `> ${apdu}\n`;
            try {
                const response = await reader.transceive(apdu);
                output.textContent += `< ${response}\n\n`;
            } catch {
                output.textContent += `< Error: Command failed\n\n`;
            }
        }

        await reader.disconnect();
        output.textContent += `Disconnected. (${Date.now() - startTime} ms)`;
    } catch (err) {
        output.textContent += `Error: ${err?.message ?? 'Connection failed'}`;
    }

    updateSendButton();
}

/**
 * Register WebCard event handlers (card/reader hot-plug events).
 */
function setupEventHandlers() {
    navigator.webcard.cardInserted = (reader) => {
        console.log(`Card inserted in ${reader.name}, ATR: ${reader.atr}`);
        renderReaderList();
        updateSendButton();
    };

    navigator.webcard.cardRemoved = (reader) => {
        console.log(`Card removed from ${reader.name}`);
        renderReaderList();
        updateSendButton();
    };

    navigator.webcard.readersConnected = (count) => {
        console.log(`${count} reader(s) connected`);
        refreshReaders();
    };

    navigator.webcard.readersDisconnected = (count) => {
        console.log(`${count} reader(s) disconnected`);
        refreshReaders();
    };
}

/**
 * Initialise: determine WebCard state and set up the appropriate UI.
 */
function init() {
    const isSupported = navigator.webcard?.isSupported ?? false;
    const isReady     = navigator.webcard?.isReady     ?? false;

    if (!isSupported) {
        showStatusNotice('unsupported');
        setPlaygroundLockedMsg('The playground requires a desktop Chromium-based browser.');
        return;
    }

    if (!isReady) {
        showStatusNotice('not-installed');
        setPlaygroundLockedMsg('Install the WebCard extension to activate the playground.');
        return;
    }

    // Extension is ready — activate the live playground
    activatePlayground();
}

// Start when the DOM is ready
if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', init);
} else {
    init();
}
