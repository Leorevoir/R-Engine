---
sidebar_position: 5
---

# Requêtes

Les requêtes sont le principal moyen pour les systèmes d'accéder et d'itérer sur les entités qui possèdent un ensemble spécifique de composants. Elles fournissent un accès sûr, efficace et avec typage sûr aux données des composants.

## Requête de Base

Une requête est définie par les wrappers de composants passés comme arguments de son template. Cet exemple interroge toutes les entités qui ont à la fois un composant `Position` et `Velocity`.

```cpp
void movement_system(ecs::Query<ecs::Mut<Position>, ecs::Ref<Velocity>> query) {
    for (auto [pos, vel] : query) {
        // Traiter les entités avec à la fois Position ET Velocity
        pos.ptr->x += vel.ptr->x;
    }
}
```

## Modes d'Accès (Wrappers)

### `Ref<T>` - Accès en Lecture Seule

Utilisez `Ref<T>` lorsque vous avez seulement besoin de lire les données d'un composant. C'est le choix par défaut préféré car il permet au planificateur plus de possibilités de parallélisme.

```cpp
ecs::Query<ecs::Ref<Position>> query;
// Fournit un accès en lecture seule à Position.
```

### `Mut<T>` - Accès Modifiable

Utilisez `Mut<T>` lorsque vous devez modifier les données d'un composant.

```cpp
ecs::Query<ecs::Mut<Position>> query;
// Fournit un accès en lecture/écriture à Position.
```

## Filtres de Requête

Les filtres vous permettent de restreindre les entités qu'une requête va correspondre sans accéder aux données du composant.

### `With<T>` - Doit Avoir le Composant

Sélectionne les entités qui ont le composant `T`.

```cpp
// Obtient la Position des entités qui ont aussi un composant Player.
ecs::Query<ecs::Ref<Position>, ecs::With<Player>> query;
```

### `Without<T>` - Ne Doit Pas Avoir le Composant

Sélectionne les entités qui n'ont **pas** le composant `T`.

```cpp
// Obtient la Position des entités qui n'ont PAS de composant Dead.
ecs::Query<ecs::Ref<Position>, ecs::Without<Dead>> query;
```

### `Optional<T>` - Peut Avoir le Composant

Fournit un accès optionnel en lecture seule à un composant. Le pointeur du wrapper sera `nullptr` si l'entité n'a pas le composant.

```cpp
void system(ecs::Query<ecs::Ref<Position>, ecs::Optional<Health>> query) {
    for (auto [pos, health_opt] : query) {
        if (health_opt.ptr) {
            // Cette entité a un composant Health.
        } else {
            // Cette entité n'en a pas.
        }
    }
}
```

## Obtenir l'ID de l'Entité

Pour obtenir l'ID de l'entité sur laquelle vous itérez actuellement, utilisez la méthode `.entity()` de l'itérateur.

```cpp
void system(ecs::Query<ecs::Ref<Position>> query) {
    for (auto it = query.begin(); it != query.end(); ++it) {
        // Obtenir le tuple de composants de l'itérateur
        auto [pos] = *it;

        // Obtenir l'ID de l'entité de l'itérateur
        ecs::Entity entity_id = it.entity();

        std::cout << "Entité " << entity_id
                  << " est à (" << pos.ptr->x << ", " << pos.ptr->y << ")\n";
    }
}
```

## Bonnes Pratiques

- Utilisez toujours `Ref<T>` par défaut et passez à `Mut<T>` uniquement lorsque la modification est nécessaire.
- Utilisez des filtres (`With<T>`, `Without<T>`) pour être aussi spécifique que possible, réduisant le nombre d'entités que votre système doit parcourir.
- N'incluez dans la requête que les composants auxquels votre système a réellement besoin d'accéder.

## Prochaines Étapes

- Apprenez-en plus sur les [Commandes](./commands.md) pour modifier les entités.
- Consultez les [Exemples](../examples/index.md) pour une utilisation pratique des requêtes.
