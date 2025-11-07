#pragma once

#include <string>

// ��������� ��� �������� ������������ WireGuard
struct WireGuardConfig {
    std::string interfacePrivateKey;
    std::string interfaceAddress;
    std::string dns;
    int mtu = 1420;  // MTU для оптимизации (по умолчанию 1420 для игр)

    std::string peerPublicKey;
    std::string presharedKey;
    std::string endpoint;
    std::string allowedIPs;
    int persistentKeepalive = 25;  // Keep-alive для стабильности в играх (секунды)
};

// ����� ��� ���������� � WireGuard
class WireGuardIntegration {
public:
    WireGuardIntegration();
    ~WireGuardIntegration();

    // �������� ������������
    bool LoadConfigFromString(const std::string& configContent);
    bool LoadConfigFromFile(const std::string& filePath);
    bool SaveConfigToFile(const std::string& filePath);

    // ���������� ������������
    bool Connect();
    bool Disconnect();
    bool IsConnected() const;

    // ���������� � �����������
    std::string GetConnectionInfo() const;

private:
    WireGuardConfig config;
    bool isConnected;
    bool configLoaded;

    // ��������������� �������
    std::string Trim(const std::string& str);
};
