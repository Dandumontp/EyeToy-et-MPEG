#!/bin/bash

# Dossier contenant les fichiers (par défaut, le dossier courant)
folder="."

# Parcours tous les fichiers du dossier
for file in "$folder"/*; do
    # Ignore les dossiers
    [ -f "$file" ] || continue

    # Nom du fichier sans extension
    base=$(basename "$file")
    name="${base%.*}"

    # Fichier de sortie
    output="${folder}/${name}.ipu"

    # Supprime les 16 premiers octets et écrit le résultat
    tail -c +17 "$file" > "$output"

    echo "Converti : $file → $output"
done

echo "Terminé !"
