# Gestion des Joueurs avec Registry

## Architecture mise en place

### 1. Composants ajoutés (`components.hpp`)
- `network_id` : Stocke l'ID du client réseau
- `entity_tag` : Identifie le type d'entité (PLAYER, ENEMY, etc.)
- `position`, `velocity`, `health`, `collider` : Composants de jeu

### 2. Classe Game améliorée

```cpp
class Game {
private:
    registry _registry;
    std::unordered_map<int, entity> _client_entities;  // client_id -> entity

public:
    // Créer un joueur pour un nouveau client
    entity create_player(int client_id, float start_x = 100.0f, float start_y = 100.0f);

    // Récupérer l'entité d'un joueur
    entity* get_player_entity(int client_id);

    // Supprimer un joueur déconnecté
    void remove_player(int client_id);
};
```

### 3. Utilisation dans la Game Loop

```cpp
// Quand un paquet arrive
NetworkPacket packet;
while (server.get_input_packet(packet)) {
    // 1. Enregistrer le client et obtenir son ID
    int client_id = server.register_client(packet.sender);

    // 2. Vérifier si le joueur existe déjà
    entity* player = get_player_entity(client_id);

    if (player == nullptr) {
        // 3. Nouveau joueur - créer son entité
        create_player(client_id, start_x, start_y);

        // 4. Envoyer message de bienvenue
        std::string msg = "Bienvenue joueur " + std::to_string(client_id);
        // ... envoyer msg au client
    } else {
        // 5. Joueur existant - traiter ses commandes
        // Exemple: déplacer le joueur
        auto& pos = _registry.get_component<position>(*player);
        if (pos.has_value()) {
            // Modifier position selon les inputs...
        }
    }
}
```

## Exemples d'utilisation

### Créer un joueur à la connexion
```cpp
// Dans runGameLoop()
int client_id = server.register_client(packet.sender);
if (get_player_entity(client_id) == nullptr) {
    float start_x = 100.0f + (client_id * 50.0f);  // Décalage
    float start_y = 300.0f;
    entity player = create_player(client_id, start_x, start_y);
    std::cout << "Joueur créé: " << player.id() << std::endl;
}
```

### Accéder aux composants d'un joueur
```cpp
entity* player = get_player_entity(client_id);
if (player) {
    // Lire position
    auto pos_opt = _registry.get_component<position>(*player);
    if (pos_opt.has_value()) {
        std::cout << "Position: " << pos_opt->x << ", " << pos_opt->y << std::endl;
    }

    // Modifier velocity
    auto vel_opt = _registry.get_component<velocity>(*player);
    if (vel_opt.has_value()) {
        vel_opt->vx = 100.0f;  // Déplacer à droite
    }

    // Vérifier santé
    auto hp_opt = _registry.get_component<health>(*player);
    if (hp_opt.has_value() && hp_opt->current <= 0) {
        remove_player(client_id);  // Joueur mort
    }
}
```

### Envoyer la position d'un joueur au client
```cpp
entity* player = get_player_entity(client_id);
if (player) {
    auto pos = _registry.get_component<position>(*player);
    if (pos.has_value()) {
        // Créer paquet avec position
        std::vector<uint8_t> data;
        data.push_back(0x42);  // Magic number
        data.push_back(0xB5);
        // Ajouter type de message (ex: UPDATE_POSITION)
        data.push_back(0x01);
        // Ajouter coordonnées (simplifié - voir serialization)
        std::string msg = std::to_string(pos->x) + "," + std::to_string(pos->y);
        data.insert(data.end(), msg.begin(), msg.end());
        server.send_to_client(client_id, data);
    }
}
```

### Gérer la déconnexion
```cpp
// Dans cleanup ou quand timeout détecté
void on_client_disconnect(int client_id) {
    remove_player(client_id);  // Supprime l'entité et le mapping
    // Optionnel: notifier les autres joueurs
    std::string msg = "Joueur " + std::to_string(client_id) + " déconnecté";
    // ... broadcast
}
```

### Broadcast de tous les joueurs
```cpp
// Envoyer positions de tous les joueurs à tous les clients
void broadcast_all_players() {
    std::vector<uint8_t> data;
    data.push_back(0x42);  // Magic number
    data.push_back(0xB5);
    data.push_back(0x02);  // Type: ALL_PLAYERS

    for (const auto& [client_id, player] : _client_entities) {
        auto pos = _registry.get_component<position>(player);
        if (pos.has_value()) {
            // Ajouter ID + position (format simplifié)
            data.push_back(static_cast<uint8_t>(client_id));
            // ... ajouter pos->x et pos->y (serialization propre recommandée)
        }
    }

    server.send_to_all(data);
}
```

## Architecture recommandée

```
Client envoie input → UDPServer → NetworkPacket
                                      ↓
                                  Game::runGameLoop
                                      ↓
                  ┌──────────────────────────────────┐
                  │  register_client(endpoint)       │
                  │       ↓                           │
                  │  get_player_entity(client_id)    │
                  │       ↓                           │
                  │  Si null: create_player()        │
                  │       ↓                           │
                  │  Traiter commandes               │
                  │       ↓                           │
                  │  Modifier composants (pos, vel)  │
                  │       ↓                           │
                  │  run_systems() (physics, etc)    │
                  │       ↓                           │
                  │  Envoyer state au client         │
                  └──────────────────────────────────┘
```

## Points importants

1. **Thread-safety**: Le mapping `_client_entities` est accédé uniquement dans la game loop (un seul thread)
2. **Entity reuse**: Le registry réutilise les IDs d'entités mortes automatiquement
3. **Components**: Ajoutez autant de composants que nécessaire (weapons, inventory, etc.)
4. **Systems**: Utilisez `_registry.add_system<>()` pour logique réutilisable
5. **Cleanup**: Appelez `remove_player()` lors des timeouts/déconnexions

## Note sur NetworkPacket.sender

Si vous avez des erreurs de compilation avec `packet.sender`, vérifiez que:
1. `NetworkPacket.hpp` définit bien `asio::ip::udp::endpoint sender;`
2. Votre IDE est à jour (parfois l'IntelliSense est en retard)
3. Le fichier compile correctement même si l'IDE souligne l'erreur

Code devrait compiler sans problème si NetworkPacket.hpp est correct.
