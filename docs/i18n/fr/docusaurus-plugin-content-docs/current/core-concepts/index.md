---
sidebar_position: 3
---

# Concepts Fondamentaux

Cette section couvre les éléments fondamentaux de R-Engine ECS.

## Vue d'Ensemble

R-Engine ECS est construit sur trois concepts principaux :

### Entités

Les entités sont des identifiants uniques qui représentent des "choses" dans votre jeu ou application. Ce sont des IDs légers qui relient les composants entre eux.

[En savoir plus sur les Entités →](./entities.md)

### Composants

Les composants sont des structures de données pures attachées aux entités. Ils définissent ce qu'une entité **est** ou **possède**.

[En savoir plus sur les Composants →](./components.md)

### Systèmes

Les systèmes sont des fonctions qui contiennent de la logique. Ils définissent ce que votre application **fait** en opérant sur les entités avec des composants spécifiques.

[En savoir plus sur les Systèmes →](./systems.md)

## Concepts Additionnels

### Ressources

Les ressources sont des données globales et uniques accessibles par tous les systèmes. Parfaites pour la configuration, les services ou l'état partagé.

[En savoir plus sur les Ressources →](./resources.md)

### Requêtes

Les requêtes permettent aux systèmes d'itérer sur les entités qui correspondent à des critères spécifiques. Elles fournissent un accès type-safe aux composants.

[En savoir plus sur les Requêtes →](./queries.md)

### Commandes

Les commandes fournissent un accès différé pour modifier le monde ECS. Elles préviennent l'invalidation des itérateurs et permettent des changements structurels sécurisés.

[En savoir plus sur les Commandes →](./commands.md)

## Le Pattern ECS

```cpp
// Composants : Données pures
struct Position { float x, y; };
struct Velocity { float x, y; };

// Systèmes : Logique pure
void movement_system(
    Query<Mut<Position>, Ref<Velocity>> query,
    Res<DeltaTime> time
) {
    for (auto [pos, vel] : query) {
        pos->x += vel->x * time->dt;
        pos->y += vel->y * time->dt;
    }
}

// Application : Assembler le tout
Application{}
    .add_systems<movement_system>(Schedule::UPDATE)
    .run();
```

Cette séparation des données et de la logique fournit :
- **Flexibilité** : Facile d'ajouter, supprimer ou modifier le comportement
- **Performance** : Conception orientée données optimisée pour les CPU modernes
- **Maintenabilité** : Séparation claire des préoccupations
- **Testabilité** : Les systèmes peuvent être testés de manière isolée

## Prochaines Étapes

Commencez par [Entités](./entities.md) pour comprendre les fondations du pattern ECS.
