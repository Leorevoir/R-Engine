---
sidebar_position: 1
slug: /
---

# Introduction à R-Engine ECS

R-Engine utilise une architecture **Entity Component System (ECS)** inspirée de Bevy, conçue pour offrir performance, flexibilité et facilité d'utilisation.

## Qu'est-ce qu'un ECS ?

:::info

Un ECS est un patron de conception architectural qui sépare les données (Components) de la logique (Systems), en les reliant via des entités (Entities). Cette approche favorise la composition plutôt que l'héritage et permet un code plus modulaire et performant.

:::

## Fonctionnalités Clés

- **Stockage basé sur les Archétypes** : Stockage optimisé par archétype pour des performances maximales
- **Sécurité des Types** : Système de types à la compilation pour éviter les erreurs
- **Injection de Dépendances** : Résolution automatique des dépendances des systèmes
- **Command Buffer** : Modifications différées pour éviter l'invalidation des itérateurs
- **Système d'Événements** : Communication asynchrone entre les systèmes
- **Support de Hiérarchie** : Relations parent-enfant natives

## Pourquoi l'ECS ?

Les architectures orientées objet traditionnelles avec des hiérarchies d'héritage profondes peuvent devenir difficiles à maintenir et à étendre. L'ECS offre plusieurs avantages :

### Performance

- **Cache-friendly** : Les composants sont stockés de manière contiguë en mémoire
- **Conception orientée données** : Optimisé pour les architectures CPU modernes
- **Exécution parallèle** : Les systèmes peuvent s'exécuter en parallèle lorsqu'ils n'entrent pas en conflit

### Flexibilité

- **Composition plutôt qu'héritage** : Mélangez et associez les composants facilement
- **Flexibilité à l'exécution** : Ajoutez/supprimez des composants dynamiquement
- **Systèmes modulaires** : Facile d'ajouter, supprimer ou modifier le comportement

### Maintenabilité

- **Séparation claire des préoccupations** : Les données et la logique sont séparées
- **Tests faciles** : Les systèmes peuvent être testés de manière isolée
- **Composants réutilisables** : Partagez des composants entre différents types d'entités

## Démarrage Rapide

Pour commencer à utiliser R-Engine ECS dans votre application :

```cpp
#include "R-Engine/Application.hpp"

using namespace REngine;

int main() {
    Application{}
        .add_systems<startup_system>(Schedule::STARTUP)
        .add_systems<update_system>(Schedule::UPDATE)
        .run();
    
    return 0;
}
```

## Prochaines Étapes

- Découvrez l'[Architecture](./architecture.md) pour comprendre comment fonctionne R-Engine ECS en interne
- Explorezz les [Concepts Fondamentaux](./core-concepts/index.md) pour comprendre les entités, composants et systèmes
- Consultez les [Exemples](./examples/index.md) pour des échantillons de code pratiques
