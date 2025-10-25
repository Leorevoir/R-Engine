---
sidebar_position: 4
---

# API des requêtes

Itération d'entités type-safe avec accès aux composants.

## Paramètres Template

```cpp
Query<Wrappers...>
```

Les wrappers peuvent être :
- `Ref<T>` - Accès composant lecture seule
- `Mut<T>` - Accès composant mutable
- `With<T>` - Filtre : l'entité doit avoir T
- `Without<T>` - Filtre : l'entité ne doit pas avoir T
- `Entity` - Obtenir l'ID d'entité

## Itération

### Boucle For Basée sur une Plage

```cpp
Query<Mut<Position>, Ref<Velocity>> query;

for (auto [pos, vel] : query) {
    pos.ptr->x += vel.ptr->x;
}
```

### Avec ID d'Entité

```cpp
Query<Entity, Ref<Position>> query;

for (auto [entity, pos] : query) {
    std::cout << "Entity " << entity.id() << "\n";
}
```

## Filtres

### With\<T\>

Seulement les entités qui ont le composant T :

```cpp
Query<Ref<Position>, With<Player>> query;
// Traite seulement les entités joueur
```

### Without\<T\>

Seulement les entités qui n'ont pas le composant T :

```cpp
Query<Ref<Position>, Without<Dead>> query;
// Traite seulement les entités vivantes
```

### Combinés

```cpp
Query<
    Mut<Position>,
    With<Player>,
    Without<Frozen>,
    Without<Dead>
> query;
// Joueurs actifs uniquement
```

## Méthodes

### get()

```cpp
template<typename Wrapper>
std::optional<Wrapper> get(Entity e)
```

Obtenir le composant pour une entité spécifique.

**Exemple** :
```cpp
if (auto pos = query.get<Ref<Position>>(entity)) {
    // Utiliser pos
}
```

### is_empty()

```cpp
bool is_empty() const
```

Vérifier si la requête correspond à des entités.

## Modes d'Accès

### Ref\<T\> - Lecture Seule

```cpp
Query<Ref<Position>> query
for (auto [pos] : query) {
    float x = pos.ptr->x;  // ✓ Lecture
    // pos.ptr->x = 0;     // ✗ Erreur : const
}
```

### Mut\<T\> - Mutable

```cpp
Query<Mut<Position>> query
for (auto [pos] : query) {
    pos.ptr->x = 0;  // ✓ Écriture autorisée
}
```

## Conseils de Performance

- Utilisez `Ref<T>` au lieu de `Mut<T>` quand vous n'avez pas besoin de modifier
- Utilisez les filtres pour réduire les itérations
- Évitez de requêter des composants inutiles

## Voir Aussi

- [Guide Queries](../core-concepts/queries.md)
- [Components](../core-concepts/components.md)
