#!/usr/bin/env python3
"""
Script pour traduire les titres et sections principales de la documentation EN -> FR
"""
import os
import re

# Dictionnaire de traductions clés
TRANSLATIONS = {
    # Titres principaux
    "Storage System": "Système de Stockage",
    "Core Concepts": "Concepts Fondamentaux",
    "Advanced Features": "Fonctionnalités Avancées",
    "API Reference": "Référence API",
    "Examples": "Exemples",
    
    # Sections communes
    "## Overview": "## Vue d'Ensemble",
    "## Getting Started": "## Démarrage Rapide",
    "## Basic Usage": "## Utilisation de Base",
    "## Advanced Usage": "## Utilisation Avancée",
    "## Example": "## Exemple",
    "## Next Steps": "## Prochaines Étapes",
    "## Performance": "## Performance",
    "## Benefits": "## Avantages",
    "## Best Practices": "## Bonnes Pratiques",
    "## API": "## API",
    "## Usage": "## Utilisation",
    "## Features": "## Fonctionnalités",
    "## Description": "## Description",
    "## Syntax": "## Syntaxe",
    "## Parameters": "## Paramètres",
    "## Returns": "## Retourne",
    "## Methods": "## Méthodes",
    "## Properties": "## Propriétés",
    "## Important": "## Important",
    
    # Termes techniques (garder en anglais dans le code mais traduire descriptions)
    "Query": "Query",
    "Commands": "Commands",
    "Entity": "Entity",
    "Component": "Component",
    "System": "System",
    "Resource": "Resource",
    "Event": "Event",
    
    # Phrases communes
    "This page": "Cette page",
    "In this example": "Dans cet exemple",
    "For more information": "Pour plus d'informations",
    "See also": "Voir aussi",
    "Learn more about": "En savoir plus sur",
    "Check out": "Consultez",
    "Explore": "Explorez",
    "Read more": "Lire la suite",
    
    # Notes
    ":::tip": ":::tip",
    ":::warning": ":::warning",
    ":::info": ":::info",
    ":::danger": ":::danger",
    ":::note": ":::note",
}

def translate_file(filepath):
    """Traduit un fichier markdown"""
    with open(filepath, 'r', encoding='utf-8') as f:
        content = f.read()
    
    original_content = content
    
    # Appliquer les traductions
    for en, fr in TRANSLATIONS.items():
        content = content.replace(en, fr)
    
    # Ne sauvegarder que si des changements ont été faits
    if content != original_content:
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write(content)
        return True
    return False

def main():
    docs_fr_path = "/home/matt_nytovo/delivery/R-Type/R-Engine/docs/i18n/fr/docusaurus-plugin-content-docs/current"
    
    translated_count = 0
    for root, dirs, files in os.walk(docs_fr_path):
        for file in files:
            if file.endswith('.md'):
                filepath = os.path.join(root, file)
                if translate_file(filepath):
                    translated_count += 1
                    print(f"✓ Traduit: {filepath}")
    
    print(f"\n{translated_count} fichiers traduits")

if __name__ == "__main__":
    main()
