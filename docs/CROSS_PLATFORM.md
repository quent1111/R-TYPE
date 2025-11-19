# 🪟 Windows vs 🐧 Linux - Build Comparison

## ✅ Automatisations identiques sur les deux plateformes

| Fonctionnalité | Linux (`build.sh`) | Windows (`build.bat`) | Automatique ? |
|----------------|--------------------|-----------------------|---------------|
| **Création profil Conan** | ✅ Oui | ✅ Oui | ✅ Auto |
| **Vérification dépendances système** | ✅ Oui (X11 libs) | ⚠️ Non (pas nécessaire) | Linux: avec confirmation |
| **Téléchargement SFML/Asio/GTest** | ✅ Oui | ✅ Oui | ✅ Auto |
| **Configuration CMake C++20** | ✅ Oui | ✅ Oui | ✅ Auto |
| **Compilation parallèle** | ✅ Oui (nproc) | ✅ Oui (NUMBER_OF_PROCESSORS) | ✅ Auto |
| **Correction chemin toolchain** | ✅ Oui | ✅ Oui | ✅ Auto |

---

## 🚀 Workflow utilisateur

### Sur Linux/Mac

```bash
# 1. Prérequis
pip install conan

# 2. Build (une seule commande !)
./scripts/build.sh

# 3. Si demandé, installer dépendances X11 (répondre "y")
# Le script liste exactement quoi installer

# 4. Exécutables prêts dans build/bin/
```

### Sur Windows

```cmd
REM 1. Prérequis
pip install conan
REM Avoir Visual Studio 2019+ avec C++ workload

REM 2. Build (une seule commande !)
scripts\build.bat

REM 3. Exécutables prêts dans build\bin\Release\
```

---

## ⏱️ Temps de build (identique sur les deux OS)

| Build | Linux | Windows | Pourquoi ? |
|-------|-------|---------|------------|
| **Premier** | 5-10 min | 5-10 min | Compile SFML, Asio, GTest depuis sources |
| **Suivants** | 10-30 sec | 10-30 sec | Tout est en cache Conan (~/.conan2) |
| **Clean rebuild** | 10-30 sec | 10-30 sec | Réutilise le cache des dépendances |

---

## 🔍 Différences techniques

### Structure des exécutables

**Linux/Mac :**
```
build/
└── bin/
    ├── r-type_server     (pas d'extension)
    ├── r-type_client
    └── test_sanity
```

**Windows :**
```
build/
└── bin/
    └── Release/          (ou Debug selon config)
        ├── r-type_server.exe
        ├── r-type_client.exe
        └── test_sanity.exe
```

### Dépendances système

**Linux uniquement :**
- Bibliothèques X11 requises pour SFML (détection automatique)
- Installation automatisée avec dnf/apt

**Windows :**
- Pas de dépendances système supplémentaires
- Visual Studio fournit tout le nécessaire

---

## 📝 Résumé

### ✅ Ce qui est **identique** :

1. **Une seule commande** pour build
2. **Automatisation complète** du profil Conan
3. **Téléchargement automatique** des dépendances
4. **Correction automatique** des chemins CMake
5. **Compilation parallèle** optimisée
6. **Temps de build** similaires

### ⚠️ Seule différence :

- **Linux** : Demande confirmation pour installer les libs X11 (une seule fois)
- **Windows** : Pas de dépendances système à installer

---

## 🎯 Pour ton équipe

**Message pour Discord/Slack :**

```
@everyone 🎮

Le build system est maintenant 100% automatisé sur Linux ET Windows !

🐧 Linux/Mac : ./scripts/build.sh
🪟 Windows : scripts\build.bat

Les deux scripts font EXACTEMENT la même chose :
✅ Créent le profil Conan automatiquement
✅ Téléchargent et compilent toutes les dépendances
✅ Configurent CMake avec C++20
✅ Compilent le projet en parallèle

Premier build : 5-10 min (compile tout)
Builds suivants : 10-30 sec (cache)

Pas de différence entre Windows et Linux ! 🚀
```

---

## 🐛 Troubleshooting

### Linux : "Missing X11 libraries"
→ **Répondre `y`** quand le script demande de les installer

### Windows : "CMake not found"
→ Installer CMake depuis https://cmake.org/download/
→ Ajouter au PATH

### Les deux : "Conan not found"
→ `pip install conan`

### Les deux : Build échoue
→ `rm -rf build/` (Linux) ou `rmdir /s build` (Windows)
→ Relancer le script
