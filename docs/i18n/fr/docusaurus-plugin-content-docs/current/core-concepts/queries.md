---
sidebar_position: 5
---

# Requêtes

Les requêtes permettent aux systèmes d'itérer sur les entités qui correspondent à des critères spécifiques. Elles fournissent un accès type-safe aux composants.

## Requête Basique

```cpp
void system(Query<Ref<Position>, Ref<Velocity>> query) {
    for (auto [pos, vel] : query) {
        // Traite les entités ayant Position ET Velocity
    }
}
```

## Modes d'Accès

### Ref\<T\> - Lecture Seule

```cpp
Query<Ref<Position>> query
// Accès en lecture seule à Position
```

### Mut\<T\> - Mutable

```cpp
Query<Mut<Position>> query
// Accès mutable à Position
```

## Filtres de Requête

### With\<T\> - Possède le Composant

```cpp
Query<Ref<Position>, With<Player>> query
// Entités qui ont les composants Position ET Player
```

### Without\<T\> - N'a Pas le Composant

```cpp
Query<Ref<Position>, Without<Dead>> query
// Entités avec Position mais SANS le composant Dead
```

### Filtres Combinés

```cpp
Query<Mut<Position>, With<Player>, Without<Frozen>> query
// Entités joueur qui ne sont pas gelées
```

## Obtenir l'ID d'Entité

Vous pouvez obtenir l'ID de l'entité actuelle depuis l'itérateur de la requête.

```cpp
void system(Query<Ref<Position>> query) {
    for (auto it = query.begin(); it != query.end(); ++it) {
        // Obtenir le tuple de composants
        auto [pos] = *it;

        // Obtenir l'ID de l'entité depuis l'itérateur
        Entity entity = it.entity();

        std::cout << "Entité " << entity
                  << " est à (" << pos->x << ", " << pos->y << ")\n";
    }
}
```

## Itération de Requête

### Boucle For Basée sur une Plage

```cpp
for (auto [pos, vel] : query) {
    pos->x += vel->x;
}
```

### Itération Manuelle

```cpp
for (auto it = query.begin(); it != query.end(); ++it) {
    auto [pos, vel] = *it;
    // Traitement...
}
```

## Bonnes Pratiques

- Utilisez `Ref<T>` par défaut, `Mut<T>` seulement quand nécessaire
- Utilisez les filtres pour réduire les itérations
- Accédez à l'ID d'entité quand vous devez référencer des entités

## Prochaines Étapes

- Apprenez les [Commandes](./commands.md)
- Consultez les [Exemples](../examples/index.md)
