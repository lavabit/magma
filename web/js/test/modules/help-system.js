/**
 * Help System Tests
 *
 * Tests for help navigation, topics, content viewer,
 * keyboard shortcuts, FAQ, quick tips, and tooltips
 */

module('help-system');

test('helpSystem exists', function() {
    ok(magma.helpSystem, 'helpSystem exists');
    ok(typeof magma.helpSystem.create === 'function', 'create exists');
    ok(typeof magma.helpSystem.createNavigation === 'function', 'createNavigation exists');
    ok(typeof magma.helpSystem.createTopicList === 'function', 'createTopicList exists');
    ok(typeof magma.helpSystem.createContentViewer === 'function', 'createContentViewer exists');
    ok(typeof magma.helpSystem.createShortcutsPanel === 'function', 'createShortcutsPanel exists');
    ok(typeof magma.helpSystem.createFAQPanel === 'function', 'createFAQPanel exists');
    ok(typeof magma.helpSystem.createQuickTips === 'function', 'createQuickTips exists');
    ok(typeof magma.helpSystem.createSearch === 'function', 'createSearch exists');
    ok(typeof magma.helpSystem.createTooltip === 'function', 'createTooltip exists');
    ok(typeof magma.helpSystem.createGettingStarted === 'function', 'createGettingStarted exists');
    ok(magma.helpSystem.defaultShortcuts, 'defaultShortcuts exists');
    ok(magma.helpSystem.defaultFAQ, 'defaultFAQ exists');
    ok(magma.helpSystem.defaultTips, 'defaultTips exists');
});

// Default shortcuts tests
test('defaultShortcuts includes expected categories', function() {
    var shortcuts = magma.helpSystem.defaultShortcuts;

    ok(shortcuts.navigation, 'Has navigation shortcuts');
    ok(shortcuts.mail, 'Has mail shortcuts');
    ok(shortcuts.selection, 'Has selection shortcuts');
    ok(shortcuts.compose, 'Has compose shortcuts');
    ok(shortcuts.general, 'Has general shortcuts');
});

test('defaultShortcuts categories have required properties', function() {
    var shortcuts = magma.helpSystem.defaultShortcuts;

    for (var key in shortcuts) {
        ok(shortcuts[key].label, key + ' has label');
        ok(shortcuts[key].shortcuts, key + ' has shortcuts array');
        ok(shortcuts[key].shortcuts.length > 0, key + ' has shortcuts');
    }
});

// Default FAQ tests
test('defaultFAQ has items', function() {
    ok($.isArray(magma.helpSystem.defaultFAQ), 'defaultFAQ is array');
    ok(magma.helpSystem.defaultFAQ.length > 0, 'defaultFAQ has items');
});

test('defaultFAQ items have required properties', function() {
    var faq = magma.helpSystem.defaultFAQ;

    $.each(faq, function(i, item) {
        ok(item.question, 'Item ' + i + ' has question');
        ok(item.answer, 'Item ' + i + ' has answer');
    });
});

// Default tips tests
test('defaultTips has items', function() {
    ok($.isArray(magma.helpSystem.defaultTips), 'defaultTips is array');
    ok(magma.helpSystem.defaultTips.length > 0, 'defaultTips has items');
});

// Navigation tests
test('createNavigation returns navigation API', function() {
    var nav = magma.helpSystem.createNavigation();

    ok(nav.element.length, 'Has element');
    ok(typeof nav.setCategories === 'function', 'Has setCategories');
    ok(typeof nav.setActive === 'function', 'Has setActive');
    ok(typeof nav.getActive === 'function', 'Has getActive');
});

test('createNavigation has list', function() {
    var nav = magma.helpSystem.createNavigation();

    ok(nav.element.find('.mgm-help-nav-list').length, 'Has list');
});

