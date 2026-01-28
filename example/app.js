/**
 * WebCard Example Application
 * Demonstrates smart card reader access from a web page
 */
'use strict';

// State
let readers = [];
let selectedReader = null;

// DOM elements
const readerList = document.getElementById('reader-list');
const apduInput = document.getElementById('apdu-input');
const sendBtn = document.getElementById('send-btn');
const output = document.getElementById('output');

/**
 * Render the reader list UI
 */
function renderReaderList() {
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

        // Add card presence class
        if (reader.atr) {
            li.classList.add('has-card');
        }

        // Add selected class
        if (selectedReader === index) {
            li.classList.add('selected');
        }

        // Reader name
        const nameDiv = document.createElement('div');
        nameDiv.className = 'reader-name';
        nameDiv.textContent = reader.name;
        li.appendChild(nameDiv);

        // Status line
        const statusDiv = document.createElement('div');
        statusDiv.className = 'reader-status';
        if (reader.atr) {
            statusDiv.textContent = 'Card present';
            // Show ATR
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

        // Click handler to select reader
        li.addEventListener('click', () => selectReader(index));

        readerList.appendChild(li);
    });
}

/**
 * Select a reader for APDU communication
 */
function selectReader(index) {
    selectedReader = index;
    renderReaderList();
    updateSendButton();
}

/**
 * Update send button enabled state
 */
function updateSendButton() {
    const hasReader = selectedReader !== null && readers[selectedReader];
    const hasCard = hasReader && readers[selectedReader].atr;
    sendBtn.disabled = !hasCard;
}

/**
 * Fetch and display the list of readers
 */
async function refreshReaders() {
    try {
        readers = await navigator.webcard.readers();
        console.log(`${readers.length} reader(s) detected`);

        // Clear selection if reader no longer exists
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
 * Send APDU commands to the selected reader
 */
async function sendCommands() {
    if (selectedReader === null) return;

    const reader = readers[selectedReader];
    if (!reader || !reader.atr) return;

    // Parse APDU commands from input (one per line, ignore empty lines)
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
        // Connect to card (shared mode)
        const atr = await reader.connect(true);
        output.textContent += `Connected. ATR: ${atr}\n\n`;

        // Send each command
        for (const apdu of commands) {
            output.textContent += `> ${apdu}\n`;
            try {
                const response = await reader.transceive(apdu);
                output.textContent += `< ${response}\n\n`;
            } catch (err) {
                output.textContent += `< Error: Command failed\n\n`;
            }
        }

        // Disconnect
        await reader.disconnect();
        output.textContent += 'Disconnected.';

    } catch (err) {
        output.textContent += `Error: ${err.message || 'Connection failed'}`;
    }

    updateSendButton();
}

/**
 * Set up WebCard event handlers
 */
function setupEventHandlers() {
    // Card inserted into a reader
    navigator.webcard.cardInserted = (reader) => {
        console.log(`Card inserted in ${reader.name}, ATR: ${reader.atr}`);
        renderReaderList();
        updateSendButton();
    };

    // Card removed from a reader
    navigator.webcard.cardRemoved = (reader) => {
        console.log(`Card removed from ${reader.name}`);
        renderReaderList();
        updateSendButton();
    };

    // New reader(s) connected (USB plugged in)
    navigator.webcard.readersConnected = (count) => {
        console.log(`${count} reader(s) connected`);
        refreshReaders();
    };

    // Reader(s) disconnected (USB unplugged)
    navigator.webcard.readersDisconnected = (count) => {
        console.log(`${count} reader(s) disconnected`);
        refreshReaders();
    };
}

/**
 * Initialize the application
 */
function init() {
    // Set up UI event handlers
    sendBtn.addEventListener('click', sendCommands);

    // Set up WebCard event handlers
    setupEventHandlers();

    // Initial reader list fetch
    refreshReaders();
}

// Start the app when DOM is ready
if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', init);
} else {
    init();
}
