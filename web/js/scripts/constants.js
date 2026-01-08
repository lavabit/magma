/**
 * magma.js
 *
 * Provide any constants here
 *
 * This script must be inserted before any method definitions!
 * Handled in build-js.php
 */
var magma = magma || {};

magma.animationSpeed = "fast";
// Use real portal endpoint by default. Set window.MAGMA_USE_MOCK = true before loading to use mock interface.
magma.portalUrl = (typeof window.MAGMA_USE_MOCK !== 'undefined' && window.MAGMA_USE_MOCK) ? '/portal/mockiface' : '/portal/camel';

// TODO: better way to get table header height
// 27px + 1px border
magma.tableHeaderHeight = 28;
