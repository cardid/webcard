const esbuild = require('esbuild');
const fs = require('fs');
const path = require('path');

// Ensure dist directory exists
const distDir = path.join(__dirname, 'dist');
if (!fs.existsSync(distDir)) {
    fs.mkdirSync(distDir);
}

async function build() {
    // ES Module (unminified, for debugging)
    await esbuild.build({
        entryPoints: ['src/webcard.js'],
        bundle: true,
        format: 'esm',
        outfile: 'dist/webcard.js',
        target: ['es2020'],
        sourcemap: false,
    });
    console.log('Built: dist/webcard.js (ES module, unminified)');

    // ES Module (minified, primary CDN target)
    await esbuild.build({
        entryPoints: ['src/webcard.js'],
        bundle: true,
        format: 'esm',
        minify: true,
        outfile: 'dist/webcard.min.js',
        target: ['es2020'],
        sourcemap: false,
    });
    console.log('Built: dist/webcard.min.js (ES module, minified)');

    // IIFE (browser global version, backward compatible)
    await esbuild.build({
        entryPoints: ['src/webcard.js'],
        bundle: true,
        format: 'iife',
        globalName: 'WebCardModule',
        minify: true,
        outfile: 'dist/webcard.iife.js',
        target: ['es2020'],
        sourcemap: false,
    });
    console.log('Built: dist/webcard.iife.js (IIFE, browser global)');

    // Create TypeScript declaration file
    const dtsContent = `// Type definitions for @cardid/webcard
// https://webcard.cardid.org

export const WEBCARD_VERSION: string;
export const WEBCARD_HOMEPAGE: string;

export interface Reader {
    index: number;
    name: string;
    atr: string;
    connected: boolean | undefined;
    connect(shared?: boolean): Promise<string>;
    disconnect(): Promise<void>;
    transceive(apdu: string): Promise<string>;
}

export interface WebCardVersions {
    addon: string;
    app: string;
    latest: string;
}

export interface WebCard {
    isReady: boolean;
    installerUrl: string;
    pendingRequests: Map<string, unknown>;
    readers(): Promise<Reader[]>;
    getVersions(): Promise<WebCardVersions>;
    send(cmdIdx: number, otherParams?: object): Promise<unknown>;
    sendEx(cmdIdx: number, otherParams?: object): { promise: Promise<unknown>; uid: string | undefined };
    responseCallback(msg: object): void;
    cardInserted?: (reader: Reader) => void;
    cardRemoved?: (reader: Reader) => void;
    readersConnected?: (count: number) => void;
    readersDisconnected?: (count: number) => void;
}

export class Reader {
    constructor(index: number, name: string, atr: string);
}

export class WebCard {
    constructor();
}

export function initWebCard(): void;
export default initWebCard;

declare global {
    interface Navigator {
        webcard: WebCard;
    }
}
`;
    fs.writeFileSync(path.join(distDir, 'webcard.d.ts'), dtsContent);
    console.log('Built: dist/webcard.d.ts (TypeScript declarations)');

    console.log('\nBuild complete!');
}

build().catch((err) => {
    console.error(err);
    process.exit(1);
});