test('createNavigation renders categories', function() {
    var nav = magma.helpSystem.createNavigation({
        categories: [
            { id: 1, label: 'Getting Started', icon: '&#128218;' },
            { id: 2, label: 'Email', icon: '&#9993;' },
            { id: 3, label: 'Contacts', icon: '&#128101;' }
        ]
    });

    var items = nav.element.find('.mgm-help-nav-item');
    equal(items.length, 3, 'Has 3 categories');
});

test('createNavigation setActive updates active category', function() {
    var nav = magma.helpSystem.createNavigation({
        categories: [
            { id: 1, label: 'Category 1' },
            { id: 2, label: 'Category 2' }
        ]
    });

    nav.setActive(2);
    equal(nav.getActive(), 2, 'Active category updated');
});

test('createNavigation calls onCategorySelect', function() {
    expect(1);

    var selectedCategory = null;
    var nav = magma.helpSystem.createNavigation({
        categories: [
            { id: 1, label: 'Test Category' }
        ],
        onCategorySelect: function(category) {
            selectedCategory = category;
        }
    });

    nav.element.find('.mgm-help-nav-link').first().click();
    equal(selectedCategory.id, 1, 'onCategorySelect called');
});

// Topic list tests
test('createTopicList returns topic list API', function() {
    var topics = magma.helpSystem.createTopicList();

    ok(topics.element.length, 'Has element');
    ok(typeof topics.setTopics === 'function', 'Has setTopics');
    ok(typeof topics.setActive === 'function', 'Has setActive');
    ok(typeof topics.getActive === 'function', 'Has getActive');
    ok(typeof topics.clear === 'function', 'Has clear');
});

test('createTopicList has list', function() {
    var topics = magma.helpSystem.createTopicList();

    ok(topics.element.find('.mgm-help-topics-list').length, 'Has list');
});

test('createTopicList renders topics', function() {
    var topics = magma.helpSystem.createTopicList({
        topics: [
            { id: 1, name: 'Topic 1' },
            { id: 2, name: 'Topic 2' }
        ]
    });

    var items = topics.element.find('.mgm-help-topic-item');
    equal(items.length, 2, 'Has 2 topics');
});

test('createTopicList shows empty state', function() {
    var topics = magma.helpSystem.createTopicList({
        topics: []
    });

    ok(topics.element.find('.mgm-help-topics-empty:visible').length, 'Shows empty state');
});

test('createTopicList clear removes topics', function() {
    var topics = magma.helpSystem.createTopicList({
        topics: [
            { id: 1, name: 'Topic 1' }
        ]
    });

    topics.clear();
    equal(topics.element.find('.mgm-help-topic-item').length, 0, 'Topics cleared');
});

// Content viewer tests
test('createContentViewer returns content viewer API', function() {
    var viewer = magma.helpSystem.createContentViewer();

    ok(viewer.element.length, 'Has element');
    ok(typeof viewer.setContent === 'function', 'Has setContent');
    ok(typeof viewer.setBreadcrumb === 'function', 'Has setBreadcrumb');
    ok(typeof viewer.clear === 'function', 'Has clear');
});

test('createContentViewer has content body', function() {
    var viewer = magma.helpSystem.createContentViewer();

    ok(viewer.element.find('.mgm-help-content-body').length, 'Has content body');
});

test('createContentViewer setContent displays content', function() {
    var viewer = magma.helpSystem.createContentViewer();

    viewer.setContent('Test Title', '<p>Test content</p>');

    equal(viewer.element.find('.mgm-help-content-title').text(), 'Test Title', 'Shows title');
    ok(viewer.element.find('.mgm-help-content-body').html().indexOf('Test content') >= 0, 'Shows content');
});

test('createContentViewer clear removes content', function() {
    var viewer = magma.helpSystem.createContentViewer();

    viewer.setContent('Title', 'Content');
    viewer.clear();

    ok(viewer.element.find('.mgm-help-content-empty:visible').length, 'Shows empty state');
});

