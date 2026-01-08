#!/usr/bin/env node
/**
 * Magma Webmail Frontend Build Script
 *
 * Replaces the PHP build process with Node.js for:
 * - JavaScript concatenation
 * - HTML blocks/templates embedding
 * - Minification (production mode)
 * - Source maps generation
 * - Asset versioning for cache busting
 */

const fs = require('fs');
const path = require('path');
const crypto = require('crypto');

// Check for production mode
const isProduction = process.argv.includes('--production') || process.env.NODE_ENV === 'production';

// Paths
const projectPath = path.resolve(__dirname, '../../');
const webPath = path.resolve(__dirname, '../');
const outputPath = path.resolve(projectPath, 'res/pages/webmail/js');

const paths = {
    plugins: path.join(webPath, 'js/plugins'),
    devPlugins: path.join(webPath, 'js/plugins/dev'),
    scripts: path.join(webPath, 'js/scripts'),
    devScripts: path.join(webPath, 'js/scripts/dev'),
    blocks: path.join(webPath, 'js/blocks'),
    templates: path.join(webPath, 'js/templates'),
    output: outputPath
};

// Console colors
const colors = {
    reset: '\x1b[0m',
    cyan: '\x1b[36m',
    green: '\x1b[32m',
    red: '\x1b[31m',
    yellow: '\x1b[33m'
};

function log(color, label, message) {
    console.log(`${colors[color]}${label}:${colors.reset} ${message}`);
}

/**
 * Get all files in a directory matching a pattern
 */
function getFiles(directory, extension = '.js') {
    if (!fs.existsSync(directory)) {
        return [];
    }
    return fs.readdirSync(directory)
        .filter(file => file.endsWith(extension))
        .sort();
}

/**
 * Build HTML blocks/templates into JavaScript object properties
 */
