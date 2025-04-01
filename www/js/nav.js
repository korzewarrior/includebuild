// Load the shared navigation
document.addEventListener('DOMContentLoaded', function() {
    // Get the navigation placeholder
    const navContainer = document.querySelector('nav');
    
    // Determine path prefix based on current location
    const pathPrefix = getPathPrefix();
    
    // Use fetch to load the navigation HTML
    fetch(pathPrefix + 'includes/nav.html')
        .then(response => {
            if (!response.ok) {
                throw new Error('Failed to load navigation: ' + response.status);
            }
            return response.text();
        })
        .then(html => {
            // Fix navigation links for subdirectories
            if (isInSubdirectory()) {
                html = adjustLinksForSubdirectory(html);
            }
            
            // Insert the navigation HTML
            navContainer.innerHTML = html;
            
            // Set the active navigation link based on current page
            setActiveNavLink();
            
            // Setup mobile navigation toggle
            setupMobileNav();
        })
        .catch(error => {
            console.error('Error loading navigation:', error);
            // Fallback navigation in case of error
            navContainer.innerHTML = createFallbackNavigation();
        });
});

// Helper function to determine if we're in a subdirectory
function isInSubdirectory() {
    return window.location.pathname.includes('/docs/');
}

// Helper function to get the path prefix
function getPathPrefix() {
    return isInSubdirectory() ? '../' : '';
}

// Adjust links for subdirectory
function adjustLinksForSubdirectory(html) {
    return html.replace(/href="(?!https?:\/\/)(?!#)([^"]+)"/g, function(match, p1) {
        // Don't modify links that already have the correct prefix
        if (p1.startsWith('../') || p1.startsWith('http')) {
            return match;
        }
        return 'href="../' + p1 + '"';
    });
}

// Create fallback navigation in case of error
function createFallbackNavigation() {
    const pathPrefix = getPathPrefix();
    return `
    <div class="container">
        <div class="logo">
            <a href="${pathPrefix}index.html">
                <span class="include">#include</span><span class="build">"build.h"</span>
            </a>
        </div>
        <div class="nav-links">
            <a href="${pathPrefix}index.html">Home</a>
            <a href="${pathPrefix}docs/guide.html">Guide</a>
            <a href="${pathPrefix}examples.html">Examples</a>
            <a href="${pathPrefix}faq.html">FAQ</a>
            <a href="${pathPrefix}docs/api.html">API</a>
            <a href="https://github.com/includebuild/includebuild" class="github-link">GitHub</a>
        </div>
    </div>`;
}

// Set the active navigation link based on current URL
function setActiveNavLink() {
    const currentPath = window.location.pathname;
    const pageName = currentPath.split('/').pop();
    
    // Remove any existing active classes
    const navLinks = document.querySelectorAll('.nav-links a');
    navLinks.forEach(link => link.classList.remove('active'));
    
    // Set active class based on current page
    if (pageName.includes('examples.html')) {
        const link = document.querySelector('[data-page="examples"]');
        if (link) link.classList.add('active');
    } else if (pageName.includes('guide.html')) {
        const link = document.querySelector('[data-page="guide"]');
        if (link) link.classList.add('active');
    } else if (pageName.includes('api.html')) {
        const link = document.querySelector('[data-page="api"]');
        if (link) link.classList.add('active');
    } else if (pageName.includes('faq.html')) {
        const link = document.querySelector('[data-page="faq"]');
        if (link) link.classList.add('active');
    } else if (pageName.includes('index.html') || pageName === '' || currentPath.endsWith('/')) {
        // For home page
        const link = document.querySelector('[data-page="home"]');
        if (link) link.classList.add('active');
    }
}

// Setup mobile navigation toggle
function setupMobileNav() {
    const mobileToggle = document.querySelector('.mobile-nav-toggle');
    const navLinks = document.querySelector('.nav-links');
    
    if (mobileToggle) {
        mobileToggle.addEventListener('click', function() {
            navLinks.classList.toggle('active');
            mobileToggle.classList.toggle('open');
            document.querySelector('nav').classList.toggle('mobile-active');
        });
    }
} 