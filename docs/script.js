// ========================================
// Language Management
// ========================================

let currentLanguage = 'en'; // Default language

// Initialize language on page load
document.addEventListener('DOMContentLoaded', function() {
    // Load saved language preference or default to English
    const savedLang = localStorage.getItem('preferredLanguage') || 'en';
    setLanguage(savedLang);
    
    // Setup language buttons
    const langButtons = document.querySelectorAll('.lang-btn');
    langButtons.forEach(button => {
        button.addEventListener('click', function() {
            const lang = this.getAttribute('data-lang');
            setLanguage(lang);
        });
    });
    
    // Update active button
    updateActiveButton(savedLang);
});

/**
 * Set the active language and update all translatable elements
 */
function setLanguage(lang) {
    if (!translations[lang]) {
        console.error(`Language ${lang} not found`);
        return;
    }
    
    currentLanguage = lang;
    localStorage.setItem('preferredLanguage', lang);
    
    // Update HTML lang attribute
    document.documentElement.lang = lang;
    document.body.setAttribute('data-lang', lang);
    
    // Update all elements with data-i18n attribute
    document.querySelectorAll('[data-i18n]').forEach(element => {
        const key = element.getAttribute('data-i18n');
        const translation = getNestedTranslation(translations[lang], key);
        
        if (translation) {
            // Check if the element should use innerHTML (for HTML content)
            if (element.hasAttribute('data-i18n-html') || translation.includes('<')) {
                element.innerHTML = translation;
            } else {
                element.textContent = translation;
            }
        }
    });
    
    // Show/hide language-specific content
    document.querySelectorAll('.lang-en, .lang-fr').forEach(element => {
        element.style.display = 'none';
    });
    document.querySelectorAll(`.lang-${lang}`).forEach(element => {
        element.style.display = '';
    });
    
    // Update active button
    updateActiveButton(lang);
    
    console.log(`Language changed to: ${lang}`);
}

/**
 * Update the active state of language buttons
 */
function updateActiveButton(lang) {
    document.querySelectorAll('.lang-btn').forEach(button => {
        if (button.getAttribute('data-lang') === lang) {
            button.classList.add('active');
        } else {
            button.classList.remove('active');
        }
    });
}

/**
 * Get nested translation value from object using dot notation
 * e.g., "nav.introduction" -> translations.en.nav.introduction
 */
function getNestedTranslation(obj, path) {
    return path.split('.').reduce((current, key) => current && current[key], obj);
}

// ========================================
// Smooth Scrolling & Active Link Highlighting
// ========================================

document.addEventListener('DOMContentLoaded', function() {
    
    // Get all navigation links
    const navLinks = document.querySelectorAll('.nav-menu a');
    
    // Add click event listeners for smooth scrolling
    navLinks.forEach(link => {
        link.addEventListener('click', function(e) {
            const href = this.getAttribute('href');
            
            // Only handle internal links
            if (href.startsWith('#')) {
                e.preventDefault();
                
                const targetId = href.substring(1);
                const targetElement = document.getElementById(targetId);
                
                if (targetElement) {
                    // Smooth scroll to target
                    targetElement.scrollIntoView({
                        behavior: 'smooth',
                        block: 'start'
                    });
                    
                    // Update active link
                    updateActiveLink(this);
                    
                    // Update URL without scrolling
                    history.pushState(null, null, href);
                }
            }
        });
    });
    
    // Update active link based on scroll position
    const sections = document.querySelectorAll('.section, .subsection[id]');
    
    window.addEventListener('scroll', debounce(function() {
        let current = '';
        
        sections.forEach(section => {
            const sectionTop = section.offsetTop;
            const sectionHeight = section.clientHeight;
            
            if (window.pageYOffset >= sectionTop - 100) {
                current = section.getAttribute('id');
            }
        });
        
        navLinks.forEach(link => {
            link.classList.remove('active');
            if (link.getAttribute('href') === '#' + current) {
                link.classList.add('active');
            }
        });
    }, 100));
    
    // Set initial active link based on URL hash
    if (window.location.hash) {
        const activeLink = document.querySelector(`.nav-menu a[href="${window.location.hash}"]`);
        if (activeLink) {
            updateActiveLink(activeLink);
        }
    }
});

