# IncludeBuild Website

This directory contains the official website for IncludeBuild, a header-only C/C++ build system.

## Structure

The website follows a simple structure:

- `index.html` - Main landing page
- `css/` - Contains CSS stylesheets
  - `styles.css` - Main stylesheet with all styling
- `js/` - JavaScript files
  - `main.js` - Handles interactivity and animations
- `images/` - Image assets
  - `favicon.svg` - Website favicon
  - `grid-pattern.svg` - Background pattern used in sections

## Design Principles

The website follows these design principles:

1. **Minimalist & Code-focused** - Clean design that emphasizes code examples
2. **Brand Colors** - Uses the official color scheme:
   - Blue (#0076ff) for the `#include` part
   - Green (#5cb85c) for the `"build.h"` part
3. **Typography** - Uses:
   - JetBrains Mono for code elements
   - Inter for general text
4. **Responsive** - Works well on all device sizes

## Local Development

To work on the website locally:

1. Clone the repository
2. Navigate to the `www` directory
3. You can use any local server to view the site, for example:
   - Python: `python -m http.server`
   - Node.js: Install `http-server` with npm and run it
   - VS Code: Use the Live Server extension

## Deployment

This website is designed to be deployed to any static hosting service:

1. **GitHub Pages**:
   - Push the contents of this directory to the `gh-pages` branch
   - Configure GitHub Pages to use this branch

2. **Netlify/Vercel**:
   - Connect your repository and set the publish directory to `/www`

3. **Traditional Hosting**:
   - Upload the contents of this directory to your web server

## Contributing

When contributing to the website:

1. Keep the design consistent with IncludeBuild's overall branding
2. Ensure all code examples are accurate and up-to-date
3. Maintain responsive design for all devices
4. Optimize images and assets for web

## Future Enhancements

Planned enhancements for the website:

- Add a blog section for updates and tutorials
- Create interactive code playground for trying IncludeBuild
- Add language/translation support
- Improve documentation with search functionality 