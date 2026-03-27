/*
    Systemd proxy for linux.

    This is a simple tool that allows you to apply proxy configuration
    files to systemd services.

*/

#pragma once

#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <vector>

#include <SharedCppLib2/arguments.hpp>
#include <SharedCppLib2/platform.hpp>

#define default_no_proxy "localhost,127.0.0.1,0.0.0.0,10.0.0.0/8,192.168.0.0/16,172.16.0.0/12"

namespace fs = std::filesystem;

// 辅助函数：标准化服务名
std::string normalize_service_name(std::string name) {
    const std::string suffix = ".service";
    if (name.size() < suffix.size() || name.compare(name.size() - suffix.size(), suffix.size(), suffix) != 0) {
        return name + suffix;
    }
    return name;
}

// 获取配置目录路径
fs::path get_conf_dir(const std::string& service_name) {
    return fs::path("/etc/systemd/system") / (service_name + ".d");
}

fs::path get_conf_file(const std::string& service_name) {
    return get_conf_dir(service_name) / "proxy.conf";
}

int main(int argc, char** argv) {
    std::arguments args(argc, argv, std::arguments::EnablePrimaryCommand);
    std::string cmd = args.getPrimaryCommand();

    if (cmd == "up") {
        std::string service_raw;
        if (!args.hasSecondaryCommand()) {
            std::cerr << "Error: no service name specified." << std::endl;
            return 1;
        }
        service_raw = args.getSecondaryCommand();

        std::string service_name = normalize_service_name(service_raw);
        std::string proxy_url;
        
        // 尝试从 --proxy 获取，否则从当前环境变量获取
        if (!args.addParameter("proxy", proxy_url)) {
            proxy_url = platform::get_env("https_proxy");
            if (proxy_url.empty()) proxy_url = platform::get_env("http_proxy");
            if (proxy_url.empty()) proxy_url = platform::get_env("all_proxy");
        }

        if (proxy_url.empty()) {
            std::cerr << "Error: No proxy specified and no proxy environment variables found.\n";
            return 1;
        }

        std::string no_proxy = platform::get_env("no_proxy");
        if (no_proxy.empty()) no_proxy = default_no_proxy;

        // 创建目录和文件
        fs::create_directories(get_conf_dir(service_name));
        std::ofstream ofs(get_conf_file(service_name));
        if (ofs.is_open()) {
            ofs << "[Service]\n";
            ofs << "Environment=\"HTTP_PROXY=" << proxy_url << "\"\n";
            ofs << "Environment=\"HTTPS_PROXY=" << proxy_url << "\"\n";
            ofs << "Environment=\"ALL_PROXY=" << proxy_url << "\"\n";
            ofs << "Environment=\"NO_PROXY=" << no_proxy << "\"\n";
            ofs.close();
            
            std::cout << "Proxy configured for " << service_name << " -> " << proxy_url << "\n";
            std::cout << "Run 'systemctl daemon-reload && systemctl restart " << service_name << "' to apply.\n";
        }

    } else if (cmd == "down") {
        if (!args.hasSecondaryCommand()) {
            std::cerr << "Error: no service name specified." << std::endl;
            return 1;
        }
        std::string service_raw = args.getSecondaryCommand();

        std::string service_name = normalize_service_name(service_raw);
        fs::path conf_file = get_conf_file(service_name);

        if (fs::exists(conf_file)) {
            fs::remove(conf_file);
            std::cout << "Proxy configuration removed for " << service_name << ".\n";
            std::cout << "Run 'systemctl daemon-reload && systemctl restart " << service_name << "' to apply.\n";
        } else {
            std::cout << "No proxy configuration found for " << service_name << ".\n";
        }

    } else if (cmd == "status") {
        std::string service_raw;
        if (!args.hasSecondaryCommand()) {
            std::cerr << "Error: no service name specified." << std::endl;
            return 1;
        }
        service_raw = args.getSecondaryCommand();

        std::string service_name = normalize_service_name(service_raw);
        fs::path conf_file = get_conf_file(service_name);

        if (fs::exists(conf_file)) {
            std::cout << "Service [" << service_name << "] has active proxy override:\n";
            std::ifstream ifs(conf_file);
            std::string line;
            while (std::getline(ifs, line)) {
                std::cout << "  " << line << "\n";
            }
        } else {
            std::cout << "Service [" << service_name << "] has no proxy override.\n";
        }
    }

    return 0;
}