// ========================================
// Helper Functions
// ========================================

function updateActiveLink(clickedLink) {
    document.querySelectorAll('.nav-menu a').forEach(link => {
        link.classList.remove('active');
    });
    clickedLink.classList.add('active');
}

function debounce(func, wait) {
    let timeout;
    return function executedFunction(...args) {
        const later = () => {
            clearTimeout(timeout);
            func(...args);
        };
        clearTimeout(timeout);
        timeout = setTimeout(later, wait);
    };
}

// ========================================
// Mobile Menu Toggle
// ========================================

// Add a menu toggle button for mobile
function createMobileMenuToggle() {
    if (window.innerWidth <= 768) {
        const toggleBtn = document.createElement('button');
        toggleBtn.className = 'mobile-menu-toggle';
        toggleBtn.innerHTML = '☰ Menu';
        toggleBtn.style.cssText = `
            position: fixed;
            top: 10px;
            left: 10px;
            z-index: 1001;
            padding: 10px 20px;
            background: #2c3e50;
            color: white;
            border: none;
            border-radius: 5px;
            cursor: pointer;
            font-size: 16px;
        `;
        
        document.body.appendChild(toggleBtn);
        
        toggleBtn.addEventListener('click', function() {
            const sidebar = document.querySelector('.sidebar');
            sidebar.classList.toggle('open');
        });
        
        // Close sidebar when clicking outside
        document.addEventListener('click', function(e) {
            const sidebar = document.querySelector('.sidebar');
            const toggleBtn = document.querySelector('.mobile-menu-toggle');
            
            if (!sidebar.contains(e.target) && !toggleBtn.contains(e.target)) {
                sidebar.classList.remove('open');
            }
        });
    }
}

window.addEventListener('resize', debounce(function() {
    const existingToggle = document.querySelector('.mobile-menu-toggle');
    if (window.innerWidth <= 768 && !existingToggle) {
        createMobileMenuToggle();
    } else if (window.innerWidth > 768 && existingToggle) {
        existingToggle.remove();
    }
}, 250));

// Initialize mobile menu on load if needed
if (window.innerWidth <= 768) {
    createMobileMenuToggle();
}

// ========================================
// Code Copy Functionality
// ========================================

document.querySelectorAll('.code-block').forEach(block => {
    const copyBtn = document.createElement('button');
    copyBtn.className = 'copy-code-btn';
    copyBtn.innerHTML = '📋 Copy';
    copyBtn.style.cssText = `
        position: absolute;
        top: 5px;
        right: 5px;
        padding: 5px 10px;
        background: rgba(255, 255, 255, 0.1);
        color: white;
        border: 1px solid rgba(255, 255, 255, 0.2);
        border-radius: 3px;
        cursor: pointer;
        font-size: 12px;
        transition: all 0.2s ease;
    `;
    
    const header = block.querySelector('.code-header');
    if (header) {
        header.style.position = 'relative';
        header.appendChild(copyBtn);
        
        copyBtn.addEventListener('click', async function() {
            const code = block.querySelector('pre code, pre').textContent;
            
            try {
                await navigator.clipboard.writeText(code);
                copyBtn.innerHTML = '✓ Copied!';
                copyBtn.style.background = 'rgba(40, 167, 69, 0.8)';
                
                setTimeout(() => {
                    copyBtn.innerHTML = '📋 Copy';
                    copyBtn.style.background = 'rgba(255, 255, 255, 0.1)';
                }, 2000);
            } catch (err) {
                console.error('Failed to copy:', err);
                copyBtn.innerHTML = '✗ Error';
                setTimeout(() => {
                    copyBtn.innerHTML = '📋 Copy';
                }, 2000);
            }
        });
        
        copyBtn.addEventListener('mouseenter', function() {
            this.style.background = 'rgba(255, 255, 255, 0.2)';
        });
        
        copyBtn.addEventListener('mouseleave', function() {
            if (this.innerHTML === '📋 Copy') {
                this.style.background = 'rgba(255, 255, 255, 0.1)';
            }
        });
    }
});