function buildProperties(directory) {
    const files = getFiles(directory, '.html');
    const properties = {};

    files.forEach(filename => {
        const property = filename.replace('.html', '');

        // Check for invalid property names
        if (/[.\- ]/.test(property)) {
            log('red', 'Error', `File ${filename} contains improper property characters`);
            return;
        }

        let content = fs.readFileSync(path.join(directory, filename), 'utf8');

        // Remove newlines and extra spaces
        content = content.replace(/\s{2,}|\n/g, '');

        // Escape single quotes
        content = content.replace(/'/g, "\\'");

        properties[property] = content;
    });

    // Convert to JS object literal format
    const lines = Object.entries(properties)
        .map(([key, value]) => `'${key}': '${value}'`);

    return lines.join(',\n');
}

/**
 * Concatenate JavaScript files from a directory
 */
function concatJs(directory, firstFiles = [], lastFiles = []) {
    let files = getFiles(directory);
    let result = '';

    // Process first files in order
    firstFiles.forEach(filename => {
        const idx = files.indexOf(filename);
        if (idx !== -1) {
            result += fs.readFileSync(path.join(directory, filename), 'utf8');
            files.splice(idx, 1);
        } else {
            log('red', 'Error', `First file ${filename} not found in ${directory}`);
        }
    });

    // Remove last files from main array temporarily
    const lastContent = [];
    lastFiles.forEach(filename => {
        const idx = files.indexOf(filename);
        if (idx !== -1) {
            lastContent.push(fs.readFileSync(path.join(directory, filename), 'utf8'));
            files.splice(idx, 1);
        } else {
            log('red', 'Error', `Last file ${filename} not found in ${directory}`);
        }
    });

    // Process remaining files
    files.forEach(filename => {
        result += fs.readFileSync(path.join(directory, filename), 'utf8');
    });

    // Append last files
    result += lastContent.join('');

    return result;
}

/**
 * Generate content hash for cache busting
 */
function generateHash(content) {
    return crypto.createHash('md5').update(content).digest('hex').substring(0, 8);
}

/**
 * Minify JavaScript using terser
 */
async function minifyJs(code, filename) {
    try {
        const { minify } = require('terser');
        const result = await minify(code, {
            compress: {
                drop_console: false, // Keep console for debugging
                drop_debugger: true,
                dead_code: true,
                unused: true
            },
            mangle: {
                reserved: ['$', 'jQuery', 'magma', 'CKEDITOR', 'DataTable']
            },
            format: {
                comments: false
            },
            sourceMap: {
                filename: filename,
                url: `${filename}.map`
            }
        });
        return result;
    } catch (err) {
        log('red', 'Minification Error', err.message);
        return { code: code, map: null };
    }
}

/**
 * Main build function
 */
async function build() {
    console.log('\n' + '='.repeat(60));
    log('cyan', 'Build Mode', isProduction ? 'PRODUCTION (minified)' : 'DEVELOPMENT');
    console.log('='.repeat(60) + '\n');

    // Ensure output directory exists
    if (!fs.existsSync(paths.output)) {
        fs.mkdirSync(paths.output, { recursive: true });
    }

    // Build blocks and templates
    log('cyan', 'Start', 'Compiling HTML blocks and templates into JS properties');
    const blocks = buildProperties(paths.blocks);
    const templates = buildProperties(paths.templates);
    log('green', 'Done', `Built ${blocks.split('\n').length} blocks, ${templates.split('\n').length} templates`);

    // Build plugins.js
    log('cyan', 'Start', 'Concatenating plugins');
    let plugins = concatJs(paths.plugins);
    if (fs.existsSync(paths.devPlugins)) {
        plugins += concatJs(paths.devPlugins);
    }

    // Build script.js
    log('cyan', 'Start', 'Concatenating scripts');
    let scripts = concatJs(paths.scripts, [], ['application.js']);

    // Replace placeholders with blocks and templates
    scripts = scripts.replace(/\/\/\$\{blocks\}\n?/, blocks + '\n');
    scripts = scripts.replace(/\/\/\$\{tmpl\}\n?/, templates + '\n');

    // Add dev scripts (excluding in production)
    if (!isProduction && fs.existsSync(paths.devScripts)) {
        scripts += concatJs(paths.devScripts);
    }

    // Generate version info
    const buildInfo = `/* Magma Webmail Build: ${new Date().toISOString()} */\n`;
    plugins = buildInfo + plugins;
    scripts = buildInfo + scripts;

    // Minify in production mode
    if (isProduction) {
        log('cyan', 'Start', 'Minifying JavaScript (this may take a moment)');

        const [pluginsResult, scriptsResult] = await Promise.all([
            minifyJs(plugins, 'plugins.js'),
            minifyJs(scripts, 'script.js')
        ]);

        plugins = pluginsResult.code;
        scripts = scriptsResult.code;

        // Write source maps
        if (pluginsResult.map) {
            fs.writeFileSync(path.join(paths.output, 'plugins.js.map'), pluginsResult.map);
            log('green', 'Success', 'Wrote plugins.js.map');
        }
        if (scriptsResult.map) {
            fs.writeFileSync(path.join(paths.output, 'script.js.map'), scriptsResult.map);
            log('green', 'Success', 'Wrote script.js.map');
        }

        log('green', 'Done', 'Minification complete');
    }

    // Generate version hashes for cache busting
    const pluginsHash = generateHash(plugins);
    const scriptsHash = generateHash(scripts);

    // Write output files
    fs.writeFileSync(path.join(paths.output, 'plugins.js'), plugins);
    log('green', 'Success', `Saved plugins.js (${(plugins.length / 1024).toFixed(1)} KB)`);

    fs.writeFileSync(path.join(paths.output, 'script.js'), scripts);
    log('green', 'Success', `Saved script.js (${(scripts.length / 1024).toFixed(1)} KB)`);

    // Write version manifest for cache busting
    const manifest = {
        buildTime: new Date().toISOString(),
        production: isProduction,
        files: {
            'plugins.js': pluginsHash,
            'script.js': scriptsHash
        }
    };
    fs.writeFileSync(path.join(paths.output, 'manifest.json'), JSON.stringify(manifest, null, 2));
    log('green', 'Success', 'Saved manifest.json');

    // Print summary
    console.log('\n' + '='.repeat(60));
    log('green', 'Build Complete', '');
    console.log(`  plugins.js: ${(plugins.length / 1024).toFixed(1)} KB (hash: ${pluginsHash})`);
    console.log(`  script.js:  ${(scripts.length / 1024).toFixed(1)} KB (hash: ${scriptsHash})`);
    if (isProduction) {
        console.log('  Source maps generated for debugging');
    }
    console.log('='.repeat(60) + '\n');
}

// Run build
build().catch(err => {
    log('red', 'Build Failed', err.message);
    process.exit(1);
});