// Keyboard shortcuts panel tests
test('createShortcutsPanel returns shortcuts API', function() {
    var panel = magma.helpSystem.createShortcutsPanel();

    ok(panel.element.length, 'Has element');
    ok(typeof panel.setShortcuts === 'function', 'Has setShortcuts');
    ok(typeof panel.show === 'function', 'Has show');
    ok(typeof panel.hide === 'function', 'Has hide');
    ok(typeof panel.toggle === 'function', 'Has toggle');
});

test('createShortcutsPanel has header', function() {
    var panel = magma.helpSystem.createShortcutsPanel();

    ok(panel.element.find('.mgm-help-shortcuts-header').length, 'Has header');
});

test('createShortcutsPanel has content', function() {
    var panel = magma.helpSystem.createShortcutsPanel();

    ok(panel.element.find('.mgm-help-shortcuts-content').length, 'Has content');
});

test('createShortcutsPanel renders shortcuts', function() {
    var panel = magma.helpSystem.createShortcutsPanel();

    var sections = panel.element.find('.mgm-help-shortcuts-section');
    ok(sections.length > 0, 'Has shortcut sections');
});

test('createShortcutsPanel show/hide works', function() {
    var panel = magma.helpSystem.createShortcutsPanel();

    panel.hide();
    ok(!panel.element.is(':visible'), 'Panel hidden');

    panel.show();
    ok(panel.element.is(':visible'), 'Panel visible');
});

// FAQ panel tests
test('createFAQPanel returns FAQ API', function() {
    var faq = magma.helpSystem.createFAQPanel();

    ok(faq.element.length, 'Has element');
    ok(typeof faq.setItems === 'function', 'Has setItems');
    ok(typeof faq.expandAll === 'function', 'Has expandAll');
    ok(typeof faq.collapseAll === 'function', 'Has collapseAll');
});

test('createFAQPanel has list', function() {
    var faq = magma.helpSystem.createFAQPanel();

    ok(faq.element.find('.mgm-help-faq-list').length, 'Has list');
});

test('createFAQPanel renders default FAQ items', function() {
    var faq = magma.helpSystem.createFAQPanel();

    var items = faq.element.find('.mgm-help-faq-item');
    equal(items.length, magma.helpSystem.defaultFAQ.length, 'Has default FAQ items');
});

test('createFAQPanel custom items', function() {
    var faq = magma.helpSystem.createFAQPanel({
        items: [
            { question: 'Q1', answer: 'A1' },
            { question: 'Q2', answer: 'A2' }
        ]
    });

    var items = faq.element.find('.mgm-help-faq-item');
    equal(items.length, 2, 'Has 2 custom items');
});

test('createFAQPanel expands on click', function() {
    var faq = magma.helpSystem.createFAQPanel({
        items: [{ question: 'Test', answer: 'Answer' }]
    });

    var $item = faq.element.find('.mgm-help-faq-item').first();
    ok(!$item.hasClass('expanded'), 'Not expanded initially');

    $item.find('.mgm-help-faq-question').click();
    ok($item.hasClass('expanded'), 'Expanded after click');
});

// Quick tips tests
test('createQuickTips returns tips API', function() {
    var tips = magma.helpSystem.createQuickTips({ autoRotate: false });

    ok(tips.element.length, 'Has element');
    ok(typeof tips.setTips === 'function', 'Has setTips');
    ok(typeof tips.show === 'function', 'Has show');
    ok(typeof tips.hide === 'function', 'Has hide');
    ok(typeof tips.next === 'function', 'Has next');
    ok(typeof tips.prev === 'function', 'Has prev');
    ok(typeof tips.destroy === 'function', 'Has destroy');

    tips.destroy();
});

test('createQuickTips has content', function() {
    var tips = magma.helpSystem.createQuickTips({ autoRotate: false });

    ok(tips.element.find('.mgm-help-tips-content').length, 'Has content');

    tips.destroy();
});

