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
//probably i have more libraries than i need, but i im to lazy to check it

//to compile:
// g++ -o analizelog analizelog.cpp -lcurl -lconfig++

// variables
std::string wordsArray;
std::string localLogName;

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
    wordsArray = cfg.lookup("words_array").c_str();
    localLogName = cfg.lookup("local_log_file_name").c_str();
    std::cout << "Done" << std::endl;
}

// Function to parse the timestamp from the log line
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

bool analazingLogFile()
{
    std::cout << "Opening log file" << std::endl;
    std::ifstream logFile(localLogName);
    if (!logFile)
    {
        std::cerr << "Error: Unable to open logfile.txt\n";
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

int main()
{
    // Read the log file
    get_config();
    if (analazingLogFile())
    {
        std::cout << "Match found" << std::endl;
    }
    else
    {
        std::cout << "No match found" << std::endl;
    }

    return 0;
}
