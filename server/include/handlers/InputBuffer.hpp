#pragma once

#include <cstdint>

#include <chrono>
#include <deque>
#include <vector>

namespace server {

/**
 * @brief Configuration du système d'input delaying
 */
struct InputDelayConfig {
    // Délai en millisecondes avant d'appliquer un input
    // Plus élevé = plus de lag mais meilleure synchronisation multi-joueurs
    static constexpr int INPUT_DELAY_MS = 50;

    // Taille maximale du buffer d'inputs par client
    static constexpr size_t MAX_BUFFERED_INPUTS = 100;

    // Timeout après lequel un input non appliqué est considéré comme périmé
    static constexpr int INPUT_TIMEOUT_MS = 5000;
};

/**
 * @brief Représente un input bufferisé avec son timestamp
 */
struct InputEntry {
    uint32_t client_timestamp;  // Timestamp envoyé par le client (millisecondes)
    uint8_t input_mask;         // Masque des touches pressées
    std::chrono::steady_clock::time_point receive_time;  // Quand le serveur a reçu l'input

    InputEntry(uint32_t timestamp, uint8_t mask)
        : client_timestamp(timestamp),
          input_mask(mask),
          receive_time(std::chrono::steady_clock::now()) {}

    /**
     * @brief Vérifie si l'input est prêt à être appliqué (délai écoulé)
     */
    bool is_ready_to_apply(const std::chrono::steady_clock::time_point& now) const {
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - receive_time);
        return elapsed.count() >= InputDelayConfig::INPUT_DELAY_MS;
    }

    /**
     * @brief Vérifie si l'input est périmé (trop vieux)
     */
    bool is_expired(const std::chrono::steady_clock::time_point& now) const {
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - receive_time);
        return elapsed.count() >= InputDelayConfig::INPUT_TIMEOUT_MS;
    }
};

/**
 * @brief Buffer d'inputs pour un client spécifique
 *
 * Gère la file d'attente des inputs avec délai configurable.
 * Permet de synchroniser les inputs de plusieurs joueurs en introduisant
 * un délai artificiel avant application.
 */
class ClientInputBuffer {
public:
    ClientInputBuffer() = default;

    /**
     * @brief Ajoute un input au buffer
     *
     * @param timestamp Timestamp client (millisecondes depuis début du jeu)
     * @param input_mask Masque des touches pressées
     * @return true si ajouté avec succès, false si buffer plein
     */
    bool add_input(uint32_t timestamp, uint8_t input_mask) {
        if (buffered_inputs_.size() >= InputDelayConfig::MAX_BUFFERED_INPUTS) {
            // Buffer plein, supprimer les plus anciens
            buffered_inputs_.pop_front();
        }

        buffered_inputs_.emplace_back(timestamp, input_mask);
        return true;
    }

    /**
     * @brief Récupère les inputs prêts à être appliqués
     *
     * @return Vector d'inputs ayant dépassé le délai configuré
     */
    std::vector<InputEntry> get_ready_inputs() {
        auto now = std::chrono::steady_clock::now();
        std::vector<InputEntry> ready;

        // Supprimer les inputs expirés
        while (!buffered_inputs_.empty() && buffered_inputs_.front().is_expired(now)) {
            buffered_inputs_.pop_front();
        }

        // Extraire les inputs prêts
        while (!buffered_inputs_.empty() && buffered_inputs_.front().is_ready_to_apply(now)) {
            ready.push_back(buffered_inputs_.front());
            buffered_inputs_.pop_front();
        }

        return ready;
    }

    /**
     * @brief Vide le buffer (utilisé à la déconnexion)
     */
    void clear() { buffered_inputs_.clear(); }

    /**
     * @brief Retourne le nombre d'inputs en attente
     */
    size_t size() const { return buffered_inputs_.size(); }

    /**
     * @brief Vérifie si le buffer est vide
     */
    bool empty() const { return buffered_inputs_.empty(); }

private:
    std::deque<InputEntry> buffered_inputs_;
};

}  // namespace server
