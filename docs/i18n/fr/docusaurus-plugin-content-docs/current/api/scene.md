---
sidebar_position: 1
---

# API de la Scène

La classe `ecs::Scene` est le conteneur principal de l'ECS, gérant toutes les entités, composants et ressources.

:::caution Modification Directe
Bien que `Scene` fournisse l'API de bas niveau pour manipuler le monde, les systèmes devraient **presque toujours** utiliser `ecs::Commands` pour les modifications. Les appels directs aux méthodes de `Scene` comme `add_component` ou `destroy_entity` depuis un système peuvent entraîner des plantages en invalidant les requêtes actives.
:::

## Méthodes pour les Systèmes

La principale façon dont les systèmes interagissent avec la scène est en demandant des ressources.

### get_resource_ptr()

```cpp
template<typename T>
T* get_resource_ptr() noexcept;
```

Obtient un pointeur brut vers une ressource globale si elle existe.

**Retourne** : Un pointeur vers la ressource, ou `nullptr` si la ressource n'existe pas.

**Exemple** :

```cpp
// Dans un système, vous utiliseriez typiquement Res<T> ou ResMut<T>
// mais ceci montre l'accès sous-jacent.
void some_function(ecs::Scene& scene) {
    if (auto* config = scene.get_resource_ptr<GameConfig>()) {
        // Utiliser la config
    }
}
```

## Méthodes de Bas Niveau (Pour Usage Interne)

Ces méthodes sont utilisées en interne par le moteur et `Commands` pour appliquer les changements. Évitez de les appeler directement depuis les systèmes.

### create_entity() / destroy_entity()

```cpp
Entity create_entity();
void destroy_entity(Entity e) noexcept;
```

Crée ou détruit une entité. **Utilisez `commands.spawn()` ou `commands.despawn()` à la place.**

### add_component() / remove_component()

````cpp
template<typename T> void add_component(Entity e, T comp);
template<typename T> void remove_component(Entity e);```
Ajoute ou supprime un composant d'une entité. **Utilisez `commands.entity(e).insert()` ou `.remove()` à la place.**

### insert_resource() / remove_resource()

```cpp
template<typename T> void insert_resource(T&& r) noexcept;
template<typename T> void remove_resource() noexcept;
````

Ajoute ou supprime une ressource globale. **Utilisez `commands.insert_resource()` ou `.remove_resource()` à la place.**

## Exemple : Utilisation Correcte dans un Système

```cpp
// Manière correcte pour un système d'interagir avec le monde
void my_system(
    ecs::Commands& commands,                 // Pour les modifications
    ecs::Res<GameConfig> config,             // Pour lire les ressources
    ecs::Query<ecs::Mut<Position>> query     // Pour accéder aux composants
) {
    // Créer une nouvelle entité
    commands.spawn(ScoreText{});

    // Modifier les entités en toute sécurité
    for (auto [pos] : query) {
        if (pos.ptr->x > config.ptr->world_width) {
            // C'est sûr car c'est une commande différée
            commands.despawn(it.entity());
        }
    }
}
```

## Voir Aussi

- [Commands](./commands.md) - La manière sûre et principale de modifier le monde.
- [Query](./query.md) - La manière principale de lire les données des composants.