test('createQuickTips displays tips', function() {
    var tips = magma.helpSystem.createQuickTips({
        autoRotate: false,
        tips: ['Tip 1', 'Tip 2', 'Tip 3']
    });

    ok(tips.element.find('.mgm-help-tips-text').text().length > 0, 'Displays tip text');

    tips.destroy();
});

test('createQuickTips next/prev navigation', function() {
    var tips = magma.helpSystem.createQuickTips({
        autoRotate: false,
        tips: ['Tip 1', 'Tip 2', 'Tip 3']
    });

    var firstTip = tips.element.find('.mgm-help-tips-text').text();
    tips.next();
    var secondTip = tips.element.find('.mgm-help-tips-text').text();

    notEqual(firstTip, secondTip, 'Next changes tip');

    tips.destroy();
});

// Search tests
test('createSearch returns search API', function() {
    var search = magma.helpSystem.createSearch();

    ok(search.element.length, 'Has element');
    ok(typeof search.getValue === 'function', 'Has getValue');
    ok(typeof search.setValue === 'function', 'Has setValue');
    ok(typeof search.clear === 'function', 'Has clear');
    ok(typeof search.focus === 'function', 'Has focus');
});

test('createSearch has input', function() {
    var search = magma.helpSystem.createSearch();

    ok(search.element.find('.mgm-help-search-input').length, 'Has input');
});

test('createSearch getValue returns input value', function() {
    var search = magma.helpSystem.createSearch();

    search.element.find('.mgm-help-search-input').val('test query');
    equal(search.getValue(), 'test query', 'Returns input value');
});

test('createSearch setValue sets input value', function() {
    var search = magma.helpSystem.createSearch();

    search.setValue('new query');
    equal(search.element.find('.mgm-help-search-input').val(), 'new query', 'Sets input value');
});

test('createSearch clear empties input', function() {
    var search = magma.helpSystem.createSearch();

    search.setValue('test');
    search.clear();
    equal(search.getValue(), '', 'Input cleared');
});

// Tooltip tests
test('createTooltip returns tooltip API', function() {
    var tooltip = magma.helpSystem.createTooltip();

    ok(tooltip.element.length, 'Has element');
    ok(typeof tooltip.show === 'function', 'Has show');
    ok(typeof tooltip.hide === 'function', 'Has hide');
    ok(typeof tooltip.setContent === 'function', 'Has setContent');
    ok(typeof tooltip.destroy === 'function', 'Has destroy');

    tooltip.destroy();
});

test('createTooltip has content area', function() {
    var tooltip = magma.helpSystem.createTooltip();

    ok(tooltip.element.find('.mgm-help-tooltip-content').length, 'Has content');

    tooltip.destroy();
});

test('createTooltip setContent updates content', function() {
    var tooltip = magma.helpSystem.createTooltip({ content: 'Initial' });

    tooltip.setContent('Updated content');
    equal(tooltip.element.find('.mgm-help-tooltip-content').text(), 'Updated content', 'Content updated');

    tooltip.destroy();
});

// Getting started guide tests
test('createGettingStarted returns guide API', function() {
    var guide = magma.helpSystem.createGettingStarted();

    ok(guide.element.length, 'Has element');
    ok(typeof guide.show === 'function', 'Has show');
    ok(typeof guide.hide === 'function', 'Has hide');
    ok(typeof guide.goToStep === 'function', 'Has goToStep');
    ok(typeof guide.destroy === 'function', 'Has destroy');

    guide.destroy();
});

test('createGettingStarted has modal', function() {
    var guide = magma.helpSystem.createGettingStarted();

    ok(guide.element.find('.mgm-help-getting-started-modal').length, 'Has modal');

    guide.destroy();
});

test('createGettingStarted has progress dots', function() {
    var guide = magma.helpSystem.createGettingStarted();

    var dots = guide.element.find('.mgm-help-getting-started-dot');
    ok(dots.length > 0, 'Has progress dots');

    guide.destroy();
});

