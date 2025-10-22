---
sidebar_position: 1
---

# API de Scene

La classe Scene gère le monde ECS - entités, composants, ressources et systèmes.

## Méthodes

### spawn()

```cpp
Entity spawn()
Entity spawn(Components... components)
```

Crée une nouvelle entité.

**Retourne** : ID de l'entité

**Exemple** :
```cpp
Entity e = scene.spawn();
Entity player = scene.spawn(Position{}, Health{100});
```

### despawn()

```cpp
void despawn(Entity entity)
```

Supprime une entité et tous ses composants.

### insert_resource()

```cpp
void insert_resource(T resource)
```

Ajoute ou remplace une ressource globale.

### remove_resource()

```cpp
void remove_resource<T>()
```

Supprime une ressource globale.

### get_resource()

```cpp
T* get_resource<T>()
const T* get_resource<T>() const
```

Obtient une ressource si elle existe.

**Retourne** : Pointeur vers la ressource, ou nullptr

## Exemple d'Utilisation

```cpp
Scene scene;

// Créer des entités
Entity player = scene.spawn(Player{}, Position{0, 0});

// Ajouter des ressources
scene.insert_resource(GameConfig{});

// Accéder aux ressources
if (auto* config = scene.get_resource<GameConfig>()) {
    // Utiliser config
}

// Détruire
scene.despawn(player);
```

## Voir Aussi

- [Commands](./commands.md) - Modifications différées
- [Query](./query.md) - Itération d'entités
