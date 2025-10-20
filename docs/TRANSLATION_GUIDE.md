# Multi-Language Documentation System

## Overview

The R-Engine ECS documentation now supports both **English** (default) and **French** languages with an easy-to-use language switcher.

## Features

- 🌐 **Bilingual Support**: English and French
- 🔄 **Dynamic Language Switching**: Change language without page reload
- 💾 **Persistent Preference**: Your language choice is saved in localStorage
- 🎯 **Easy to Extend**: Simple JSON-based translation system

## How It Works

### 1. Language Selector

A dropdown menu in the sidebar allows users to switch between languages:

```html
<div class="language-selector">
    <label data-i18n="langSelector.label">Language:</label>
    <select id="languageSelect">
        <option value="en">English</option>
        <option value="fr">Français</option>
    </select>
</div>
```

### 2. Translation System

Translations are managed through three files:

#### `translations.js`
Contains all translation strings organized by language:

```javascript
const translations = {
    en: {
        nav: {
            introduction: "Introduction",
            architecture: "Architecture",
            // ...
        }
    },
    fr: {
        nav: {
            introduction: "Introduction",
            architecture: "Architecture",
            // ...
        }
    }
};
```

#### `index.html`
Elements use `data-i18n` attributes to mark translatable content:

```html
<h1 data-i18n="introduction.title">Introduction to R-Engine ECS</h1>
<a href="#introduction" data-i18n="nav.introduction">Introduction</a>
```

#### `script.js`
Handles language switching logic:

```javascript
function setLanguage(lang) {
    // Updates all elements with data-i18n attributes
    // Saves preference to localStorage
    // Updates HTML lang attribute
}
```

## Usage

### For End Users

1. Open the documentation at `http://localhost:8000`
2. Look for the language selector in the sidebar (below the logo)
3. Select your preferred language
4. All translatable content updates instantly
5. Your choice is automatically saved for next visit

### For Developers/Contributors

#### Adding New Translations

1. **Add the translation key to `translations.js`**:

```javascript
const translations = {
    en: {
        mySection: {
            title: "My Section Title",
            description: "My description"
        }
    },
    fr: {
        mySection: {
            title: "Titre de Ma Section",
            description: "Ma description"
        }
    }
};
```

2. **Use the key in your HTML**:

```html
<h2 data-i18n="mySection.title">My Section Title</h2>
<p data-i18n="mySection.description">My description</p>
```

3. **For HTML content, use `data-i18n-html`**:

```html
<p data-i18n="mySection.description" data-i18n-html>
    Content with <strong>HTML</strong> tags
</p>
```

#### Adding a New Language

1. Add the language code to the selector in `index.html`:

```html
<select id="languageSelect">
    <option value="en">English</option>
    <option value="fr">Français</option>
    <option value="es">Español</option> <!-- New language -->
</select>
```

2. Add translations in `translations.js`:

```javascript
const translations = {
    en: { /* ... */ },
    fr: { /* ... */ },
    es: {
        nav: {
            introduction: "Introducción",
            // ...
        }
    }
};
```

## Translation Keys Structure

The translation keys follow a hierarchical structure:

```
translations
├── en/fr
    ├── langSelector
    │   ├── label
    │   ├── en
    │   └── fr
    ├── logo
    │   ├── title
    │   └── subtitle
    ├── nav
    │   ├── introduction
    │   ├── architecture
    │   ├── coreConcepts
    │   └── ...
    ├── introduction
    │   ├── title
    │   ├── lead
    │   └── ...
    └── ...
```

## Current Translation Coverage

✅ **Fully Translated**:
- Navigation menu
- Logo and header
- Language selector
- Footer
- Section titles
- Button labels

⏳ **Partially Translated**:
- Some code examples retain English comments
- Technical terms remain in English (Entity, Component, System, etc.)

📝 **English Only**:
- Code snippets
- API documentation tables (technical content)

## Technical Details

### Language Detection

1. Check localStorage for `preferredLanguage`
2. If not found, default to English (`en`)
3. Apply language immediately on page load

### Performance

- No AJAX requests needed
- All translations loaded once with the page
- Instant language switching
- Minimal performance impact

### Browser Support

- Modern browsers with ES6+ support
- localStorage API required
- Works offline (no external dependencies)

## Customization

### Changing Default Language

In `script.js`, change the default language:

```javascript
let currentLanguage = 'fr'; // Change from 'en' to 'fr'
const savedLang = localStorage.getItem('preferredLanguage') || 'fr';
```

### Styling the Language Selector

Modify `.language-selector` in `style.css`:

```css
.language-selector {
    /* Your custom styles */
}
```

## Troubleshooting

### Language not switching?

1. Check browser console for errors
2. Ensure `translations.js` is loaded before `script.js`
3. Verify translation keys match between HTML and JS

### Missing translations?

1. Check that the key exists in `translations.js`
2. Verify the `data-i18n` attribute value is correct
3. Ensure the key path is correct (e.g., `nav.introduction`)

### Styles broken after language change?

1. Clear browser cache
2. Check for CSS conflicts
3. Inspect element to verify classes are applied

## Future Enhancements

Potential improvements for the translation system:

- [ ] Auto-detect browser language
- [ ] Add more languages (Spanish, German, Japanese, etc.)
- [ ] Translate code comments
- [ ] RTL language support
- [ ] Export/import translations as JSON
- [ ] Translation management interface
- [ ] Crowdsourced translations

## Files Modified

- `index.html` - Added language selector and data-i18n attributes
- `style.css` - Added language selector styling
- `script.js` - Added translation logic
- `translations.js` - New file with all translations
- `TRANSLATION_GUIDE.md` - This file

## License

This translation system is part of the R-Engine project.

---

**Last Updated**: 2025-10-20  
**Version**: 1.0.0  
**Languages**: English, Français