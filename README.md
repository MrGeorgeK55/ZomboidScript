## Script to check mods updates of your Project Zomboid server

## Basic explanation:  
- connects to RCON server with the credentials of config.cfg file  
- sends the command "CheckModsNeedUpdate" to generate the response in the log file  
- downloads the server-console.txt of your Zomboid server via FTP (most services or virtual machines you rent already have the FTP server installed and running)  
- filters and analizes the log file in the search of "CheckModsNeedUpdate: Mods need update"  
- if the exact text matches then alert the users in the server and notifies you via telegram
- it has a time grace of 5 minutes (one notification each minute to the users) before it restarts the server
- in case of the mods need update but no players are present it forces a restart without notifications (except via telegram)
- also if all the players disconnect after the first notification it skips the minutes remaining

This script is designed to be executed as a one-time operation, you can execute in any time interval (once each hour is recommended)  
also the approach and trouble of downloading the logfile and analizing its because the response of the command "CheckModsNeedUpdate" is "Checking started. The answer will be written in the log file and in the chat" but it never writes to a chat or responds with a "We already checked and we have updates".
Standalone examples are in the folder  
  
> [!IMPORTANT] 
> edit and rename the config file with your credentials

## Compilation: 

```sh
g++ zomboidScript.cpp -o zomboidScript --std=c++14 -I/usr/local/include -lTgBot -lboost_system -lssl -lcrypto -lpthread -lcurl -lrconpp -lconfig++
```

## Libraries used:


[RCON++](https://github.com/Jaskowicz1/rconpp)
[Telegram C++](https://github.com/reo7sp/tgbot-cpp/tree/master)
[LibConfig++](https://github.com/hyperrealm/libconfig)