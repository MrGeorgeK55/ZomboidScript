#include <iostream>
#include <curl/curl.h>
#include <libconfig.h++>
#include <fstream>
#include <string>
#include <chrono>
#include <ctime>
#include <string>
#include <sstream>
#include <regex>
#include <vector>
#include <tgbot/tgbot.h>

// variables
// FTP
std::string ftpUsername;
std::string ftpPassword;
std::string ftpAddress;
int ftpPort;
// rcon
std::string rconAddress;
int rconPort;
std::string rconPassword;
std::atomic<bool> connected{false};
// telegram
std::string telegramToken;
int telegramChatId;
// words
std::string wordsArray;
// file name
std::string logNameOnServer;
// file downloaded
std::string logNameDownloaded;


// get the variables from a .config file
void get_config()
{
    //check if config file exists
    std::ifstream file("config.cfg");
    if (!file)
    {
        std::cerr << "Error: Unable to open config file or file dont exist\n";
        exit(1);
    }

    std::cout << "Reading config file" << std::endl;
    libconfig::Config cfg;
    cfg.readFile("config.cfg");
    // FTP
    ftpUsername = cfg.lookup("ftp_username").c_str();
    ftpPassword = cfg.lookup("ftp_password").c_str();
    ftpAddress = cfg.lookup("ftp_addressnfile").c_str();
    ftpPort = cfg.lookup("ftp_port");
    // rcon
    rconAddress = cfg.lookup("rcon_address").c_str();
    rconPort = cfg.lookup("rcon_port");
    rconPassword = cfg.lookup("rcon_password").c_str();
    // telegram
    telegramToken = cfg.lookup("telegram_token").c_str();
    telegramChatId = cfg.lookup("telegram_chat_id");
    // words
    wordsArray = cfg.lookup("words_array").c_str();
    // file name
    localLogName = cfg.lookup("local_log_file_name").c_str();

    std::cout << "Config file read" << std::endl;
}

//no idea what this does
size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t totalSize = size * nmemb;
    std::ofstream *file = static_cast<std::ofstream *>(userp);
    file->write(static_cast<char *>(contents), totalSize);
    return totalSize;
}

// download log file
void get_log()
{
    std::cout << "Downloading log file" << std::endl;
    CURL *curl;
    CURLcode res;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl)
    {
        // FTP username and password
        curl_easy_setopt(curl, CURLOPT_USERNAME, ftpUsername.c_str());
        curl_easy_setopt(curl, CURLOPT_PASSWORD, ftpPassword.c_str());

        std::string outputFilePath = localLogName;

        std::ofstream outputFile(outputFilePath, std::ios::binary);
        if (!outputFile)
        {
            std::cerr << "Failed to open output file." << std::endl;
            exit(1);
        }
        // FTP address
        curl_easy_setopt(curl, CURLOPT_URL, ftpAddress.c_str());
        // FTP port
        curl_easy_setopt(curl, CURLOPT_PORT, ftpPort);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &outputFile);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK)
        {
            std::cerr << "Failed to download file: " << curl_easy_strerror(res) << std::endl;
            exit(1);
        }

        curl_easy_cleanup(curl);
        outputFile.close();
    }

    curl_global_cleanup();
    std::cout << "Log file downloaded" << std::endl;
}

//log functions

// Function to parse the timestamp from a log line
long long parseTimestamp(const std::string &logLine)
{
    size_t pos = logLine.find('>');
    if (pos != std::string::npos)
    {
        std::string timestampStr = logLine.substr(0, pos);
        timestampStr = timestampStr.substr(timestampStr.find_last_of(' ') + 1);
        return std::stoll(timestampStr);
    }
    throw std::invalid_argument("Invalid log line format: " + logLine);
}