// ========================================
// Search Functionality (Basic)
// ========================================

function createSearchBox() {
    const searchContainer = document.createElement('div');
    searchContainer.className = 'search-container';
    searchContainer.style.cssText = `
        padding: 15px 2rem;
        border-bottom: 1px solid rgba(255,255,255,0.1);
        margin-bottom: 1rem;
    `;
    
    const searchInput = document.createElement('input');
    searchInput.type = 'text';
    searchInput.placeholder = 'Rechercher...';
    searchInput.className = 'search-input';
    searchInput.style.cssText = `
        width: 100%;
        padding: 10px;
        background: rgba(255, 255, 255, 0.1);
        border: 1px solid rgba(255, 255, 255, 0.2);
        border-radius: 5px;
        color: white;
        font-size: 14px;
    `;
    
    // Set placeholder based on current language
    searchInput.setAttribute('data-i18n-placeholder', 'search.placeholder');
    updateSearchPlaceholder();
    
    searchContainer.appendChild(searchInput);
    
    const logo = document.querySelector('.logo');
    logo.after(searchContainer);
    
    // Simple search implementation
    searchInput.addEventListener('input', debounce(function(e) {
        const searchTerm = e.target.value.toLowerCase();
        const navLinks = document.querySelectorAll('.nav-menu a');
        
        navLinks.forEach(link => {
            const text = link.textContent.toLowerCase();
            const listItem = link.closest('li');
            
            if (text.includes(searchTerm) || searchTerm === '') {
                listItem.style.display = '';
            } else {
                listItem.style.display = 'none';
            }
        });
    }, 300));
}

// Add search box to sidebar
createSearchBox();

// Update search placeholder when language changes
function updateSearchPlaceholder() {
    const searchInput = document.querySelector('.search-input');
    if (searchInput && translations[currentLanguage]) {
        searchInput.placeholder = translations[currentLanguage].search.placeholder;
    }
}

// Override setLanguage to also update search placeholder
const originalSetLanguage = setLanguage;
setLanguage = function(lang) {
    originalSetLanguage(lang);
    updateSearchPlaceholder();
};

// ========================================
// Back to Top Button
// ========================================

const backToTopBtn = document.createElement('button');
backToTopBtn.innerHTML = '↑';
backToTopBtn.className = 'back-to-top';
backToTopBtn.style.cssText = `
    position: fixed;
    bottom: 30px;
    right: 30px;
    width: 50px;
    height: 50px;
    background: #0066cc;
    color: white;
    border: none;
    border-radius: 50%;
    font-size: 24px;
    cursor: pointer;
    opacity: 0;
    visibility: hidden;
    transition: all 0.3s ease;
    box-shadow: 0 4px 10px rgba(0,0,0,0.3);
    z-index: 999;
`;

document.body.appendChild(backToTopBtn);

window.addEventListener('scroll', function() {
    if (window.pageYOffset > 300) {
        backToTopBtn.style.opacity = '1';
        backToTopBtn.style.visibility = 'visible';
    } else {
        backToTopBtn.style.opacity = '0';
        backToTopBtn.style.visibility = 'hidden';
    }
});

backToTopBtn.addEventListener('click', function() {
    window.scrollTo({
        top: 0,
        behavior: 'smooth'
    });
});

backToTopBtn.addEventListener('mouseenter', function() {
    this.style.transform = 'scale(1.1)';
});

backToTopBtn.addEventListener('mouseleave', function() {
    this.style.transform = 'scale(1)';
});

// ========================================
// Print Current Page
// ========================================

console.log('%c📚 R-Engine ECS Documentation', 'font-size: 20px; font-weight: bold; color: #0066cc;');
console.log('%cVersion: 1.0.0', 'color: #666;');
console.log('%cDocumentation générée pour le système ECS de R-Engine', 'color: #666;');