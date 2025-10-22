---
sidebar_position: 7
---

# Système de Stockage

R-Engine utilise un **système de stockage basé sur les archétypes** pour des performances optimales et une efficacité mémoire.

## Vue d'Ensemble des Archétypes

Un archétype est une combinaison unique de types de composants. Les entités avec le même ensemble de composants partagent le même archétype.

```
Archétype A : [Position, Velocity]
  Entité 1 : Position{0,0}, Velocity{1,0}
  Entité 2 : Position{5,3}, Velocity{-1,2}

Archétype B : [Position, Velocity, Health]
  Entité 10 : Position{2,1}, Velocity{0,1}, Health{100}
```

## Organisation Mémoire

Les composants sont stockés dans des tableaux contigus par archétype :

```
Archétype [Position, Velocity]
┌──────────────────────────────┐
│ Positions: [P1][P2][P3][P4]  │
│ Velocities: [V1][V2][V3][V4] │
│ Entity IDs: [E1][E2][E3][E4] │
└──────────────────────────────┘
```

## Avantages

### Optimisé Cache

Les composants du même type sont adjacents en mémoire, améliorant l'utilisation du cache CPU.

### Itération Rapide

Les requêtes itèrent sur des archétypes entiers d'un coup, traitant plusieurs entités efficacement.

### Efficacité Mémoire

Pas d'indirection par pointeur - accès direct aux données des composants.

## Changements d'Archétype

Quand des composants sont ajoutés ou supprimés, les entités se déplacent entre archétypes :

```cpp
// Entité dans l'archétype [Position, Velocity]
Entity e = commands.spawn(Position{}, Velocity{}).id();

// Ajouter Health → déplace vers l'archétype [Position, Velocity, Health]
commands.entity(e).insert(Health{100});

// Supprimer Velocity → déplace vers l'archétype [Position, Health]
commands.entity(e).remove<Velocity>();
```

:::tip Performance
Minimisez les changements d'archétype pendant le code critique en performance. Chaque changement nécessite de déplacer les données des composants.
:::

## Correspondance de Requêtes

Les requêtes correspondent automatiquement à tous les archétypes compatibles :

```cpp
// Requête : Position + Velocity
Query<Mut<Position>, Ref<Velocity>> query;

// Correspond :
// ✓ [Position, Velocity]
// ✓ [Position, Velocity, Health]
// ✓ [Position, Velocity, Sprite, AI]
//
// Ne correspond PAS :
// ✗ [Position]
// ✗ [Velocity]
// ✗ [Position, Health]
```

## Caractéristiques de Performance

| Opération | Complexité | Notes |
|-----------|-----------|-------|
| Accès Composant | O(1) | Accès direct au tableau |
| Itération Requête | O(n) | Linéaire au nombre d'entités |
| Ajouter Composant | O(n_components) | Copier composants vers nouvel archétype |
| Supprimer Composant | O(n_components) | Copier vers nouvel archétype |
| Créer Entité | O(1) amorti | Ajouter à l'archétype |
| Détruire Entité | O(1) amorti | Swap-remove de l'archétype |

## Bonnes Pratiques

### ✅ À Faire

- Concevez les composants pour minimiser les changements d'archétype
- Utilisez des composants marqueurs pour la catégorisation
- Groupez les opérations de création

### ❌ À Éviter

- N'ajoutez/supprimez pas de composants dans des boucles serrées
- Ne créez pas trop de combinaisons uniques de composants
- Ne changez pas fréquemment la composition des entités

## Exemple : Conception Efficace

```cpp
// Bon : Composition de composants stable
struct Unit {
    int health;
    bool is_dead;  // ✓ Drapeau au lieu d'ajouter/supprimer le composant Dead
};

// Moins efficace : Changements d'archétype fréquents
void bad_pattern(Entity e, Commands& commands) {
    if (health <= 0) {
        commands.entity(e).insert(Dead{});  // ✗ Cause un changement d'archétype
    } else {
        commands.entity(e).remove<Dead>();  // ✗ Un autre changement
    }
}

// Meilleur : Utiliser un drapeau
void good_pattern(Query<Mut<Unit>> query) {
    for (auto [unit] : query) {
        unit->is_dead = (unit->health <= 0);  // ✓ Pas de changement d'archétype
    }
}
```

## Prochaines Étapes

- Explorez les [Exemples](./examples/index.md) pour voir l'utilisation des archétypes
- Consultez la [Référence API](./api/index.md) pour les détails techniques