// Function to check if a log line contains the specified text after the prefix
bool containsText(const std::string &logLine, const std::string &searchText)
{
    size_t pos = logLine.find("LOG  : General");
    if (pos != std::string::npos)
    {
        pos = logLine.find(">", pos);
        if (pos != std::string::npos)
        {
            pos = logLine.find(">", pos + 1);
            if (pos != std::string::npos)
            {
                std::string message = logLine.substr(pos + 1);
                return message.find(searchText) != std::string::npos;
            }
        }
    }
    return false;
}

// analize log file to check if the specified text is present
bool analazingLogFile()
{
    std::cout << "Opening log file" << std::endl;
    std::ifstream logFile(localLogName);
    if (!logFile)
    {
        std::cerr << "Error: Unable to open log file\n";
        return 1;
    }
    std::cout << "Done" << std::endl;
    std::cout << "Getting current time" << std::endl;
    // Get current time
    auto currentTime = std::chrono::system_clock::now();
    auto currentTimeStamp = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime.time_since_epoch()).count();

    std::cout << "Current time: " << currentTimeStamp << std::endl;
    // Vector to store log lines from 5 minutes ago
    std::vector<std::string> logLinesFiveMinutesAgo;

    std::cout << "Filtering log file" << std::endl;
    std::string line;
    while (std::getline(logFile, line))
    {
        if (line.find("LOG  : General") == 0)
        { // Check if the line starts with "LOG  : General"
            long long timestamp = parseTimestamp(line);
            // Check if the timestamp is within the last 5 minutes
            if (currentTimeStamp - timestamp <= 5 * 60 * 1000)
            {
                logLinesFiveMinutesAgo.push_back(line);
            }
        }
    }
    // Define the text to match
    std::string searchText = wordsArray;
    std::cout << "Searching for text: " << searchText << std::endl;
    // Compare the last lines from 5 minutes ago with the specified text
    bool foundMatch = false;

    // Compare the last lines from 5 minutes ago with the specified text
    for (const auto &logLine : logLinesFiveMinutesAgo)
    {
        if (containsText(logLine, searchText))
        {
            std::cout << "Match found: " << logLine << std::endl;
            foundMatch = true;
            // If you need to take some action here, you can do it.
        }
    }

    if (foundMatch)
    {
        // Trigger the action if at least one match was found
        std::cout << "At least one match was found." << std::endl;
        return true;
    }
    else
    {
        // Do nothing if no matches were found
        std::cout << "No matches were found." << std::endl;
        return false;
    }
}

// look number of players connected
int getNumberOfPlayers(const std::string& response) {
    size_t pos = response.find("Players connected (");
    if (pos == std::string::npos) {
        // No player information found in the response
        return 0;
    }

    pos += 19; // Move past "Players connected ("

    // Find the closing parenthesis
    size_t endPos = response.find(')', pos);
    if (endPos == std::string::npos) {
        // Invalid response format
        return 0;
    }

    // Extract the number of players as a substring
    std::string numPlayersStr = response.substr(pos, endPos - pos);

    // Convert the substring to an integer
    int numPlayers = std::stoi(numPlayersStr);

    return numPlayers;
}

