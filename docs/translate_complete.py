#!/usr/bin/env python3
import os

FR_DIR = "/home/matt_nytovo/delivery/R-Type/R-Engine/docs/i18n/fr/docusaurus-plugin-content-docs/current/core-concepts"

# Traductions communes
common_replacements = [
    ("Resources are", "Les ressources sont"),
    ("Systems are", "Les systèmes sont"),
    ("Queries allow", "Les requêtes permettent"),
    ("Commands provide", "Les commandes fournissent"),
    ("A resource is", "Une ressource est"),
    ("A system is", "Un système est"),
    ("A query is", "Une requête est"),
    ("are unique, global data accessible by all systems", "sont des données globales uniques accessibles par tous les systèmes"),
    ("are functions that contain logic", "sont des fonctions qui contiennent de la logique"),
    ("allow systems to iterate", "permettent aux systèmes d'itérer"),
    ("provide deferred access", "fournissent un accès différé"),
    
    # Phrases communes
    ("They are perfect for", "Elles sont parfaites pour"),
    ("Unlike components which are attached to entities", "Contrairement aux composants qui sont attachés aux entités"),
    ("is a singleton piece of data that exists outside of entities", "est un élément de données singleton qui existe en dehors des entités"),
    ("are global and unique", "sont globales et uniques"),
    
    # Sections
    ("At Application Startup", "Au Démarrage de l'Application"),
    ("From Systems", "Depuis les Systèmes"),
    ("Read-Only Access", "Accès en Lecture Seule"),
    ("Mutable Access", "Accès Mutable"),
    ("Optional Resources", "Ressources Optionnelles"),
    
    # Instructions
    ("Use", "Utilisez"),
    ("for immutable access", "pour un accès immuable"),
    ("for mutable access", "pour un accès mutable"),
    ("when the resource might not exist", "quand la ressource peut ne pas exister"),
    
    # Exemples et notes
    ("Good:", "Bon :"),
    ("Bad:", "Mauvais :"),
    ("Note:", "Note :"),
    ("Tip:", "Astuce :"),
    ("Warning:", "Attention :"),
    ("Example:", "Exemple :"),
    
    # Descriptions
    ("perfect for storing configuration, global state, or services", "parfaites pour stocker la configuration, l'état global ou les services"),
    ("to modify the ECS world", "pour modifier le monde ECS"),
    ("They prevent iterator invalidation", "Elles préviennent l'invalidation des itérateurs"),
    ("and enable safe structural changes", "et permettent des changements structurels sécurisés"),
    
    # Queries spécifiques
    ("systems to iterate over entities", "aux systèmes d'itérer sur les entités"),
    ("that match specific criteria", "qui correspondent à des critères spécifiques"),
    ("They provide type-safe access to components", "Elles fournissent un accès type-safe aux composants"),
    
    # Autres phrases
    ("Keep resources focused", "Gardez les ressources ciblées"),
    ("Don't store per-entity data", "Ne stockez pas de données par entité"),
    ("Use components instead", "Utilisez des composants à la place"),
    ("Avoid large resources", "Évitez les grandes ressources"),
    ("that change frequently", "qui changent fréquemment"),
]

for filename in ["resources.md", "queries.md", "commands.md"]:
    filepath = os.path.join(FR_DIR, filename)
    with open(filepath, 'r', encoding='utf-8') as f:
        content = f.read()
    
    original_len = len(content)
    
    for en, fr in common_replacements:
        content = content.replace(en, fr)
    
    with open(filepath, 'w', encoding='utf-8') as f:
        f.write(content)
    
    changes = abs(len(content) - original_len)
    print(f"✓ Traduit: {filename} ({changes} caractères modifiés)")

print(f"\n3 fichiers traduits avec plus de détails")
