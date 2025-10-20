# Documentation R-Engine ECS

Documentation web complète du système Entity Component System (ECS) de R-Engine.

## 📖 Contenu

Cette documentation couvre tous les aspects de l'ECS R-Engine :

- **Introduction** : Présentation générale de l'architecture ECS
- **Architecture** : Diagrammes et flux d'exécution
- **Concepts Clés** : 
  - Entities (Entités)
  - Components (Composants)
  - Systems (Systèmes)
  - Resources (Ressources)
  - Queries (Requêtes)
  - Commands (Commandes)
- **Fonctionnalités Avancées** :
  - Events (Événements)
  - Hierarchies (Relations Parent-Enfant)
  - Run Conditions (Conditions d'exécution)
  - Schedules (Planification des systèmes)
- **Système de Stockage** : Architecture basée sur les archétypes
- **Exemples Complets** : Code fonctionnel et commenté
- **Référence API** : Documentation complète des classes et méthodes

## 🚀 Utilisation

### Option 1 : Ouvrir Directement le Fichier HTML

```bash
# Linux/MacOS
xdg-open docs/index.html

# Windows
start docs/index.html
```

### Option 2 : Serveur HTTP Local

Pour une meilleure expérience (notamment pour tester toutes les fonctionnalités JavaScript) :

```bash
# Avec Python 3
cd docs
python3 -m http.server 8000

# Avec Node.js (npx)
cd docs
npx http-server -p 8000

# Avec PHP
cd docs
php -S localhost:8000
```

Puis ouvrez votre navigateur à l'adresse : `http://localhost:8000`

### Option 3 : VS Code Live Server

Si vous utilisez VS Code avec l'extension "Live Server" :

1. Cliquez droit sur `docs/index.html`
2. Sélectionnez "Open with Live Server"

## ✨ Fonctionnalités

### Navigation

- **Sidebar** : Navigation rapide entre les sections
- **Smooth Scrolling** : Défilement fluide vers les sections
- **Active Link Highlighting** : Mise en évidence automatique du lien actif
- **Mobile Responsive** : Menu hamburger sur mobile

### Interactivité

- **Copy Code** : Bouton pour copier les exemples de code
- **Search** : Recherche dans la navigation (sidebar)
- **Back to Top** : Bouton pour remonter en haut de page

### Contenu

- **Code Examples** : Plus de 20 exemples de code commentés
- **API Tables** : Tableaux de référence pour toutes les APIs
- **Diagrams** : Diagrammes ASCII pour visualiser l'architecture
- **Info Boxes** : Encarts d'information, avertissements, et bonnes pratiques

## 📁 Structure des Fichiers

```
docs/
├── index.html     # Page principale de la documentation
├── style.css      # Styles CSS (design moderne et responsive)
├── script.js      # Scripts JavaScript (interactivité)
└── README.md      # Ce fichier
```

## 🎨 Personnalisation

### Modifier les Couleurs

Éditez les variables CSS dans `style.css` :

```css
:root {
    --primary-color: #0066cc;      /* Couleur principale */
    --primary-dark: #004999;       /* Couleur principale foncée */
    --bg-sidebar: #2c3e50;         /* Fond de la sidebar */
    /* ... autres variables ... */
}
```

### Ajouter du Contenu

1. Ajoutez une nouvelle section dans `index.html`
2. Créez un lien dans la sidebar
3. Les scripts géreront automatiquement le smooth scrolling et la navigation

## 🌐 Compatibilité Navigateurs

- ✅ Chrome/Chromium (90+)
- ✅ Firefox (88+)
- ✅ Safari (14+)
- ✅ Edge (90+)

## 📱 Support Mobile

La documentation est entièrement responsive et optimisée pour :
- Smartphones (320px+)
- Tablettes (768px+)
- Desktop (1024px+)

## 🖨️ Impression

La documentation inclut des styles d'impression optimisés. Utilisez `Ctrl+P` ou `Cmd+P` pour imprimer.

## 🔍 SEO et Accessibilité

- Hiérarchie HTML sémantique
- Navigation au clavier
- Contraste de couleurs WCAG AA compliant
- Meta tags pour le partage social (à ajouter si nécessaire)

## 📝 Notes Techniques

### Technologies Utilisées

- **HTML5** : Structure sémantique
- **CSS3** : Variables CSS, Flexbox, Grid, Animations
- **Vanilla JavaScript** : Pas de dépendances externes
- **Responsive Design** : Mobile-first approach

### Performance

- Pas de bibliothèques externes (charge rapide)
- CSS et JS optimisés
- Images remplacées par des diagrammes ASCII (léger)

## 🤝 Contribution

Pour améliorer cette documentation :

1. Modifiez `index.html` pour le contenu
2. Modifiez `style.css` pour le design
3. Modifiez `script.js` pour les fonctionnalités interactives

## 📚 Ressources Additionnelles

- [Code source R-Engine](../)
- [Exemples pratiques](../examples/)
- [Tests unitaires](../tests/)

## 📄 Licence

Cette documentation fait partie du projet R-Engine.

---

**Généré le** : 2025-10-20  
**Version** : 1.0.0  
**Auteur** : R-Engine Team