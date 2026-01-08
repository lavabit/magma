#!/usr/bin/env node
/**
 * Library Update Script
 *
 * Downloads updated versions of frontend libraries.
 * Run with: npm run update-libs
 */

const https = require('https');
const fs = require('fs');
const path = require('path');

const LIBS_DIR = path.resolve(__dirname, '../js/plugins');
const JQUERY_DIR = path.resolve(__dirname, '../../res/pages/webmail/js');

// Library CDN URLs
const LIBRARIES = {
    'jquery': {
        url: 'https://code.jquery.com/jquery-3.7.1.min.js',
        filename: 'jquery.js',
        dest: JQUERY_DIR
    },
    'jquery-migrate': {
        url: 'https://code.jquery.com/jquery-migrate-3.4.1.min.js',
        filename: 'jquery-migrate.js',
        dest: JQUERY_DIR,
        notes: 'Helps migrate from jQuery 1.x/2.x to 3.x'
    },
    'jquery-ui': {
        url: 'https://code.jquery.com/ui/1.13.3/jquery-ui.min.js',
        filename: 'jquery.ui.js',
        dest: LIBS_DIR
    },
    'datatables': {
        url: 'https://cdn.datatables.net/1.13.8/js/jquery.dataTables.min.js',
        filename: 'jquery.dataTables.js',
        dest: LIBS_DIR
    }
};

// Console colors
const colors = {
    reset: '\x1b[0m',
    cyan: '\x1b[36m',
    green: '\x1b[32m',
    red: '\x1b[31m',
    yellow: '\x1b[33m'
};

function log(color, message) {
    console.log(`${colors[color]}${message}${colors.reset}`);
}

function download(url, dest) {
    return new Promise((resolve, reject) => {
        const file = fs.createWriteStream(dest);

        https.get(url, (response) => {
            // Handle redirects
            if (response.statusCode === 301 || response.statusCode === 302) {
                download(response.headers.location, dest)
                    .then(resolve)
                    .catch(reject);
                return;
            }

            if (response.statusCode !== 200) {
                reject(new Error(`HTTP ${response.statusCode}`));
                return;
            }

            response.pipe(file);
            file.on('finish', () => {
                file.close();
                resolve();
            });
        }).on('error', (err) => {
            fs.unlink(dest, () => {}); // Delete partial file
            reject(err);
        });
    });
}

async function backupFile(filepath) {
    if (fs.existsSync(filepath)) {
        const backupPath = filepath + '.backup';
        fs.copyFileSync(filepath, backupPath);
        return backupPath;
    }
    return null;
}

async function updateLibrary(name, config) {
    const destPath = path.join(config.dest, config.filename);

    log('cyan', `\nUpdating ${name}...`);

    // Backup existing file
    const backupPath = await backupFile(destPath);
    if (backupPath) {
        log('yellow', `  Backed up to ${path.basename(backupPath)}`);
    }

    try {
        await download(config.url, destPath);
        const stats = fs.statSync(destPath);
        log('green', `  Downloaded ${config.filename} (${(stats.size / 1024).toFixed(1)} KB)`);

        if (config.notes) {
            log('yellow', `  Note: ${config.notes}`);
        }
    } catch (err) {
        log('red', `  Failed: ${err.message}`);

        // Restore backup
        if (backupPath && fs.existsSync(backupPath)) {
            fs.copyFileSync(backupPath, destPath);
            log('yellow', `  Restored from backup`);
        }
        throw err;
    }
}

async function main() {
    const args = process.argv.slice(2);

    console.log('\n' + '='.repeat(60));
    log('cyan', 'Library Update Script');
    console.log('='.repeat(60));

    if (args.length === 0) {
        console.log('\nUsage: npm run update-libs -- <library>');
        console.log('\nAvailable libraries:');
        Object.entries(LIBRARIES).forEach(([name, config]) => {
            console.log(`  ${name.padEnd(15)} -> ${config.filename}`);
            if (config.notes) {
                console.log(`                    (${config.notes})`);
            }
        });
        console.log('\n  all            -> Update all libraries');
        console.log('\nExample: npm run update-libs -- jquery');
        console.log('         npm run update-libs -- all');
        console.log('\n' + '='.repeat(60) + '\n');
        return;
    }

    const toUpdate = args[0] === 'all' ? Object.keys(LIBRARIES) : args;

    for (const name of toUpdate) {
        if (!LIBRARIES[name]) {
            log('red', `Unknown library: ${name}`);
            continue;
        }

        try {
            await updateLibrary(name, LIBRARIES[name]);
        } catch (err) {
            log('red', `Failed to update ${name}`);
        }
    }

    console.log('\n' + '='.repeat(60));
    log('green', 'Update complete!');
    log('yellow', 'Remember to test thoroughly after updating libraries.');
    log('yellow', 'Run: npm run build && open test/test.html');
    console.log('='.repeat(60) + '\n');
}

main().catch(err => {
    log('red', `Error: ${err.message}`);
    process.exit(1);
});
