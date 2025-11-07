#pragma once

#include <string>

// Структура для хранения конфигурации WireGuard
struct WireGuardConfig {
    std::string interfacePrivateKey;
    std::string interfaceAddress;
    std::string dns;

    std::string peerPublicKey;
    std::string presharedKey;
    std::string endpoint;
    std::string allowedIPs;
};

// Класс для интеграции с WireGuard
class WireGuardIntegration {
public:
    WireGuardIntegration();
    ~WireGuardIntegration();

    // Загрузка конфигурации
    bool LoadConfigFromString(const std::string& configContent);
    bool LoadConfigFromFile(const std::string& filePath);
    bool SaveConfigToFile(const std::string& filePath);

    // Управление подключением
    bool Connect();
    bool Disconnect();
    bool IsConnected() const;

    // Информация о подключении
    std::string GetConnectionInfo() const;

private:
    WireGuardConfig config;
    bool isConnected;
    bool configLoaded;

    // Вспомогательные функции
    std::string Trim(const std::string& str);
};
