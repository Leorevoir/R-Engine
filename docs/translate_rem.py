#!/usr/bin/env python3
import os
FR_DIR = "/home/matt_nytovo/delivery/R-Type/R-Engine/docs/i18n/fr/docusaurus-plugin-content-docs/current/core-concepts"

replacements = [
    ("# Systems", "# Systèmes"),
    ("# Resources", "# Ressources"),
    ("# Queries", "# Requêtes"),
    ("# Commands", "# Commandes"),
    ("## What is a", "## Qu'est-ce qu'un"),
    ("## What are", "## Que sont les"),
    ("## Creating", "## Créer des"),
    ("## Adding", "## Ajouter des"),
    ("## Accessing", "## Accéder aux"),
    ("## Modifying", "## Modifier les"),
    ("## Removing", "## Supprimer les"),
    ("## Using", "## Utiliser les"),
    ("## Spawning Entities", "## Créer des Entités"),
    ("## System Parameters", "## Paramètres des Systèmes"),
    ("## System Execution", "## Exécution des Systèmes"),
    ("## System Ordering", "## Ordre des Systèmes"),
    ("## Query Types", "## Types de Requêtes"),
    ("## Query Filters", "## Filtres de Requête"),
    ("## Iterating Queries", "## Itérer les Requêtes"),
    ("## Entity Commands", "## Commandes d'Entité"),
    ("## Common Use Cases", "## Cas d'Usage Courants"),
    ("### ✅ Do", "### ✅ À Faire"),
    ("### ❌ Don't", "### ❌ À Éviter"),
    ("Learn more about", "En savoir plus sur"),
    ("See how", "Voyez comment"),
]

for filename in ["systems.md", "resources.md", "queries.md", "commands.md"]:
    filepath = os.path.join(FR_DIR, filename)
    with open(filepath, 'r', encoding='utf-8') as f:
        content = f.read()
    
    for en, fr in replacements:
        content = content.replace(en, fr)
    
    with open(filepath, 'w', encoding='utf-8') as f:
        f.write(content)
    
    print(f"✓ Traduit: {filename}")

print(f"\n4 fichiers traduits")
