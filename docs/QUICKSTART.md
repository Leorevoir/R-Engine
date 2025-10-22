# Guide de Démarrage Rapide - Documentation Docusaurus

## 🚀 Démarrage en 3 étapes

### 1. Installation

```bash
cd docs-new
npm install
```

### 2. Lancement

```bash
npm start
```

→ Ouvre automatiquement http://localhost:3000

### 3. Modification

Éditez les fichiers dans `docs/` et voyez les changements en temps réel !

## 📝 Éditer la documentation

### Structure des fichiers

```
docs/
├── intro.md               # Page d'accueil
├── architecture.md        # Architecture
├── core-concepts/         # Concepts de base
│   ├── index.md
│   ├── entities.md
│   ├── components.md
│   ├── systems.md
│   ├── resources.md
│   ├── queries.md
│   └── commands.md
├── advanced/              # Fonctionnalités avancées
│   ├── index.md
│   ├── events.md
│   ├── hierarchies.md
│   ├── run-conditions.md
│   └── schedules.md
├── storage.md            # Stockage
├── examples/             # Exemples
│   ├── index.md
│   ├── bouncing-balls.md
│   ├── system-hierarchy.md
│   └── event-communication.md
└── api/                  # API Reference
    ├── index.md
    ├── scene.md
    ├── commands.md
    ├── entity-commands.md
    ├── query.md
    └── events.md
```

### Format des fichiers

Chaque fichier `.md` commence par un frontmatter:

```markdown
---
sidebar_position: 1
---

# Titre de la page

Contenu...
```

## 🎨 Blocs spéciaux

### Notes et conseils

```markdown
:::note
Ceci est une note
:::

:::tip
Ceci est un conseil
:::

:::warning
Ceci est un avertissement
:::

:::danger
Attention danger !
:::

:::info
Information utile
:::
```

### Code avec coloration

````markdown
```cpp
void system(Query<Mut<Position>> query) {
    for (auto [pos] : query) {
        pos->x += 1.0f;
    }
}
```
````

Langages supportés: `cpp`, `cmake`, `bash`, `json`, `typescript`, etc.

## 🔗 Liens internes

```markdown
[Lien vers une autre page](./architecture.md)
[Lien relatif](../advanced/events.md)
[Lien absolu](/core-concepts/entities)
```

## 🌐 Traduction (i18n)

### 1. Générer les fichiers de traduction

```bash
npm run write-translations -- --locale fr
```

### 2. Traduire

Fichiers générés dans:
```
i18n/fr/docusaurus-plugin-content-docs/current/
```

Copier et traduire chaque fichier `.md` depuis `docs/`.

### 3. Tester

```bash
npm start -- --locale fr
```

### 4. Build avec toutes les langues

```bash
npm run build
```

## 🎯 Commandes essentielles

```bash
# Développement
npm start                  # Serveur de dev avec hot-reload

# Build
npm run build             # Build de production

# Test du build
npm run serve             # Servir le build localement

# Maintenance
npm run clear             # Nettoyer le cache
npm run write-translations -- --locale fr  # Générer traductions

# Déploiement
npm run deploy            # Déployer sur GitHub Pages
```

## 📦 Build de production

```bash
npm run build
```

Génère les fichiers statiques dans `build/`:

```
build/
├── index.html
├── assets/
├── docs/
└── ...
```

Ces fichiers peuvent être servis par n'importe quel serveur web statique (nginx, Apache, GitHub Pages, Netlify, Vercel, etc.)

## 🔧 Personnalisation

### Couleurs et styles

Éditer `src/css/custom.css`:

```css
:root {
  --ifm-color-primary: #0066cc;
  --ifm-code-font-size: 95%;
  /* ... */
}
```

### Configuration

Éditer `docusaurus.config.ts`:

```typescript
const config: Config = {
  title: 'Mon Titre',
  tagline: 'Mon Tagline',
  // ...
};
```

### Navigation (Sidebar)

Éditer `sidebars.ts`:

```typescript
const sidebars: SidebarsConfig = {
  tutorialSidebar: [
    'intro',
    {
      type: 'category',
      label: 'Ma Catégorie',
      items: ['page1', 'page2'],
    },
  ],
};
```

## 🐛 Résolution de problèmes

### Le serveur ne démarre pas

```bash
rm -rf node_modules package-lock.json
npm install
npm start
```

### Erreurs de build

```bash
npm run clear
npm run build
```

### Port déjà utilisé

```bash
npm start -- --port 3001
```

## 📚 Ressources

- [Documentation Docusaurus](https://docusaurus.io/docs)
- [Markdown Features](https://docusaurus.io/docs/markdown-features)
- [i18n Tutorial](https://docusaurus.io/docs/i18n/tutorial)
- [Deployment](https://docusaurus.io/docs/deployment)

## 💡 Conseils

1. **Hot reload**: Les changements dans `docs/` sont instantanés
2. **Configuration**: Les changements dans `docusaurus.config.ts` nécessitent un redémarrage
3. **Sidebar**: Les changements dans `sidebars.ts` nécessitent un redémarrage
4. **Build avant deploy**: Toujours tester le build localement avant de déployer

## ✅ Checklist avant déploiement

- [ ] Tester le build: `npm run build`
- [ ] Vérifier le résultat: `npm run serve`
- [ ] Vérifier tous les liens internes
- [ ] Vérifier la navigation
- [ ] Tester le responsive (mobile)
- [ ] Vérifier les traductions si activées
- [ ] Nettoyer le cache si nécessaire

---

**Prêt à commencer !** 🎉

Lancez simplement `npm start` et commencez à éditer les fichiers dans `docs/`.
