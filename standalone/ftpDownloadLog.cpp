#include <iostream>
#include <curl/curl.h>
#include <libconfig.h++>
#include <fstream>
//probably i have more libraries than i need, but i im too lazy to check it

// g++ ftpDownloadLog.cpp -o ftpDownloadLog -lcurl -lconfig++

// variables
std::string ftpUsername;
std::string ftpPassword;
std::string ftpAddress;
std::string localLogName;
int ftpPort;

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
    ftpUsername = cfg.lookup("ftp_username").c_str();
    ftpPassword = cfg.lookup("ftp_password").c_str();
    ftpAddress = cfg.lookup("ftp_addressnfile").c_str();
    ftpPort = cfg.lookup("ftp_port");
    localLogName = cfg.lookup("local_log_file_name").c_str();
    std::cout << "Config file read" << std::endl;
}

size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t totalSize = size * nmemb;
    std::ofstream *file = static_cast<std::ofstream *>(userp);
    file->write(static_cast<char *>(contents), totalSize);
    return totalSize;
}

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

int main()
{
    get_config();
    get_log();

    // print the variables
    std::cout << ftpUsername << std::endl;
    std::cout << ftpPassword << std::endl;
    std::cout << ftpAddress << std::endl;
    std::cout << ftpPort << std::endl;

    return 0;
}