test('createGettingStarted goToStep changes step', function() {
    var guide = magma.helpSystem.createGettingStarted();

    var firstTitle = guide.element.find('.mgm-help-getting-started-title').text();
    guide.goToStep(1);
    var secondTitle = guide.element.find('.mgm-help-getting-started-title').text();

    notEqual(firstTitle, secondTitle, 'Step changed');

    guide.destroy();
});

// Main component tests
test('create returns help system API', function() {
    var $container = $('<div>');
    var help = magma.helpSystem.create({
        container: $container
    });

    ok(help.element.length, 'Has element');
    ok(typeof help.setCategories === 'function', 'Has setCategories');
    ok(typeof help.loadCategory === 'function', 'Has loadCategory');
    ok(typeof help.loadTopic === 'function', 'Has loadTopic');
    ok(typeof help.showShortcuts === 'function', 'Has showShortcuts');
    ok(typeof help.hideShortcuts === 'function', 'Has hideShortcuts');
    ok(typeof help.showFAQ === 'function', 'Has showFAQ');
    ok(typeof help.hideFAQ === 'function', 'Has hideFAQ');
    ok(typeof help.showTips === 'function', 'Has showTips');
    ok(typeof help.hideTips === 'function', 'Has hideTips');
    ok(typeof help.destroy === 'function', 'Has destroy');

    help.destroy();
});

test('create has sidebar', function() {
    var $container = $('<div>');
    var help = magma.helpSystem.create({
        container: $container
    });

    ok($container.find('.mgm-help-sidebar').length, 'Has sidebar');

    help.destroy();
});

test('create has content area', function() {
    var $container = $('<div>');
    var help = magma.helpSystem.create({
        container: $container
    });

    ok($container.find('.mgm-help-content-area').length, 'Has content area');

    help.destroy();
});

test('create getNavigation returns navigation', function() {
    var $container = $('<div>');
    var help = magma.helpSystem.create({
        container: $container,
        showNav: true
    });

    var nav = help.getNavigation();
    ok(nav, 'Returns navigation');
    ok(nav.element.length, 'Navigation has element');

    help.destroy();
});

test('create getContentViewer returns content viewer', function() {
    var $container = $('<div>');
    var help = magma.helpSystem.create({
        container: $container
    });

    var viewer = help.getContentViewer();
    ok(viewer, 'Returns content viewer');
    ok(viewer.element.length, 'Content viewer has element');

    help.destroy();
});

test('create getShortcutsPanel returns shortcuts panel', function() {
    var $container = $('<div>');
    var help = magma.helpSystem.create({
        container: $container,
        showShortcuts: true
    });

    var panel = help.getShortcutsPanel();
    ok(panel, 'Returns shortcuts panel');
    ok(panel.element.length, 'Shortcuts panel has element');

    help.destroy();
});

test('create getFAQPanel returns FAQ panel', function() {
    var $container = $('<div>');
    var help = magma.helpSystem.create({
        container: $container,
        showFAQ: true
    });

    var faq = help.getFAQPanel();
    ok(faq, 'Returns FAQ panel');
    ok(faq.element.length, 'FAQ panel has element');

    help.destroy();
});

test('create setCategories updates navigation', function() {
    var $container = $('<div>');
    var help = magma.helpSystem.create({
        container: $container,
        showNav: true
    });

    help.setCategories([
        { id: 1, label: 'New Category' }
    ]);

    var items = $container.find('.mgm-help-nav-item');
    equal(items.length, 1, 'Navigation updated');

    help.destroy();
});

test('destroy removes element', function() {
    var $container = $('<div>');
    var help = magma.helpSystem.create({
        container: $container
    });

    ok($container.find('.mgm-help-system').length, 'Help system in DOM');

    help.destroy();
    equal($container.find('.mgm-help-system').length, 0, 'Help system removed');
});
