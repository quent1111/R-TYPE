#include <iostream>
#include <string>
#include <vector>
#include <cstdint>
#include <cstring>
#include <thread>
#include <atomic>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#include <errno.h>
#include "server/src/inputKey.hpp"

class SimpleUDPClient {
private:
    int socket_fd_;
    struct sockaddr_in server_addr_;
    static constexpr uint16_t MAGIC_NUMBER = 0xB542;
    std::atomic<bool> running_;
    std::thread receiver_thread_;

public:
    SimpleUDPClient(const std::string& host, unsigned short port) : running_(true) {
        socket_fd_ = socket(AF_INET, SOCK_DGRAM, 0);
        if (socket_fd_ < 0) {
            throw std::runtime_error("Impossible de créer le socket");
        }
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 500000;
        setsockopt(socket_fd_, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        struct hostent* server = gethostbyname(host.c_str());
        if (server == nullptr) {
            close(socket_fd_);
            throw std::runtime_error("Hôte introuvable: " + host);
        }
        std::memset(&server_addr_, 0, sizeof(server_addr_));
        server_addr_.sin_family = AF_INET;
        std::memcpy(&server_addr_.sin_addr.s_addr, server->h_addr, server->h_length);
        server_addr_.sin_port = htons(port);
        std::cout << "[Client] Connecté au serveur " << host << ":" << port << std::endl;
        receiver_thread_ = std::thread(&SimpleUDPClient::receive_loop, this);
    }

    void receive_loop() {
        std::vector<uint8_t> buffer(4096);
        struct sockaddr_in from_addr;
        socklen_t from_len = sizeof(from_addr);

        while (running_) {
            ssize_t received = recvfrom(socket_fd_, buffer.data(), buffer.size(), 0,
                                       (struct sockaddr*)&from_addr, &from_len);

            if (received > 0) {
                if (received >= 2) {
                    uint16_t magic = static_cast<uint16_t>(buffer[0]) |
                                    (static_cast<uint16_t>(buffer[1]) << 8);

                    if (magic == MAGIC_NUMBER) {
                        if (received >= 3) {
                            uint8_t msg_type = buffer[2];

                            if (msg_type == 0x13) {
                                decode_player_positions(buffer, received);
                            } else if (msg_type == 0x01) {
                                std::cout << "\n[Serveur] Message de connexion\n> " << std::flush;
                            } else {
                                std::cout << "\n[Serveur] ";
                                for (ssize_t i = 2; i < received; i++) {
                                    std::cout << static_cast<char>(buffer[i]);
                                }
                                std::cout << "\n> " << std::flush;
                            }
                        } else {
                            std::cout << "\n[Serveur] Message vide\n> " << std::flush;
                        }
                    } else {
                        std::cout << "\n[Serveur] Données brutes (hex): ";
                        for (ssize_t i = 0; i < received; i++) {
                            printf("%02X ", buffer[i]);
                        }
                        std::cout << "\n> " << std::flush;
                    }
                }
            } else if (received < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
                if (running_) {
                    std::cerr << "\n[Erreur] Réception: " << strerror(errno) << std::endl;
                }
            }
        }
    }

    void decode_player_positions(const std::vector<uint8_t>& buffer, ssize_t received) {
        std::cout << "\n[Game] Positions des joueurs:\n";

        if (received < 4) {
            std::cout << "  Paquet invalide (trop court)\n> " << std::flush;
            return;
        }

        size_t offset = 3;
        uint8_t player_count = buffer[offset];
        offset += 1;
        std::cout << "  Nombre de joueurs: " << static_cast<int>(player_count) << "\n";

        for (int i = 0; i < player_count && offset + 12 <= static_cast<size_t>(received); i++) {
            uint32_t client_id = static_cast<uint32_t>(buffer[offset]) |
                                (static_cast<uint32_t>(buffer[offset + 1]) << 8) |
                                (static_cast<uint32_t>(buffer[offset + 2]) << 16) |
                                (static_cast<uint32_t>(buffer[offset + 3]) << 24);
            offset += 4;

            float x;
            std::memcpy(&x, &buffer[offset], sizeof(float));
            offset += 4;

            float y;
            std::memcpy(&y, &buffer[offset], sizeof(float));
            offset += 4;

            std::cout << "  Player " << client_id << ": (" << x << ", " << y << ")\n";
        }

        if (player_count == 0) {
            std::cout << "  Aucun joueur connecté\n";
        }

        std::cout << "> " << std::flush;
    }

    void send_login_request() {
        std::vector<uint8_t> packet;

        packet.push_back(static_cast<uint8_t>(MAGIC_NUMBER & 0xFF));
        packet.push_back(static_cast<uint8_t>((MAGIC_NUMBER >> 8) & 0xFF));
        packet.push_back(0x01);
        ssize_t sent = sendto(socket_fd_, packet.data(), packet.size(), 0,
                             (struct sockaddr*)&server_addr_, sizeof(server_addr_));
        if (sent < 0) {
            std::cerr << "[Erreur] Échec de l'envoi de connexion: " << strerror(errno) << std::endl;
        } else {
            std::cout << "[Client] Demande de connexion envoyée" << std::endl;
        }
    }
    void send_input(char key) {
        std::vector<uint8_t> packet;
        packet.push_back(static_cast<uint8_t>(MAGIC_NUMBER & 0xFF));
        packet.push_back(static_cast<uint8_t>((MAGIC_NUMBER >> 8) & 0xFF));
        packet.push_back(0x10);
        switch (key)
        {
        case 'z':
            packet.push_back(KEY_Z);
            break;
        case 's':
            packet.push_back(KEY_S);
            break;
        case 'q':
            packet.push_back(KEY_Q);
            break;
        case 'd':
            packet.push_back(KEY_D);
            break;
        default:
            packet.push_back(static_cast<uint8_t>(key));
            break;
        }
        packet.push_back(0);
        packet.push_back(0);
        packet.push_back(0);
        packet.push_back(0);
        ssize_t sent = sendto(socket_fd_, packet.data(), packet.size(), 0,
                             (struct sockaddr*)&server_addr_, sizeof(server_addr_));
        if (sent < 0) {
            std::cerr << "[Erreur] Échec de l'envoi d'input: " << strerror(errno) << std::endl;
        } else {
            std::cout << "[Client] Input envoyé: '" << key << "'" << std::endl;
        }
    }

    void send_message(const std::string& message) {
        std::vector<uint8_t> packet;
        packet.push_back(static_cast<uint8_t>(MAGIC_NUMBER & 0xFF));
        packet.push_back(static_cast<uint8_t>((MAGIC_NUMBER >> 8) & 0xFF));
        for (char c : message) {
            packet.push_back(static_cast<uint8_t>(c));
        }
        ssize_t sent = sendto(socket_fd_, packet.data(), packet.size(), 0,
                             (struct sockaddr*)&server_addr_, sizeof(server_addr_));
        if (sent < 0) {
            std::cerr << "[Erreur] Échec de l'envoi: " << strerror(errno) << std::endl;
        } else {
            std::cout << "[Client] Message envoyé (" << packet.size() << " octets)" << std::endl;
        }
    }

    void run() {
        std::cout << "=== Client UDP R-TYPE ===" << std::endl;
        std::cout << "Commandes disponibles:" << std::endl;
        std::cout << "  'connect' - Se connecter au serveur" << std::endl;
        std::cout << "  'z/q/s/d' - Déplacements (up/left/down/right)" << std::endl;
        std::cout << "  'space'   - Tirer" << std::endl;
        std::cout << "  'quit'    - Quitter" << std::endl;
        std::cout << "=========================" << std::endl;

        std::string input;
        while (true) {
            std::cout << "> ";
            std::getline(std::cin, input);

            if (input == "quit" || input == "exit") {
                std::cout << "[Client] Déconnexion..." << std::endl;
                break;
            }

            if (input == "connect") {
                send_login_request();
            }
            else if (input == "z") {
                send_input('z');
            }
            else if (input == "s") {
                send_input('s');
            }
            else if (input == "q") {
                send_input('q');
            }
            else if (input == "d") {
                send_input('d');
            }
            else if (input == "space") {
                send_input(' ');
            }
            else if (!input.empty()) {
                send_message(input);
            }
        }
    }

    ~SimpleUDPClient() {
        running_ = false;
        if (receiver_thread_.joinable()) {
            receiver_thread_.join();
        }
        if (socket_fd_ >= 0) {
            close(socket_fd_);
        }
    }
};

int main(int argc, char* argv[]) {
    std::string host = "localhost";
    unsigned short port = 4242;

    if (argc >= 2) {
        host = argv[1];
    }
    if (argc >= 3) {
        try {
            port = static_cast<unsigned short>(std::stoi(argv[2]));
        } catch (...) {
            std::cerr << "[Erreur] Port invalide, utilisation du port par défaut 4242" << std::endl;
        }
    }
    std::cout << "[Client] Connexion à " << host << ":" << port << std::endl;
    try {
        SimpleUDPClient client(host, port);
        client.run();
    } catch (const std::exception& e) {
        std::cerr << "[Erreur Fatale] " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
