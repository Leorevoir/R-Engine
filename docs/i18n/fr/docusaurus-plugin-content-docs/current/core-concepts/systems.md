---
sidebar_position: 3
---

# Systèmes

Les systèmes sont des fonctions qui contiennent la logique de votre application. Ils opèrent sur les entités via des requêtes et peuvent accéder aux ressources globales.

## Qu'est-ce qu'un System?

Un système est une fonction C++ standard qui prend des paramètres ECS et implémente la logique du jeu :

```cpp
void movement_system(
    Query<Mut<Position>, Ref<Velocity>> query,
    Res<DeltaTime> time
) {
    for (auto [pos, vel] : query) {
        pos->x += vel->x * time->dt;
        pos->y += vel->y * time->dt;
    }
}
```

## Paramètres des Systèmes

Les systèmes peuvent accepter différents types de paramètres :

| Type | Description | Cas d'Usage |
|------|-------------|----------|
| `Query<...>` | Itérer sur les entités | Traiter plusieurs entités |
| `Res<T>` | Accès ressource en lecture seule | Lire configuration, temps |
| `ResMut<T>` | Accès ressource mutable | Modifier l'état global |
| `Commands` | Tampon de commandes différé | Créer/détruire des entités |
| `EventWriter<T>` | Envoyer des événements | Communication asynchrone |
| `EventReader<T>` | Lire des événements | Réagir aux événements |

### Paramètre Query

Accéder aux composants d'entité :

```cpp
void system(Query<Mut<Position>, Ref<Velocity>, With<Player>> query) {
    for (auto [pos, vel, _] : query) {
        // Traiter les entités joueur
    }
}
```

[En savoir plus sur Queries →](./queries.md)

### Resource Parameters

Accéder aux données globales :

```cpp
void system(
    Res<Config> config,      // Lecture seule
    ResMut<Score> score      // Mutable
) {
    if (score->value > config->high_score) {
        // Nouveau meilleur score !
    }
}
```

[En savoir plus sur Resources →](./resources.md)

### Commandes Parameter

Modifier le monde ECS :

```cpp
void system(Commands& commands) {
    // Créer une entité
    commands.spawn(Position{}, Velocity{});
    
    // Insérer une ressource
    commands.insert_resource(NewResource{});
}
```

[En savoir plus sur Commands →](./commands.md)

### Event Parameters

Envoyer et recevoir des événements :

```cpp
void system(
    EventWriter<CollisionEvent> writer,
    EventReader<InputEvent> reader
) {
    for (const auto& input : reader.iter()) {
        // Traiter l'entrée
    }
    
    writer.send(CollisionEvent{});
}
```

[En savoir plus sur Events →](../advanced/events.md)

## Registering Systems

Ajoutez des systèmes aux plannings dans votre application :

```cpp
Application{}
    // Systèmes STARTUP (exécutés une fois)
    .add_systems<setup, init>(Schedule::STARTUP)
    
    // Systèmes UPDATE (exécutés chaque frame)
    .add_systems<input, movement, collision>(Schedule::UPDATE)
    
    // Systèmes RENDER
    .add_systems<render_sprites>(Schedule::RENDER_2D)
    .run();
```

## Ordre des Systèmes

### Ordre Automatique

Par défaut, les systèmes s'exécutent dans l'ordre où ils sont ajoutés :

```cpp
.add_systems<system_a, system_b, system_c>(Schedule::UPDATE)
// S'exécute : a → b → c
```

### Dépendances Explicites

Utilisez `.after<>()` et `.before<>()` pour spécifier l'ordre :

```cpp
.add_systems<input_system, movement_system>(Schedule::UPDATE)
.after<input_system>()  // movement_system s'exécute après input_system

.add_systems<cleanup_system>(Schedule::UPDATE)
.before<render_system>()  // cleanup s'exécute avant render
```

### System Sets

Groupez les systèmes liés :

```cpp
.add_systems<physics_a, physics_b>(Schedule::UPDATE)
.in_set(SystemSet::Physics)

.add_systems<render_a, render_b>(Schedule::UPDATE)
.in_set(SystemSet::Rendering)
.after_set(SystemSet::Physics)  // Tout le rendu après la physique
```

[En savoir plus sur System Sets →](../advanced/schedules.md)

## Exécution des Systèmes

### Exécution Unique

Les systèmes s'exécutent une fois par exécution du planning :

```cpp
void system() {
    std::cout << "S'exécute une fois par frame\n";
}
```

### Exécution Conditionnelle

Utilisez les conditions d'exécution pour contrôler quand les systèmes s'exécutent :

```cpp
bool is_paused() {
    // Vérifier l'état de pause
    return /* ... */;
}

.add_systems<game_logic>(Schedule::UPDATE)
.run_if(not_run_condition<is_paused>())
```

[En savoir plus sur Run Conditions →](../advanced/run-conditions.md)

