# R-Engine ECS Documentation# Website



Documentation Docusaurus pour le système ECS (Entity Component System) de R-Engine.This website is built using [Docusaurus](https://docusaurus.io/), a modern static website generator.



## 🚀 Démarrage rapide## Installation



### Prérequis```bash

yarn

- Node.js 18+ ```

- npm ou yarn

## Local Development

### Installation

```bash

```bashyarn start

cd docs-new```

npm install

```This command starts a local development server and opens up a browser window. Most changes are reflected live without having to restart the server.



### Développement## Build



```bash```bash

npm startyarn build

``````



Ouvre http://localhost:3000 avec hot-reload.This command generates static content into the `build` directory and can be served using any static contents hosting service.



### Build de production## Deployment



```bashUsing SSH:

npm run build

``````bash

USE_SSH=true yarn deploy

### Prévisualisation du build```



```bashNot using SSH:

npm run serve

``````bash

GIT_USER=<Your GitHub username> yarn deploy

## 📖 Contenu```



La documentation couvre:If you are using GitHub pages for hosting, this command is a convenient way to build the website and push to the `gh-pages` branch.


- **Introduction**: Vue d'ensemble de l'ECS
- **Architecture**: Diagrammes et flux d'exécution
- **Concepts de base**: Entities, Components, Systems, Resources, Queries, Commands
- **Fonctionnalités avancées**: Events, Hierarchies, Run Conditions, Schedules
- **Système de stockage**: Architecture par archétypes
- **Exemples**: Code fonctionnel et commenté
- **Référence API**: Documentation complète des classes

## 🌐 Langues supportées

- 🇬🇧 English (par défaut)
- 🇫🇷 Français

Pour ajouter des traductions:

```bash
npm run write-translations -- --locale fr
```

## 📁 Structure

```
docs/               # Documentation source (Markdown)
├── intro.md
├── architecture.md
├── core-concepts/
├── advanced/
├── examples/
└── api/

i18n/               # Traductions
└── fr/
    └── docusaurus-plugin-content-docs/
        └── current/

static/             # Assets statiques
└── img/

src/                # Composants React personnalisés
└── css/
    └── custom.css
```

## ⚙️ Configuration

### docusaurus.config.ts

Configuration principale:
- Métadonnées du site
- Thème et plugins
- i18n
- Navbar/Footer

### sidebars.ts

Structure de la navigation latérale.

## 🎨 Personnalisation

Modifier les styles dans `src/css/custom.css`:

```css
:root {
  --ifm-color-primary: #0066cc;
  --ifm-code-font-size: 95%;
}
```

## 📦 Déploiement

### GitHub Pages

```bash
npm run deploy
```

### Autres plateformes

```bash
npm run build
# Servir le dossier build/
```

## 🔧 Scripts disponibles

- `npm start` - Serveur de développement
- `npm run build` - Build de production
- `npm run serve` - Prévisualiser le build
- `npm run clear` - Nettoyer le cache
- `npm run write-translations` - Générer les fichiers de traduction
- `npm run deploy` - Déployer sur GitHub Pages

## 📝 Écrire de la documentation

### Créer un nouveau document

1. Créer `docs/mon-document.md`:

```markdown
---
sidebar_position: 1
---

# Mon Document

Contenu...
```

2. Tester avec `npm start`

### Blocs Markdown spéciaux

```markdown
:::note
Note
:::

:::tip
Conseil
:::

:::warning
Avertissement
:::

:::danger
Danger
:::
```

### Code avec coloration

````markdown
```cpp
void system() {
    // Code C++
}
```
````

## 🌍 i18n (Internationalisation)

### Workflow de traduction

1. **Écrire en anglais** dans `docs/`
2. **Générer les templates**:
   ```bash
   npm run write-translations -- --locale fr
   ```
3. **Traduire** les fichiers dans `i18n/fr/`
4. **Build avec toutes les locales**:
   ```bash
   npm run build
   ```

### Tester une locale

```bash
npm start -- --locale fr
```

## 🤝 Contribution

1. Fork le projet
2. Créer une branche (`git checkout -b doc/feature`)
3. Modifier les fichiers dans `docs/`
4. Tester localement
5. Commit et Push
6. Créer une Pull Request

## 📚 Ressources

- [Docusaurus Documentation](https://docusaurus.io/docs)
- [Markdown Guide](https://docusaurus.io/docs/markdown-features)
- [i18n Tutorial](https://docusaurus.io/docs/i18n/tutorial)

## 🐛 Problèmes connus

### Port déjà utilisé

```bash
npm start -- --port 3001
```

### Erreurs de cache

```bash
npm run clear
npm start
```

## 📄 Licence

Partie du projet R-Engine.

---

**Version**: 1.0.0  
**Construit avec**: [Docusaurus](https://docusaurus.io/)  
**Date**: Octobre 2025
