# R-Engine ECS Documentation

Complete web documentation for the R-Engine Entity Component System (ECS).

## 📖 Content

This documentation covers all aspects of the R-Engine ECS:

- **Introduction**: General overview of the ECS architecture
- **Architecture**: Diagrams and execution flow
- **Core Concepts**: 
  - Entities
  - Components
  - Systems
  - Resources
  - Queries
  - Commands
- **Advanced Features**:
  - Events
  - Hierarchies (Parent-Child relationships)
  - Run Conditions
  - Schedules (System scheduling)
- **Storage System**: Archetype-based architecture
- **Complete Examples**: Functional and commented code
- **API Reference**: Complete documentation of classes and methods

## 🚀 Usage

### Option 1: Open HTML File Directly

```bash
# Linux/MacOS
xdg-open docs/index.html

# Windows
start docs/index.html
```

### Option 2: Local HTTP Server

For the best experience (especially to test all JavaScript features):

```bash
# With Python 3
cd docs
python3 -m http.server 8000

# With Node.js (npx)
cd docs
npx http-server -p 8000

# With PHP
cd docs
php -S localhost:8000
```

Then open your browser at: `http://localhost:8000`

### Option 3: VS Code Live Server

If you're using VS Code with the "Live Server" extension:

1. Right-click on `docs/index.html`
2. Select "Open with Live Server"

## ✨ Features

### Navigation

- **Sidebar**: Quick navigation between sections
- **Smooth Scrolling**: Smooth scrolling to sections
- **Active Link Highlighting**: Automatic highlighting of active links
- **Mobile Responsive**: Hamburger menu on mobile

### Interactivity

- **Copy Code**: Button to copy code examples
- **Search**: Search within navigation (sidebar)
- **Back to Top**: Button to scroll back to top

### Content

- **Code Examples**: Over 20 commented code examples
- **API Tables**: Reference tables for all APIs
- **Diagrams**: ASCII diagrams to visualize architecture
- **Info Boxes**: Information boxes, warnings, and best practices

## 📁 File Structure

```
docs/
├── index.html     # Main documentation page
├── style.css      # CSS styles (modern and responsive design)
├── script.js      # JavaScript scripts (interactivity)
└── README.md      # This file
```

## 🎨 Customization

### Changing Colors

Edit the CSS variables in `style.css`:

```css
:root {
    --primary-color: #0066cc;      /* Primary color */
    --primary-dark: #004999;       /* Dark primary color */
    --bg-sidebar: #2c3e50;         /* Sidebar background */
    /* ... other variables ... */
}
```

### Adding Content

1. Add a new section in `index.html`
2. Create a link in the sidebar
3. Scripts will automatically handle smooth scrolling and navigation

## 🌐 Browser Compatibility

- ✅ Chrome/Chromium (90+)
- ✅ Firefox (88+)
- ✅ Safari (14+)
- ✅ Edge (90+)

## 📱 Mobile Support

The documentation is fully responsive and optimized for:
- Smartphones (320px+)
- Tablets (768px+)
- Desktop (1024px+)

## 🖨️ Printing

The documentation includes optimized print styles. Use `Ctrl+P` or `Cmd+P` to print.

## 🔍 SEO and Accessibility

- Semantic HTML hierarchy
- Keyboard navigation
- WCAG AA compliant color contrast
- Meta tags for social sharing (to be added if needed)

## 📝 Technical Notes

### Technologies Used

- **HTML5**: Semantic structure
- **CSS3**: CSS Variables, Flexbox, Grid, Animations
- **Vanilla JavaScript**: No external dependencies
- **Responsive Design**: Mobile-first approach

### Performance

- No external libraries (fast loading)
- Optimized CSS and JS
- Images replaced with ASCII diagrams (lightweight)

## 🤝 Contributing

To improve this documentation:

1. Edit `index.html` for content
2. Edit `style.css` for design
3. Edit `script.js` for interactive features

## 📚 Additional Resources

- [R-Engine source code](../)
- [Practical examples](../examples/)
- [Unit tests](../tests/)

## 📄 License

This documentation is part of the R-Engine project.

---

**Generated on**: 2025-10-20  
**Version**: 1.0.0  
**Author**: R-Engine Team