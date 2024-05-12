#include <iostream>
#include <curl/curl.h>
#include <libconfig.h++>
#include <fstream>
#include <chrono>
#include <ctime>
#include <string>
#include <sstream>
#include <regex>
#include <vector>
#include <rconpp/rcon.h>
#include <future>
// probably i have more libraries than i need, but i im to lazy to check it

// g++ rcon.cpp -o rcon -lrconpp -lconfig++

// variables
std::string rconAddress;
int rconPort;
std::string rconPassword;
std::atomic<bool> connected{false};

// get the variables from a .config file
void get_config()
{
    // check if config file exists
    std::ifstream file("config.cfg");
    if (!file)
    {
        std::cerr << "Error: Unable to open config file or file dont exist\n";
        exit(1);
    }
    std::cout << "Reading config file" << std::endl;
    libconfig::Config cfg;
    cfg.readFile("config.cfg");
    rconAddress = cfg.lookup("rcon_address").c_str();
    rconPort = cfg.lookup("rcon_port");
    rconPassword = cfg.lookup("rcon_password").c_str();
    std::cout << "Done" << std::endl;
}

int main()
{
    get_config();

    std::cout << "Connecting to RCON with address: " << rconAddress << " port: " << rconPort << " password: " << rconPassword << std::endl;
    rconpp::rcon_client client(rconAddress, rconPort, rconPassword);

    client.on_log = [](const std::string_view &log)
    {
        std::cout << log << "\n";
    };

    client.start(true);
    // wait for the connection to be established even though it should be instant
    std::this_thread::sleep_for(std::chrono::seconds(3));

    bool isConnected = client.connected.load();
    // check if the connection is established
    std::cout << isConnected << std::endl;
    if (isConnected == false)
    {
        std::cout << "Failed to connect to RCON" << std::endl;
        exit(1);
    }

    // FUCK send_data, send_data_sync rules
    rconpp::response response;
    // send any commands you want
    response = client.send_data_sync("checkModsNeedUpdate", 3, rconpp::data_type::SERVERDATA_EXECCOMMAND, true);
    std::cout << "Response data: " << response.data << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(5));
    response = client.send_data_sync("servermsg putos_todos", 3, rconpp::data_type::SERVERDATA_EXECCOMMAND, true);
    std::cout << "Response data: " << response.data << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(5));
    response = client.send_data_sync("gunshot", 3, rconpp::data_type::SERVERDATA_EXECCOMMAND, true);
    std::cout << "Response data: " << response.data << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(5));
    response = client.send_data_sync("players", 3, rconpp::data_type::SERVERDATA_EXECCOMMAND, true);
    std::cout << "Response data: " << response.data << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(5));
    response = client.send_data_sync("help", 3, rconpp::data_type::SERVERDATA_EXECCOMMAND, true);
    std::cout << "Response data: " << response.data << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(5));
    return 0;
}
