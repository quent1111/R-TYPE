# R-Type Server Administration

## 🎯 Vue d'ensemble

Le système d'administration permet de gérer le serveur R-Type en temps réel via une interface graphique.

## 🚀 Lancement

```bash
# Lancer le panel admin
./r-type.sh admin

# Ou directement (depuis le dossier build/bin)
./r-type_admin [host] [port]

# Exemples
./r-type_admin                    # Connecte à localhost:4242
./r-type_admin 192.168.1.100      # Connecte à une IP spécifique
./r-type_admin localhost 4242     # Port personnalisé
```

## 🔐 Authentification

**Mot de passe par défaut** : `admin123`

> ⚠️ **Important** : Changez ce mot de passe en production !

Le mot de passe est configuré dans [`server/src/game/ServerCore.cpp`](server/src/game/ServerCore.cpp#L17) (ligne 17).

## 📊 Fonctionnalités

### Panel principal

- **Statut du serveur** : Uptime, nombre de joueurs, lobbies actifs
- **Liste des joueurs connectés** : ID, adresse IP, port
- **Liste des lobbies** : Nom, joueurs, état (Waiting/In Game)
- **Boutons d'actions rapides** : Refresh, Announce, etc.

### Commandes disponibles

Le client admin communique avec le serveur via des commandes :

| Commande | Description | Exemple |
|----------|-------------|---------|
| `list-players` | Liste tous les joueurs | Auto |
| `kick <id>` | Expulser un joueur | `kick 5` |
| `list-lobbies` | Liste les lobbies | Auto |
| `close-lobby <id>` | Fermer un lobby | `close-lobby 2` |
| `status` | État du serveur | Auto |
| `announce <msg>` | Message global | `announce Maintenance in 5 min` |
| `help` | Liste des commandes | `help` |

## 🔧 Architecture

### Côté serveur

```
server/
├── include/admin/
│   └── AdminManager.hpp       # Gestionnaire admin
└── src/admin/
    └── AdminManager.cpp       # Implémentation
```

**Opcodes ajoutés** :
- `0xA0` : `AdminLogin` - Authentification
- `0xA1` : `AdminLoginAck` - Réponse d'authentification
- `0xA2` : `AdminCommand` - Envoi de commande
- `0xA3` : `AdminResponse` - Réponse à une commande
- `0xA4` : `AdminLogout` - Déconnexion admin

### Côté client

```
admin-client/
├── include/
│   ├── AdminClient.hpp        # Communication réseau
│   ├── AdminUI.hpp            # Interface principale
│   └── LoginScreen.hpp        # Écran de connexion
└── src/
    ├── main.cpp
    ├── AdminClient.cpp
    ├── AdminUI.cpp
    └── LoginScreen.cpp
```

## 🎨 Interface

L'interface graphique est construite avec SFML et affiche :

1. **Écran de connexion** (saisie du mot de passe)
2. **Dashboard principal** avec :
   - Panel de statut serveur (en haut à gauche)
   - Liste des joueurs (centre)
   - Liste des lobbies (bas)
   - Boutons d'action (droite)

## 🔒 Sécurité

### Authentification

- Le mot de passe est hashé (fonction de hash simple pour le moment)
- Sessions avec timeout automatique (30 minutes d'inactivité)
- Logs de toutes les tentatives de connexion et actions admin

### Bonnes pratiques

1. **Changez le mot de passe par défaut**
2. Limitez l'accès réseau au port admin
3. Surveillez les logs d'administration
4. Utilisez un VPN pour l'accès distant

## 📝 Logs

Les actions admin sont loguées dans la console serveur :

```
[AdminManager] Initialized with authentication
[AdminManager] Client 5 authenticated as admin
[AdminManager] Client 5 requested full game state
[AdminManager] Admin client 5 logged out
```

## 🛠️ Développement

### Ajouter une nouvelle commande

1. **Ajouter le type dans AdminManager.hpp** :
```cpp
enum class Type {
    // ... existing
    NewCommand
};
```

2. **Parser la commande** (AdminManager.cpp) :
```cpp
else if (command == "new-command") {
    cmd.type = AdminCommand::Type::NewCommand;
}
```

3. **Implémenter l'exécution** :
```cpp
case AdminCommand::Type::NewCommand:
    return execute_new_command(cmd.args, server);
```

4. **Ajouter dans le help** :
```cpp
ss << "new-command - Description|";
```

### Modifier le mot de passe

Éditez [`server/src/game/ServerCore.cpp`](server/src/game/ServerCore.cpp) :

```cpp
ServerCore::ServerCore()
    : _lobby_manager(4),
      _lobby_command_handler(_lobby_manager),
      _admin_manager(std::make_unique<AdminManager>("VotreNouveauMotDePasse")) {
    // ...
}
```

## 🐛 Dépannage

### Le client admin ne se connecte pas

- Vérifiez que le serveur est lancé
- Vérifiez l'IP et le port
- Vérifiez que le firewall autorise le port 4242

### Authentification échoue

- Vérifiez le mot de passe (sensible à la casse)
- Vérifiez les logs serveur pour voir les tentatives

### Interface ne s'affiche pas correctement

- Vérifiez que la police `assets/fonts/arial.ttf` existe
- Vérifiez la résolution d'écran (min. 1280x900)

## 🚧 TODO / Améliorations futures

- [ ] Chiffrement du mot de passe (SHA256)
- [ ] Multi-niveaux de privilèges (admin, moderator)
- [ ] Graphiques temps réel (courbe joueurs)
- [ ] Export des statistiques (CSV/JSON)
- [ ] Interface web alternative
- [ ] Filtres et recherche dans les listes
- [ ] Ban IP permanent
- [ ] Configuration du serveur en live

## 📞 Support

En cas de problème, consultez :
- Les logs du serveur
- Les logs du client admin
- La documentation principale du projet
