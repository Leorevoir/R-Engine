---
sidebar_position: 4
---

# API des Requêtes

`ecs::Query` permet une itération avec typage sûr sur les entités possédant un ensemble spécifique de composants.

## Paramètres de Template

```cpp
ecs::Query<Wrappers...>
```

Les Wrappers définissent les composants à accéder et les filtres à appliquer :

- `Ref<T>` : Accès en lecture seule au composant `T`.
- `Mut<T>` : Accès modifiable (lecture/écriture) au composant `T`.
- `With<T>` : Filtre pour inclure uniquement les entités qui _ont_ le composant `T`.
- `Without<T>` : Filtre pour exclure les entités qui _ont_ le composant `T`.
- `Optional<T>` : Accès optionnel en lecture seule au composant `T`. Le pointeur résultant sera `nullptr` si le composant est manquant.

## Itération

### Boucle For Basée sur une Plage

La manière la plus courante d'utiliser une requête est avec une boucle `for` basée sur une plage.

```cpp
void system(ecs::Query<ecs::Mut<Position>, ecs::Ref<Velocity>> query) {
    for (auto [pos, vel] : query) {
        pos.ptr->x += vel.ptr->x;
    }
}
```

### Accéder à l'ID de l'Entité

Pour obtenir l'ID de l'entité, utilisez un itérateur et sa méthode `.entity()`.

```cpp
void system(ecs::Query<ecs::Ref<Position>> query) {
    for (auto it = query.begin(); it != query.end(); ++it) {
        auto [pos] = *it;
        ecs::Entity entity_id = it.entity();
        std::cout << "Entity " << entity_id << "\n";
    }
}
```

## Filtres

### With\<T\>

Inclure uniquement les entités qui ont le composant `T` :

```cpp
// Cette requête n'itérera que sur les entités qui ont un composant Player.
ecs::Query<ecs::Ref<Position>, ecs::With<Player>> query;
```

### Without\<T\>

Exclure les entités qui ont le composant `T` :

```cpp
// Cette requête n'itérera que sur les entités qui n'ont PAS de composant Dead.
ecs::Query<ecs::Ref<Position>, ecs::Without<Dead>> query;
```

### Filtres Combinés

Vous pouvez combiner plusieurs filtres.

```cpp
// Obtenir un accès modifiable à la Position des entités qui sont des Players,
// ne sont pas Frozen, et ne sont pas Dead.
ecs::Query<
    ecs::Mut<Position>,
    ecs::With<Player>,
    ecs::Without<Frozen>,
    ecs::Without<Dead>
> query;
```

## Méthodes

### size()

```cpp
u64 size() const;
```

Retourne le nombre d'entités qui correspondent actuellement à la requête.

## Accéder aux Pointeurs de Composants

Lorsque vous itérez, vous obtenez des wrappers (`Mut<T>`, `Ref<T>`) qui contiennent un `ptr` vers les données du composant.

### Ref\<T\> - Lecture Seule

```cpp
ecs::Query<ecs::Ref<Position>> query;
for (auto [pos] : query) {
    float x = pos.ptr->x;  // ✓ La lecture est OK
    // pos.ptr->x = 0;     // ✗ Erreur de compilation : ptr est const
}
```

### Mut\<T\> - Modifiable

```cpp
ecs::Query<ecs::Mut<Position>> query;
for (auto [pos] : query) {
    pos.ptr->x = 0;  // ✓ L'écriture est OK
}
```

## Conseils de Performance

- Utilisez `Ref<T>` au lieu de `Mut<T>` lorsque vous n'avez pas besoin de modifier les données. Cela permet au planificateur d'exécuter potentiellement plus de systèmes en parallèle.
- Utilisez les filtres `With<T>` et `Without<T>` pour affiner l'ensemble des entités que votre système doit traiter.
- Évitez de requêter des composants que votre système n'utilise pas réellement.

## Voir Aussi

- [Guide des Requêtes](../core-concepts/queries.md)
- [Composants](../core-concepts/components.md)
