---
sidebar_position: 2
---

# Architecture

Comprendre l'architecture de R-Engine ECS vous aidera à écrire du code plus efficace et maintenable.

## Vue d'Ensemble du Système

```
┌─────────────────────────────────────────────────────────┐
│                      Application                         │
│  ┌────────────────────────────────────────────────────┐ │
│  │                   Ordonnanceur (Scheduler)          │ │
│  │  ┌──────────┐  ┌──────────┐  ┌──────────┐         │ │
│  │  │ STARTUP  │  │  UPDATE  │  │  RENDER  │  ...    │ │
│  │  └──────────┘  └──────────┘  └──────────┘         │ │
│  └────────────────────────────────────────────────────┘ │
│                                                          │
│  ┌────────────────────────────────────────────────────┐ │
│  │                     Scène (Scene)                   │ │
│  │  ┌──────────────┐  ┌──────────────┐               │ │
│  │  │   Stockage   │  │  Ressources  │               │ │
│  │  │  (Entités)   │  │  (Globales)  │               │ │
│  │  └──────────────┘  └──────────────┘               │ │
│  │  ┌──────────────┐  ┌──────────────┐               │ │
│  │  │  Événements  │  │  Commandes   │               │ │
│  │  └──────────────┘  └──────────────┘               │ │
│  └────────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────┘
```

## Flux d'Exécution

### 1. Phase de Démarrage

```cpp
Application{}
    .add_systems<setup_system, init_resources>(Schedule::STARTUP)
    .run();
```

Les systèmes dans le schedule STARTUP s'exécutent une fois avant la boucle principale :

```
┌─────────────┐
│   DÉMARRAGE │
└──────┬──────┘
       │
       v
┌─────────────────┐
│ Systèmes STARTUP│
└──────┬──────────┘
       │
       v
┌─────────────┐
│ Boucle Princ.│
└─────────────┘
```

### 2. Boucle de Mise à Jour

La boucle principale s'exécute de manière répétée :

```
┌──────────────────────────────────────────┐
│            Schedule UPDATE               │
│                                          │
│  Entrée → Logique → Physique → Rendu     │
│    ↓       ↓        ↓          ↓         │
│  Systèmes s'exécutant avec dépendances   │
│                                          │
│  Commandes appliquées entre les systèmes │
│  Événements distribués entre les systèmes│
└──────────────────────────────────────────┘
       │
       v (Image suivante)
```

### 3. Dépendances des Systèmes

Les systèmes peuvent spécifier l'ordre d'exécution :

```cpp
Application{}
    .add_systems<input_system, movement_system, render_system>(Schedule::UPDATE)
    .after<input_system>()  // movement_system s'exécute après input_system
    .run();
```

## Stockage basé sur les Archétypes

R-Engine utilise un **stockage basé sur les archétypes** pour des performances optimales.

### Qu'est-ce qu'un Archétype ?

Un archétype est une combinaison unique de types de composants. Les entités avec les mêmes composants partagent le même archétype :

```
Archétype A: [Position, Velocity]
  Entité 1: Position{0,0}, Velocity{1,0}
  Entité 2: Position{5,3}, Velocity{-1,2}
  ...

Archétype B: [Position, Velocity, Health]
  Entité 10: Position{2,1}, Velocity{0,1}, Health{100}
  Entité 11: Position{-3,4}, Velocity{2,0}, Health{75}
  ...
```

### Avantages

- **Optimisé pour le cache**: Les composants du même type sont stockés de manière contiguë
- **Itération rapide**: Les requêtes itèrent sur des archétypes entiers à la fois
- **Efficacité mémoire**: Pas d'indirection de pointeur

### Changements d'Archétype

Quand des composants sont ajoutés ou supprimés, les entités se déplacent entre archétypes :

```cpp
// L'entité commence dans l'archétype [Position, Velocity]
commands.spawn(Position{}, Velocity{});

// Ajouter Health déplace l'entité vers l'archétype [Position, Velocity, Health]
entity_commands.insert(Health{100});

// Supprimer Velocity déplace l'entité vers l'archétype [Position, Health]
entity_commands.remove<Velocity>();
```

## Organisation Mémoire

```
┌──────────────────────────────────────────────────┐
│              Stockage par Archétype              │
├──────────────────────────────────────────────────┤
│ Archétype [Position, Velocity]                   │
│ ┌──────────────────────────────────────────────┐ │
│ │ Positions: [P1][P2][P3][P4]...               │ │
│ │ Velocités: [V1][V2][V3][V4]...               │ │
│ │ IDs Entité: [E1][E2][E3][E4]...              │ │
│ └──────────────────────────────────────────────┘ │
│                                                  │
│ Archétype [Position, Health]                     │
│ ┌──────────────────────────────────────────────┐ │
│ │ Positions: [P5][P6][P7]...                   │ │
│ │ Santé:    [H5][H6][H7]...                    │ │
│ │ IDs Entité: [E5][E6][E7]...                  │ │
│ └──────────────────────────────────────────────┘ │
└──────────────────────────────────────────────────┘
```

## Exécution des Requêtes

Les requêtes itèrent efficacement sur les archétypes correspondants :

```cpp
void system(Query<Mut<Position>, Ref<Velocity>> query) {
    // Itère automatiquement sur tous les archétypes
    // qui ont Position ET Velocity
    for (auto [pos, vel] : query) {
        // Traitement...
    }
}
```

### Correspondance des Requêtes

```
Archétypes Disponibles:
  [Position, Velocity]       ← Correspond ✓
  [Position, Velocity, HP]   ← Correspond ✓
  [Position, Sprite]         ← Pas de correspondance ✗
  [Velocity]                 ← Pas de correspondance ✗
```

## Tampon de Commandes (Command Buffer)

Les commandes sont différées pour éviter l'invalidation des itérateurs :

```
Exécution du Système:
┌────────────────────┐
│   Le système tourne│
│   Les requêtes itèrent│
│   Commandes en file│
└─────────┬──────────┘
          │
          v
┌────────────────────┐
│ Commandes appliquées│
│ Entités créées     │
│ Composants ajoutés │
│ Archétypes mis à jour│
└────────────────────┘
```

Cela garantit que les modifications n'affectent pas l'itération en cours.

## Prochaines Étapes

- Explorez les [Concepts Fondamentaux](./core-concepts/index.md) pour des explications détaillées
- Consultez le [Système de Stockage](./storage.md) pour les détails d'implémentation
- Regardez les [Exemples](./examples/index.md) pour du code pratique
