// ========================================
// Translation Data
// ========================================

const translations = {
    en: {
        // Language selector
        langSelector: {
            label: "Language",
            en: "English",
            fr: "Français"
        },
        
        // Logo
        logo: {
            title: "R-Engine",
            subtitle: "ECS Documentation"
        },
        
        // Navigation
        nav: {
            introduction: "Introduction",
            architecture: "Architecture",
            coreConcepts: "Core Concepts",
            entities: "Entities",
            components: "Components",
            systems: "Systems",
            resources: "Resources",
            queries: "Queries",
            commands: "Commands",
            advanced: "Advanced Features",
            events: "Events",
            hierarchies: "Hierarchies",
            runConditions: "Run Conditions",
            schedules: "Schedules",
            storage: "Storage System",
            examples: "Examples",
            apiReference: "API Reference"
        },
        
        // Search
        search: {
            placeholder: "Search..."
        },
        
        // Introduction
        introduction: {
            title: "Introduction to R-Engine ECS",
            lead: "R-Engine uses an <strong>Entity Component System (ECS)</strong> architecture inspired by Bevy, designed to provide performance, flexibility and ease of use.",
            whatIsECS: "What is an ECS?",
            whatIsECSDesc: "An ECS is an architectural design pattern that separates data (Components) from logic (Systems), connecting them through entities (Entities). This approach favors composition over inheritance and enables more modular and performant code.",
            keyFeatures: "Key Features",
            features: {
                archetype: "<strong>Archetype-based Storage</strong>: Archetype-optimized storage for maximum performance",
                typeSafety: "<strong>Type Safety</strong>: Compile-time type system to avoid errors",
                di: "<strong>Dependency Injection</strong>: Automatic system dependency resolution",
                commandBuffer: "<strong>Command Buffer</strong>: Deferred modifications to avoid iterator invalidation",
                events: "<strong>Events System</strong>: Asynchronous communication between systems",
                hierarchy: "<strong>Hierarchy Support</strong>: Native parent-child relationships"
            }
        },
        
        // Architecture
        architecture: {
            title: "System Architecture",
            executionFlow: "Execution Flow",
            flowSteps: {
                init: "Initialization",
                initDesc: "Scene creation and STARTUP systems execution",
                mainLoop: "Main Loop",
                update: "Execute UPDATE systems (game logic)",
                applyCommands: "Apply Command Buffer (structural modifications)",
                render: "Execute RENDER systems (display)",
                updateEvents: "Update Events (double buffering)"
            }
        },
        
        // Core Concepts
        coreConcepts: {
            title: "Core Concepts",
            
            // Entities
            entities: {
                title: "Entities",
                desc: "An entity is simply a unique identifier (<code>u32</code>) that groups components together. It has no data or behavior of its own.",
                creating: "Creating Entities",
                example: "Example"
            },
            
            // Components
            components: {
                title: "Components",
                desc: "Components are pure data structures that define an entity's properties. They contain no logic, only data.",
                bestPractices: "Best Practices",
                practices: {
                    simple: "Keep components simple and focused",
                    pod: "Use POD (Plain Old Data) structs as much as possible",
                    multiple: "Prefer multiple small components to one monolithic component",
                    markers: "Use marker components (without data) to tag entities"
                },
                examples: "Component Examples"
            },
            
            // Systems
            systems: {
                title: "Systems",
                desc: "Systems are functions that implement your application's logic. They operate on entities via queries and can access global resources.",
                anatomy: "Anatomy of a System",
                basicStructure: "Basic Structure",
                parameters: "System Parameters",
                paramTable: {
                    type: "Type",
                    description: "Description",
                    usage: "Usage",
                    query: "Iterate over entities with certain components",
                    queryUsage: "Logic on multiple entities",
                    res: "Read-only access to a resource",
                    resUsage: "Read configuration, time, etc.",
                    resMut: "Mutable access to a resource",
                    resMutUsage: "Modify global state",
                    commands: "Deferred command buffer",
                    commandsUsage: "Spawn, despawn, add/remove components",
                    eventWriter: "Send events",
                    eventWriterUsage: "Asynchronous communication",
                    eventReader: "Read events",
                    eventReaderUsage: "React to events"
                },
                completeExample: "Complete Example",
                registration: "Registering Systems"
            },
            
            // Resources
            resources: {
                title: "Resources",
                desc: "Resources are global, unique data accessible by all systems. They are perfect for storing configuration, global state, or services.",
                defAndUsage: "Definition and Usage"
            },
            
            // Queries
            queries: {
                title: "Queries",
                desc: "Queries allow you to iterate over entities that match certain criteria. They support multiple types of filters and access patterns.",
                accessTypes: "Access Types",
                accessTable: {
                    wrapper: "Wrapper",
                    description: "Description",
                    example: "Example",
                    mut: "Mutable access to component T",
                    ref: "Read-only access to component T",
                    with: "Filter: requires component T (no access)",
                    without: "Filter: excludes entities with T",
                    optional: "Optional access (nullptr if absent)"
                },
                examples: "Query Examples",
                warning: "⚠️ Important",
                warningText: "Never modify the ECS structure (spawn, despawn, add/remove components) while iterating a query. Use <code>Commands</code> instead to defer these operations."
            },
            
            // Commands
            commands: {
                title: "Commands",
                desc: "The command system allows you to defer structural ECS modifications (entity creation/destruction, component addition/removal) to avoid iterator invalidation during system execution.",
                api: "Commands API",
                operations: "Available Operations",
                hierarchies: "Parent-Child Hierarchies",
                hierarchyRelations: "Hierarchical Relations",
                applyMoment: "💡 Application Timing",
                applyMomentText: "All commands are automatically applied at the end of the current schedule, just before the next system application. You don't need to manually call <code>apply()</code>."
            }
        },
        
        // Advanced Features
        advanced: {
            title: "Advanced Features",
            
            // Events
            events: {
                title: "Events",
                desc: "The event system enables asynchronous communication between systems. Events use double buffering: events sent at frame N are readable at frame N+1.",
                defAndUsage: "Definition and Usage",
                warning: "⚠️ Lifetime",
                warningText: "Events only persist for one frame. They are automatically cleaned up at the beginning of each new frame cycle."
            },
            
            // Hierarchies
            hierarchies: {
                title: "Hierarchies",
                desc: "R-Engine natively supports parent-child relationships via the <code>Parent</code> and <code>Children</code> components.",
                components: "Hierarchy Components",
                autoComponents: "Automatic components",
                creation: "Creating a Hierarchy",
                traversal: "Traversing a Hierarchy"
            },
            
            // Run Conditions
            runConditions: {
                title: "Run Conditions",
                desc: "Run conditions allow systems to execute conditionally, based on application state or event presence.",
                available: "Available Conditions",
                custom: "Custom Conditions",
                creating: "Creating a Condition"
            },
            
            // Schedules
            schedules: {
                title: "Schedules",
                desc: "Schedules organize system execution into different phases. R-Engine provides several predefined schedules.",
                scheduleTable: {
                    schedule: "Schedule",
                    when: "When",
                    usage: "Usage",
                    startup: "Once at startup",
                    startupUsage: "Initialization, spawn initial entities",
                    preUpdate: "Before UPDATE",
                    preUpdateUsage: "Preparation, frame cleanup",
                    update: "Every frame",
                    updateUsage: "Game logic, physics, AI",
                    postUpdate: "After UPDATE",
                    postUpdateUsage: "Post-processing, finalization",
                    render2d: "2D render phase",
                    render2dUsage: "2D drawing (sprites, UI)",
                    render3d: "3D render phase",
                    render3dUsage: "3D drawing (models, lights)"
                },
                executionOrder: "Execution Order",
                definingOrder: "Defining Order"
            }
        },
        
        // Storage
        storage: {
            title: "Storage System",
            desc: "R-Engine uses <strong>archetype-based</strong> storage to optimize performance. This approach organizes entities by unique component combinations.",
            archetypes: "Archetypes",
            archetypeDesc: "An <strong>archetype</strong> represents a unique combination of component types. All entities with exactly the same components are stored in the same archetype.",
            exampleOrganization: "Example Organization",
            columnarTables: "Columnar Tables",
            columnarDesc: "Each archetype stores its components in columns (Structure of Arrays - SoA), which optimizes memory locality and enables efficient vectorization.",
            transitions: "Archetype Transitions",
            transitionsDesc: "When a component is added or removed from an entity, it changes archetype. R-Engine uses a cache (edges) to optimize these frequent transitions.",
            transitionExample: "Transition Example",
            advantages: "System Advantages",
            advantagesList: {
                cacheLocality: "<strong>Cache Locality</strong>: Data is contiguous in memory",
                fastIteration: "<strong>Fast Iteration</strong>: Queries only iterate over relevant archetypes",
                noFragmentation: "<strong>No Fragmentation</strong>: Swap-and-pop for deletions",
                typeSafety: "<strong>Type Safety</strong>: Compile-time checks",
                scalability: "<strong>Scalability</strong>: Constant performance with many entities"
            },
            internalApi: "Internal API",
            archetypeStructure: "Archetype Structure"
        },
        
        // Examples
        examples: {
            title: "Complete Examples",
            example1: {
                title: "Example 1: Bouncing Balls",
                desc: "Complete demo of a bouncing balls system with physics and rendering."
            },
            example2: {
                title: "Example 2: Event System",
                desc: "Demonstration of the event system for inter-system communication."
            },
            example3: {
                title: "Example 3: States & Menus",
                desc: "Using states to manage different game phases."
            }
        },
        
        // API Reference
        apiReference: {
            title: "Complete API Reference",
            scene: {
                title: "Scene",
                desc: "Main ECS container.",
                method: "Method",
                description: "Description"
            },
            commands: {
                title: "Commands",
                desc: "Interface for deferred modifications."
            },
            entityCommands: {
                title: "EntityCommands",
                desc: "Builder for entity modification."
            },
            query: {
                title: "Query<Wrappers...>",
                desc: "Entity iteration.",
                iterator: "Query::Iterator"
            },
            eventWriter: {
                title: "EventWriter<T>",
                desc: "Event sending."
            },
            eventReader: {
                title: "EventReader<T>",
                desc: "Event reading."
            }
        },
        
        // Footer
        footer: {
            copyright: "© 2025 R-Engine Project. Documentation generated for ECS."
        }
    },
    
    fr: {
        // Sélecteur de langue
        langSelector: {
            label: "Langue",
            en: "English",
            fr: "Français"
        },
        
        // Logo
        logo: {
            title: "R-Engine",
            subtitle: "Documentation ECS"
        },
        
        // Navigation
        nav: {
            introduction: "Introduction",
            architecture: "Architecture",
            coreConcepts: "Concepts Clés",
            entities: "Entities",
            components: "Components",
            systems: "Systems",
            resources: "Resources",
            queries: "Queries",
            commands: "Commands",
            advanced: "Fonctionnalités Avancées",
            events: "Events",
            hierarchies: "Hierarchies",
            runConditions: "Run Conditions",
            schedules: "Schedules",
            storage: "Système de Stockage",
            examples: "Exemples",
            apiReference: "Référence API"
        },
        
        // Recherche
        search: {
            placeholder: "Rechercher..."
        },
        
        // Introduction
        introduction: {
            title: "Introduction à l'ECS R-Engine",
            lead: "R-Engine utilise une architecture <strong>Entity Component System (ECS)</strong> inspirée de Bevy, conçue pour offrir performance, flexibilité et simplicité d'utilisation.",
            whatIsECS: "Qu'est-ce qu'un ECS ?",
            whatIsECSDesc: "Un ECS est un patron de conception architectural qui sépare les données (Components) de la logique (Systems), en les reliant via des entités (Entities). Cette approche favorise la composition plutôt que l'héritage et permet un code plus modulaire et performant.",
            keyFeatures: "Caractéristiques Principales",
            features: {
                archetype: "<strong>Archétype-based Storage</strong> : Stockage optimisé par archétype pour des performances maximales",
                typeSafety: "<strong>Type Safety</strong> : Système de types compile-time pour éviter les erreurs",
                di: "<strong>Dependency Injection</strong> : Résolution automatique des dépendances système",
                commandBuffer: "<strong>Command Buffer</strong> : Modifications différées pour éviter l'invalidation des itérateurs",
                events: "<strong>Events System</strong> : Communication asynchrone entre systèmes",
                hierarchy: "<strong>Hierarchy Support</strong> : Relations parent-enfant natives"
            }
        },
        
        // Architecture
        architecture: {
            title: "Architecture du Système",
            executionFlow: "Flux d'Exécution",
            flowSteps: {
                init: "Initialisation",
                initDesc: "Création de la Scene et exécution des systèmes STARTUP",
                mainLoop: "Boucle Principale",
                update: "Exécution des systèmes UPDATE (logique du jeu)",
                applyCommands: "Application du Command Buffer (modifications structurelles)",
                render: "Exécution des systèmes RENDER (affichage)",
                updateEvents: "Update des Events (double buffering)"
            }
        },
        
        // Concepts Clés
        coreConcepts: {
            title: "Concepts Clés",
            
            // Entities
            entities: {
                title: "Entities (Entités)",
                desc: "Une entité est simplement un identifiant unique (<code>u32</code>) qui regroupe des composants. Elle n'a pas de données ou de comportement propre.",
                creating: "Création d'Entités",
                example: "Exemple"
            },
            
            // Components
            components: {
                title: "Components (Composants)",
                desc: "Les composants sont des structures de données pures qui définissent les propriétés d'une entité. Ils ne contiennent pas de logique, seulement des données.",
                bestPractices: "Bonnes Pratiques",
                practices: {
                    simple: "Garder les composants simples et focused",
                    pod: "Utiliser des structs POD (Plain Old Data) autant que possible",
                    multiple: "Préférer plusieurs petits composants à un gros composant monolithique",
                    markers: "Utiliser des marker components (sans données) pour taguer des entités"
                },
                examples: "Exemples de Composants"
            },
            
            // Systems
            systems: {
                title: "Systems (Systèmes)",
                desc: "Les systèmes sont des fonctions qui implémentent la logique de votre application. Ils opèrent sur des entités via des queries et peuvent accéder aux ressources globales.",
                anatomy: "Anatomie d'un Système",
                basicStructure: "Structure de Base",
                parameters: "Paramètres de Système",
                paramTable: {
                    type: "Type",
                    description: "Description",
                    usage: "Utilisation",
                    query: "Itérer sur des entités avec certains composants",
                    queryUsage: "Logique sur plusieurs entités",
                    res: "Accès lecture seule à une ressource",
                    resUsage: "Lire configuration, temps, etc.",
                    resMut: "Accès mutable à une ressource",
                    resMutUsage: "Modifier état global",
                    commands: "Buffer de commandes différées",
                    commandsUsage: "Spawn, despawn, add/remove components",
                    eventWriter: "Envoyer des événements",
                    eventWriterUsage: "Communication asynchrone",
                    eventReader: "Lire des événements",
                    eventReaderUsage: "Réagir aux événements"
                },
                completeExample: "Exemple Complet",
                registration: "Enregistrement des Systèmes"
            },
            
            // Resources
            resources: {
                title: "Resources (Ressources)",
                desc: "Les ressources sont des données globales, uniques, accessibles par tous les systèmes. Elles sont parfaites pour stocker la configuration, l'état global, ou des services.",
                defAndUsage: "Définition et Utilisation"
            },
            
            // Queries
            queries: {
                title: "Queries (Requêtes)",
                desc: "Les queries permettent d'itérer sur des entités qui correspondent à certains critères. Elles supportent plusieurs types de filtres et d'accès.",
                accessTypes: "Types d'Accès",
                accessTable: {
                    wrapper: "Wrapper",
                    description: "Description",
                    example: "Exemple",
                    mut: "Accès mutable au composant T",
                    ref: "Accès lecture seule au composant T",
                    with: "Filtre: requiert le composant T (sans accès)",
                    without: "Filtre: exclut les entités avec T",
                    optional: "Accès optionnel (nullptr si absent)"
                },
                examples: "Exemples de Queries",
                warning: "⚠️ Important",
                warningText: "Ne modifiez jamais la structure de l'ECS (spawn, despawn, add/remove components) pendant l'itération d'une query. Utilisez plutôt <code>Commands</code> pour différer ces opérations."
            },
            
            // Commands
            commands: {
                title: "Commands (Commandes)",
                desc: "Le système de commandes permet de différer les modifications structurelles de l'ECS (création/destruction d'entités, ajout/suppression de composants) pour éviter l'invalidation des itérateurs pendant l'exécution des systèmes.",
                api: "API des Commandes",
                operations: "Opérations Disponibles",
                hierarchies: "Hiérarchies Parent-Enfant",
                hierarchyRelations: "Relations Hiérarchiques",
                applyMoment: "💡 Moment d'Application",
                applyMomentText: "Toutes les commandes sont appliquées automatiquement à la fin du schedule courant, juste avant l'application suivante des systèmes. Vous n'avez pas besoin d'appeler manuellement <code>apply()</code>."
            }
        },
        
        // Fonctionnalités Avancées
        advanced: {
            title: "Fonctionnalités Avancées",
            
            // Events
            events: {
                title: "Events (Événements)",
                desc: "Le système d'événements permet une communication asynchrone entre systèmes. Les événements utilisent un double buffering: les événements envoyés à la frame N sont lisibles à la frame N+1.",
                defAndUsage: "Définition et Utilisation",
                warning: "⚠️ Durée de Vie",
                warningText: "Les événements ne persistent qu'une seule frame. Ils sont automatiquement nettoyés au début de chaque nouveau cycle de frame."
            },
            
            // Hierarchies
            hierarchies: {
                title: "Hierarchies (Hiérarchies)",
                desc: "R-Engine supporte nativement les relations parent-enfant via les composants <code>Parent</code> et <code>Children</code>.",
                components: "Composants de Hiérarchie",
                autoComponents: "Composants automatiques",
                creation: "Création d'une Hiérarchie",
                traversal: "Parcourir une Hiérarchie"
            },
            
            // Run Conditions
            runConditions: {
                title: "Run Conditions (Conditions d'Exécution)",
                desc: "Les run conditions permettent d'exécuter des systèmes conditionnellement, en fonction de l'état de l'application ou de la présence d'événements.",
                available: "Conditions Disponibles",
                custom: "Conditions Personnalisées",
                creating: "Créer une Condition"
            },
            
            // Schedules
            schedules: {
                title: "Schedules (Planificateurs)",
                desc: "Les schedules organisent l'exécution des systèmes en différentes phases. R-Engine fournit plusieurs schedules prédéfinis.",
                scheduleTable: {
                    schedule: "Schedule",
                    when: "Quand",
                    usage: "Usage",
                    startup: "Une fois au démarrage",
                    startupUsage: "Initialisation, spawn des entités initiales",
                    preUpdate: "Avant UPDATE",
                    preUpdateUsage: "Préparation, nettoyage de frame",
                    update: "Chaque frame",
                    updateUsage: "Logique du jeu, physique, AI",
                    postUpdate: "Après UPDATE",
                    postUpdateUsage: "Post-traitement, finalisation",
                    render2d: "Phase de rendu 2D",
                    render2dUsage: "Dessin 2D (sprites, UI)",
                    render3d: "Phase de rendu 3D",
                    render3dUsage: "Dessin 3D (modèles, lumières)"
                },
                executionOrder: "Ordre d'Exécution",
                definingOrder: "Définir l'Ordre"
            }
        },
        
        // Stockage
        storage: {
            title: "Système de Stockage",
            desc: "R-Engine utilise un stockage basé sur les <strong>archétypes</strong> pour optimiser les performances. Cette approche organise les entités par combinaison unique de composants.",
            archetypes: "Archétypes",
            archetypeDesc: "Un <strong>archetype</strong> représente une combinaison unique de types de composants. Toutes les entités avec exactement les mêmes composants sont stockées dans le même archetype.",
            exampleOrganization: "Exemple d'Organisation",
            columnarTables: "Tables Columnar",
            columnarDesc: "Chaque archetype stocke ses composants dans des colonnes (Structure of Arrays - SoA), ce qui optimise la localité mémoire et permet une vectorisation efficace.",
            transitions: "Transitions d'Archétypes",
            transitionsDesc: "Lorsqu'un composant est ajouté ou retiré d'une entité, celle-ci change d'archetype. R-Engine utilise un cache (edges) pour optimiser ces transitions fréquentes.",
            transitionExample: "Exemple de Transition",
            advantages: "Avantages du Système",
            advantagesList: {
                cacheLocality: "<strong>Cache Locality</strong> : Les données sont contiguës en mémoire",
                fastIteration: "<strong>Itération Rapide</strong> : Les queries n'itèrent que sur les archétypes pertinents",
                noFragmentation: "<strong>Pas de Fragmentation</strong> : Swap-and-pop pour les suppressions",
                typeSafety: "<strong>Type Safety</strong> : Vérifications à la compilation",
                scalability: "<strong>Scalabilité</strong> : Performance constante avec beaucoup d'entités"
            },
            internalApi: "API Interne",
            archetypeStructure: "Structure Archetype"
        },
        
        // Exemples
        examples: {
            title: "Exemples Complets",
            example1: {
                title: "Exemple 1: Bouncing Balls",
                desc: "Démo complète d'un système de balles rebondissantes avec physique et rendu."
            },
            example2: {
                title: "Exemple 2: Event System",
                desc: "Démonstration du système d'événements pour la communication entre systèmes."
            },
            example3: {
                title: "Exemple 3: States & Menus",
                desc: "Utilisation des états pour gérer différentes phases du jeu."
            }
        },
        
        // Référence API
        apiReference: {
            title: "Référence API Complète",
            scene: {
                title: "Scene",
                desc: "Container principal de l'ECS.",
                method: "Méthode",
                description: "Description"
            },
            commands: {
                title: "Commands",
                desc: "Interface pour modifications différées."
            },
            entityCommands: {
                title: "EntityCommands",
                desc: "Builder pour modification d'entités."
            },
            query: {
                title: "Query<Wrappers...>",
                desc: "Itération sur les entités.",
                iterator: "Query::Iterator"
            },
            eventWriter: {
                title: "EventWriter<T>",
                desc: "Envoi d'événements."
            },
            eventReader: {
                title: "EventReader<T>",
                desc: "Lecture d'événements."
            }
        },
        
        // Footer
        footer: {
            copyright: "© 2025 R-Engine Project. Documentation générée pour l'ECS."
        }
    }
};

// Export for use in other scripts
if (typeof module !== 'undefined' && module.exports) {
    module.exports = translations;
}