## System Patterns

### Système d'Initialisation

S'exécute une fois au démarrage :

```cpp
void setup_system(Commands& commands) {
    // Initialiser les ressources
    commands.insert_resource(GameConfig{});
    
    // Créer les entités initiales
    commands.spawn(Camera{}, Transform{});
    commands.spawn(Player{}, Position{}, Health{100});
}

// Enregistrer dans le planning STARTUP
.add_systems<setup_system>(Schedule::STARTUP)
```

### Système de Mise à Jour

S'exécute chaque frame :

```cpp
void update_system(
    Query<Mut<Position>, Ref<Velocity>> query,
    Res<DeltaTime> time
) {
    for (auto [pos, vel] : query) {
        pos->x += vel->x * time->dt;
        pos->y += vel->y * time->dt;
    }
}

.add_systems<update_system>(Schedule::UPDATE)
```

### Système Gestionnaire d'Événements

Réagit aux événements :

```cpp
void collision_handler(
    EventReader<CollisionEvent> events,
    Commands& commands
) {
    for (const auto& event : events.iter()) {
        // Gérer la collision
        if (event.damage > 0) {
            commands.entity(event.target).despawn();
        }
    }
}
```

### Système de Nettoyage

Supprimer les entités mortes :

```cpp
void cleanup_dead_entities(
    Query<Entity, With<Dead>> query,
    Commands& commands
) {
    for (auto [entity, _] : query) {
        commands.entity(entity).despawn();
    }
}
```

## Bonnes Pratiques

### ✅ À Faire

- Gardez les systèmes concentrés sur une tâche
- Utilisez les requêtes pour itérer sur les entités
- Préférez l'accès en lecture seule (`Ref<T>`) quand possible
- Utilisez les commandes pour les changements structurels

```cpp
// Bon : Système concentré
void apply_gravity(Query<Mut<Velocity>> query, Res<Gravity> gravity) {
    for (auto [vel] : query) {
        vel->y += gravity->value;
    }
}
```

### ❌ À Éviter

- Ne stockez pas d'état dans les systèmes (utilisez des ressources ou composants)
- Ne modifiez pas les entités directement pendant l'itération de requête
- Ne créez pas de dépendances circulaires

```cpp
// Mauvais : Stockage d'état dans le système
void bad_system() {
    static int counter = 0;  // ❌ Utilisez une ressource à la place
    counter++;
}
```

## Conseils de Performance

### Minimiser l'Accès Mutable

Utilisez `Ref<T>` au lieu de `Mut<T>` quand vous n'avez pas besoin de modifier :

```cpp
// Permet l'exécution parallèle avec d'autres systèmes en lecture seule
void read_only_system(Query<Ref<Position>> query) {
    for (auto [pos] : query) {
        // Juste de la lecture
    }
}
```

### Diviser les Grands Systèmes

Divisez les systèmes complexes en plus petits, concentrés :

```cpp
// Au lieu d'un grand système :
void game_logic() { /* physique, IA, entrées... */ }

// Divisez en systèmes concentrés :
void physics_system() { /* ... */ }
void ai_system() { /* ... */ }
void input_system() { /* ... */ }
```

### Utiliser les Filtres

Filtrez les requêtes pour réduire les itérations :

```cpp
// Traiter uniquement les ennemis actifs
void ai_system(Query<Ref<Position>, With<Enemy>, Without<Dead>> query) {
    for (auto [pos, _] : query) {
        // Logique IA uniquement pour les ennemis vivants
    }
}
```

## Exemple: Complete System Setup

```cpp
// Composants
struct Position { float x, y; };
struct Velocity { float x, y; };
struct Player {};

// Ressources
struct DeltaTime { float dt; };

// Systèmes
void movement_system(
    Query<Mut<Position>, Ref<Velocity>> query,
    Res<DeltaTime> time
) {
    for (auto [pos, vel] : query) {
        pos->x += vel->x * time->dt;
        pos->y += vel->y * time->dt;
    }
}

void input_system(
    Query<Mut<Velocity>, With<Player>> query,
    Res<Input> input
) {
    for (auto [vel, _] : query) {
        vel->x = input->horizontal * 5.0f;
        vel->y = input->vertical * 5.0f;
    }
}

// Application
int main() {
    Application{}
        .insert_resource(DeltaTime{})
        .insert_resource(Input{})
        .add_systems<input_system, movement_system>(Schedule::UPDATE)
        .after<input_system>()  // mouvement après entrée
        .run();
}
```

## Prochaines Étapes

- Apprenez les [Resources](./resources.md) pour l'état global
- Explorez les [Queries](./queries.md) pour l'accès aux entités
- Consultez les [Commands](./commands.md) pour la modification du monde
- Découvrez les [Fonctionnalités Avancées](../advanced/index.md) pour plus de capacités système
