#include "WireGuardIntegration.h"
#include <fstream>
#include <sstream>
#include <Windows.h>
#include <shellapi.h>
#include <commdlg.h>
#include <filesystem>
#ifndef OFN_PATHMUSTEXIST
#define OFN_PATHMUSTEXIST 0x0008
#endif
#ifndef OFN_FILEMUSTEXIST
#define OFN_FILEMUSTEXIST 0x0001
#endif
using std::filesystem::path;
WireGuardIntegration::WireGuardIntegration() {
    isConnected = false;
    configLoaded = false;
}
WireGuardIntegration::~WireGuardIntegration() {
    if (isConnected) {
        Disconnect();
    }
}
bool WireGuardIntegration::LoadConfigFromString(const std::string& configContent) {
    try {
        std::istringstream stream(configContent);
        std::string line;
        std::string currentSection;
        while (std::getline(stream, line)) {
            if (line.empty()) continue;
            if (line[0] == '[' && line[line.length() - 1] == ']') {
                currentSection = line.substr(1, line.length() - 2);
                continue;
            }
            size_t delimPos = line.find('=');
            if (delimPos != std::string::npos) {
                std::string key = Trim(line.substr(0, delimPos));
                std::string value = Trim(line.substr(delimPos + 1));
                if (currentSection == "Interface") {
                    if (key == "PrivateKey") config.interfacePrivateKey = value;
                    else if (key == "Address") config.interfaceAddress = value;
                    else if (key == "DNS") config.dns = value;
                    else if (key == "MTU") config.mtu = std::stoi(value);
                }
                else if (currentSection == "Peer") {
                    if (key == "PublicKey") config.peerPublicKey = value;
                    else if (key == "PresharedKey") config.presharedKey = value;
                    else if (key == "Endpoint") config.endpoint = value;
                    else if (key == "AllowedIPs") config.allowedIPs = value;
                    else if (key == "PersistentKeepalive") config.persistentKeepalive = std::stoi(value);
                }
            }
        }
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
        file << "MTU = " << config.mtu << std::endl;
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
        file << "PersistentKeepalive = " << config.persistentKeepalive << std::endl;
        file.close();
        return true;
    }
    catch (const std::exception&) {
        return false;
    }
}
bool WireGuardIntegration::Connect() {
    if (!configLoaded || isConnected) {
        return false;
    }
    try {
        path tempPath = std::filesystem::temp_directory_path();
        std::string configPath = (tempPath / "freedom_vpn_temp.conf").string();
        if (!SaveConfigToFile(configPath)) {
            return false;
        }
        std::string wgPath = "C:\\Program Files\\WireGuard\\wireguard.exe";
        std::string command = "\"" + wgPath + "\" /installtunnelservice \"" + configPath + "\"";
        STARTUPINFOA si = { sizeof(STARTUPINFOA) };
        si.dwFlags = STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_HIDE;
        PROCESS_INFORMATION pi;
        bool processStarted = CreateProcessA(
            NULL,
            (LPSTR)command.c_str(),
            NULL,
            NULL,
            FALSE,
            CREATE_NO_WINDOW | CREATE_UNICODE_ENVIRONMENT,
            NULL,
            NULL,
            &si,
            &pi
        );
        if (!processStarted) {
            std::string altCommand = "wg-quick up \"" + configPath + "\"";
            processStarted = CreateProcessA(
                NULL,
                (LPSTR)altCommand.c_str(),
                NULL,
                NULL,
                FALSE,
                CREATE_NO_WINDOW,
                NULL,
                NULL,
                &si,
                &pi
            );
            if (!processStarted) {
                return false;
            }
        }
        DWORD waitResult = WaitForSingleObject(pi.hProcess, 30000);
        DWORD exitCode = 1;
        if (waitResult == WAIT_OBJECT_0) {
            GetExitCodeProcess(pi.hProcess, &exitCode);
        }
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        Sleep(2000);
        isConnected = true;
        return true;
    }
    catch (const std::exception&) {
        return false;
    }
}
bool WireGuardIntegration::Disconnect() {
    if (!isConnected) {
        return false;
    }
    try {
        path tempPath = std::filesystem::temp_directory_path();
        std::string configPath = (tempPath / "freedom_vpn_temp.conf").string();
        std::string tunnelName = "freedom_vpn_temp";
        std::string wgPath = "C:\\Program Files\\WireGuard\\wireguard.exe";
        std::string command = "\"" + wgPath + "\" /uninstalltunnelservice \"" + tunnelName + "\"";
        STARTUPINFOA si = { sizeof(STARTUPINFOA) };
        si.dwFlags = STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_HIDE;
        PROCESS_INFORMATION pi;
        bool processStarted = CreateProcessA(
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
        );
        if (!processStarted) {
            std::string altCommand = "wg-quick down \"" + configPath + "\"";
            processStarted = CreateProcessA(
                NULL,
                (LPSTR)altCommand.c_str(),
                NULL,
                NULL,
                FALSE,
                CREATE_NO_WINDOW,
                NULL,
                NULL,
                &si,
                &pi
            );
        }
        if (processStarted) {
            WaitForSingleObject(pi.hProcess, 10000);
            DWORD exitCode;
            GetExitCodeProcess(pi.hProcess, &exitCode);
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        }
        isConnected = false;
        return true;
    }
    catch (const std::exception&) {
        isConnected = false;
        return false;
    }
}
bool WireGuardIntegration::IsConnected() const {
    return isConnected;
}
std::string WireGuardIntegration::GetConnectionInfo() const {
    if (!isConnected || !configLoaded) {
        return "Not connected";
    }
    std::stringstream info;
    info << "Connected to: " << config.endpoint << std::endl;
    info << "IP Address: " << config.interfaceAddress << std::endl;
    info << "Allowed IPs: " << config.allowedIPs << std::endl;
    info << "MTU: " << config.mtu << std::endl;
    info << "PersistentKeepalive: " << config.persistentKeepalive << "s";
    return info.str();
}
std::string WireGuardIntegration::Trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t");
    if (first == std::string::npos) {
        return "";
    }
    size_t last = str.find_last_not_of(" \t");
    return str.substr(first, last - first + 1);
}