int main()
{
    // get variables from config file
    get_config();

    // attempt to connect to RCON
    std::cout << "Connecting to RCON with address: " << rconAddress << " port: " << rconPort << " password: " << rconPassword << std::endl;
    rconpp::rcon_client client(rconAddress, rconPort, rconPassword);

    client.on_log = [](const std::string_view &log)
    {
        std::cout << log << "\n";
    };

    client.start(true);
    bool isConnected = connected.load();
    if (isConnected == false)
    {
        std::cout << "Failed to connect to RCON server" << std::endl;
        exit(1);
    }
    rconpp::response response;

    // i hate send_data, send_data_sync rules
    // send command to check if mods need update
    //this generates the log file in the server 
    response = client.send_data_sync("checkModsNeedUpdate", 3, rconpp::data_type::SERVERDATA_EXECCOMMAND, true);
    std::cout << "Response data: " << response.data << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(5));
    
    // get the log file
    get_log();
    
    // check if the log file contains the specified text
    if (analazingLogFile())
    {
        std::cout << "Mods Need update" << std::endl;
        std::cout << "Sendig alerts to connected users and George via telegram" << std::endl;
        //telegram notify
        bot.getApi().sendMessage(chat_id, "Estamos reiniciando el server");
        //check if they are any players connected
        response = client.send_data_sync("players", 3, rconpp::data_type::SERVERDATA_EXECCOMMAND, true);
        std::cout << "Response data: " << response.data << std::endl;
        int numPlayers = getNumberOfPlayers(response);
        if (numPlayers = 0)
        {
            std::cout << "No players connected" << std::endl;
            bot.getApi().sendMessage(chat_id, "No hay jugadores conectados, reiniciando a la fuerza");
            response = client.send_data_sync("save", 3, rconpp::data_type::SERVERDATA_EXECCOMMAND, true);
            std::cout << "Response data: " << response.data << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(5));
            response = client.send_data_sync("quit", 3, rconpp::data_type::SERVERDATA_EXECCOMMAND, true);
            std::cout << "Response data: " << response.data << std::endl;
            bot.getApi().sendMessage(chat_id, "Server reiniciado");
            exit(1);
        }
        response = client.send_data_sync("servermsg Los_Mods_necesitan_update!__Reinicio_Inminente__", 3, rconpp::data_type::SERVERDATA_EXECCOMMAND, true);
        std::cout << "Response data: " << response.data << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(10));
        response = client.send_data_sync("servermsg Reiniciando_server_en_5_min", 3, rconpp::data_type::SERVERDATA_EXECCOMMAND, true);
        std::cout << "Response data: " << response.data << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(60));
        response = client.send_data_sync("servermsg Reiniciando_server_en_4_min", 3, rconpp::data_type::SERVERDATA_EXECCOMMAND, true);
        std::cout << "Response data: " << response.data << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(60));

        response = client.send_data_sync("players", 3, rconpp::data_type::SERVERDATA_EXECCOMMAND, true);
        std::cout << "Response data: " << response.data << std::endl;
        numPlayers = getNumberOfPlayers(response);
        if (numPlayers = 0)
        {
            std::cout << "Players already disconnected" << std::endl;
            response = client.send_data_sync("save", 3, rconpp::data_type::SERVERDATA_EXECCOMMAND, true);
            std::cout << "Response data: " << response.data << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(5));
            response = client.send_data_sync("quit", 3, rconpp::data_type::SERVERDATA_EXECCOMMAND, true);
            std::cout << "Response data: " << response.data << std::endl;
            bot.getApi().sendMessage(chat_id, "Server reiniciado");
            exit(1);
        }
        response = client.send_data_sync("servermsg Reiniciando_server_en_3_min", 3, rconpp::data_type::SERVERDATA_EXECCOMMAND, true);
        std::cout << "Response data: " << response.data << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(60));
        response = client.send_data_sync("servermsg Reiniciando_server_en_2_min", 3, rconpp::data_type::SERVERDATA_EXECCOMMAND, true);
        std::cout << "Response data: " << response.data << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(60));
        response = client.send_data_sync("servermsg Reiniciando_server_en_1_min", 3, rconpp::data_type::SERVERDATA_EXECCOMMAND, true);
        std::cout << "Response data: " << response.data << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(60));
        response = client.send_data_sync("servermsg Reiniciando_server_en_30_seg", 3, rconpp::data_type::SERVERDATA_EXECCOMMAND, true);
        std::cout << "Response data: " << response.data << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(30));
        response = client.send_data_sync("save", 3, rconpp::data_type::SERVERDATA_EXECCOMMAND, true);
        std::cout << "Response data: " << response.data << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(5));
        bot.getApi().sendMessage(chat_id, "Server reiniciado");    
    
    }
    else
    {
        std::cout << "No match found entering cooldown" << std::endl;
    }

    return 0;
}
