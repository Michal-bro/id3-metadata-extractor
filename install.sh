#!/bin/bash

# Název vstupního binárního souboru
OUTPUT_FILE="id3_extraktor"

# Kontrola oprávnění (sudo práva jsou potřeba pro instalaci balíčků)
if [ "$EUID" -ne 0 ]; then
    echo "Spusťte skript s oprávněním superuživatele (sudo)."
    exit
fi

# Aktualizace seznamu balíčků
echo "Aktualizace seznamu balíčků..."
apt-get update

# Instalace g++, TagLib a libcurl
echo "Instalace požadovaných balíčků: g++, TagLib, libcurl..."
apt-get install -y g++ libtag1-dev libcurl4-openssl-dev

# Kontrola, zda zdrojový soubor existuje
if [ ! -f "id3_extraktor.cpp" ]; then
    echo "Zdrojový soubor id3_extraktor.cpp nebyl nalezen!"
    exit 1
fi

# Kompilace aplikace
echo "Kompilace aplikace..."
g++ -o $OUTPUT_FILE id3_extraktor.cpp -ltag -lcurl

# Kontrola, zda byla kompilace úspěšná
if [ $? -eq 0 ]; then
    echo "Kompilace dokončena. Výstupní soubor: $OUTPUT_FILE"
else
    echo "Kompilace selhala."
    exit 1
fi
