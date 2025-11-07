#include "WireGuardIntegration.h"
#include <fstream>
#include <sstream>
#include <Windows.h>
#include <shellapi.h>
#include <commdlg.h>
#include <filesystem>

// Определения для диалога открытия файла, если они не определены
#ifndef OFN_PATHMUSTEXIST
#define OFN_PATHMUSTEXIST 0x0008
#endif

#ifndef OFN_FILEMUSTEXIST
#define OFN_FILEMUSTEXIST 0x0001
#endif

// Используем пространство имен std для filesystem
using std::filesystem::path;

// Конструктор
WireGuardIntegration::WireGuardIntegration() {
    isConnected = false;
    configLoaded = false;
}

// Деструктор
WireGuardIntegration::~WireGuardIntegration() {
    if (isConnected) {
        Disconnect();
    }
}

// Загрузка конфигурации из строки
bool WireGuardIntegration::LoadConfigFromString(const std::string& configContent) {
    try {
        std::istringstream stream(configContent);
        std::string line;
        std::string currentSection;

        while (std::getline(stream, line)) {
            // Пропускаем пустые строки
            if (line.empty()) continue;

            // Проверяем, является ли строка заголовком секции
            if (line[0] == '[' && line[line.length() - 1] == ']') {
                currentSection = line.substr(1, line.length() - 2);
                continue;
            }

            // Разбор ключ-значение
            size_t delimPos = line.find('=');
            if (delimPos != std::string::npos) {
                std::string key = Trim(line.substr(0, delimPos));
                std::string value = Trim(line.substr(delimPos + 1));

                if (currentSection == "Interface") {
                    if (key == "PrivateKey") config.interfacePrivateKey = value;
                    else if (key == "Address") config.interfaceAddress = value;
                    else if (key == "DNS") config.dns = value;
                }
                else if (currentSection == "Peer") {
                    if (key == "PublicKey") config.peerPublicKey = value;
                    else if (key == "PresharedKey") config.presharedKey = value;
                    else if (key == "Endpoint") config.endpoint = value;
                    else if (key == "AllowedIPs") config.allowedIPs = value;
                }
            }
        }

        // Проверка наличия необходимых полей
        if (config.interfacePrivateKey.empty() || config.peerPublicKey.empty() ||
            config.interfaceAddress.empty() || config.endpoint.empty()) {
            return false;
        }

        configLoaded = true;
        return true;
    }
    catch (const std::exception&) {
        return false;
    }
}

// Загрузка конфигурации из файла
bool WireGuardIntegration::LoadConfigFromFile(const std::string& filePath) {
    try {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            return false;
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        file.close();

        return LoadConfigFromString(buffer.str());
    }
    catch (const std::exception&) {
        return false;
    }
}

// Сохранение конфигурации в файл
bool WireGuardIntegration::SaveConfigToFile(const std::string& filePath) {
    if (!configLoaded) {
        return false;
    }

    try {
        std::ofstream file(filePath);
        if (!file.is_open()) {
            return false;
        }

        file << "[Interface]" << std::endl;
        file << "PrivateKey = " << config.interfacePrivateKey << std::endl;
        file << "Address = " << config.interfaceAddress << std::endl;
        if (!config.dns.empty()) {
            file << "DNS = " << config.dns << std::endl;
        }
        file << std::endl;

        file << "[Peer]" << std::endl;
        file << "PublicKey = " << config.peerPublicKey << std::endl;
        if (!config.presharedKey.empty()) {
            file << "PresharedKey = " << config.presharedKey << std::endl;
        }
        file << "Endpoint = " << config.endpoint << std::endl;
        file << "AllowedIPs = " << config.allowedIPs << std::endl;

        file.close();
        return true;
    }
    catch (const std::exception&) {
        return false;
    }
}

// Подключение к VPN
bool WireGuardIntegration::Connect() {
    if (!configLoaded || isConnected) {
        return false;
    }

    try {
        // Сохраним конфигурацию во временный файл
        path tempPath = std::filesystem::temp_directory_path();
        std::string configPath = (tempPath / "freedom_vpn_temp.conf").string();

        if (!SaveConfigToFile(configPath)) {
            return false;
        }

        // Запускаем WireGuard с нашей конфигурацией
        // Примечание: требуется установленный WireGuard
        std::string command = "wireguard.exe /installtunnelservice \"" + configPath + "\"";

        STARTUPINFOA si = { sizeof(STARTUPINFOA) };
        PROCESS_INFORMATION pi;

        if (!CreateProcessA(
            NULL,
            (LPSTR)command.c_str(),
            NULL,
            NULL,
            FALSE,
            CREATE_NO_WINDOW,
            NULL,
            NULL,
            &si,
            &pi
        )) {
            return false;
        }

        // Ждем завершения процесса
        WaitForSingleObject(pi.hProcess, INFINITE);

        DWORD exitCode;
        GetExitCodeProcess(pi.hProcess, &exitCode);

        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);

        if (exitCode != 0) {
            return false;
        }

        isConnected = true;
        return true;
    }
    catch (const std::exception&) {
        return false;
    }
}

// Отключение от VPN
bool WireGuardIntegration::Disconnect() {
    if (!isConnected) {
        return false;
    }

    try {
        // Получаем имя туннеля из имени файла конфигурации
        path tempPath = std::filesystem::temp_directory_path();
        std::string configPath = (tempPath / "freedom_vpn_temp.conf").string();

        std::string tunnelName = "freedom_vpn_temp";  // Имя по умолчанию

        // Выполняем команду удаления туннеля
        std::string command = "wireguard.exe /uninstalltunnelservice " + tunnelName;

        STARTUPINFOA si = { sizeof(STARTUPINFOA) };
        PROCESS_INFORMATION pi;

        if (!CreateProcessA(
            NULL,
            (LPSTR)command.c_str(),
            NULL,
            NULL,
            FALSE,
            CREATE_NO_WINDOW,
            NULL,
            NULL,
            &si,
            &pi
        )) {
            return false;
        }

        // Ждем завершения процесса
        WaitForSingleObject(pi.hProcess, INFINITE);

        DWORD exitCode;
        GetExitCodeProcess(pi.hProcess, &exitCode);

        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);

        isConnected = false;
        return true;
    }
    catch (const std::exception&) {
        return false;
    }
}

// Проверка статуса подключения
bool WireGuardIntegration::IsConnected() const {
    return isConnected;
}

// Получение информации о соединении
std::string WireGuardIntegration::GetConnectionInfo() const {
    if (!isConnected || !configLoaded) {
        return "Не подключено";
    }

    std::stringstream info;
    info << "Подключено к: " << config.endpoint << std::endl;
    info << "IP адрес: " << config.interfaceAddress << std::endl;
    info << "Разрешенные IP: " << config.allowedIPs;

    return info.str();
}

// Вспомогательная функция для удаления пробелов
std::string WireGuardIntegration::Trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t");
    if (first == std::string::npos) {
        return "";
    }
    size_t last = str.find_last_not_of(" \t");
    return str.substr(first, last - first + 1);
}