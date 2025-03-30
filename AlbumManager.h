#pragma once
#include <vector>
#include <windows.h>
#include <algorithm>
#include "Constants.h"
#include "MemoryAccess.h"
#include "Album.h"
#include <iostream>
#include "Constants.h"
#include "MyException.h"
#include "AlbumNotOpenException.h"
#include <csignal>
#include <atomic>
#include <thread>
#include <queue>
#include "Pipe.h"

#define MSPAINT "1"
#define IFRANVIEW "2"


/*
i have not removed the original without the gui this is because
i may wanna use it in the future 
or just keep it for console app
*/
class AlbumManager
{
public:
	AlbumManager(IDataAccess& dataAccess);

	virtual ~AlbumManager();
	/*
	executing the command that it get
	@param command number(type)
	*/
	void executeCommand(CommandType command, const std::string& params);
	//printing the commands
	void printHelp() const;
	
	void handleCommands();
	using handler_func_t = void (AlbumManager::*)(void);    

private:
    int m_nextPictureId{};
    int m_nextUserId{};
    std::string m_currentAlbumName{};
	int m_currentUserId;
	IDataAccess& m_dataAccess;
	Album m_openAlbum;
	std::queue<std::pair<CommandType, std::string>> _messages;
	std::queue<std::string> _params;
	Pipe* _pipe;

	//commands created doc in the databaseaccess
	void help();
	// albums management
	void createAlbum();
	void openAlbum();
	void closeAlbum();
	void deleteAlbum();
	void closeUser();
	void listUsers();

	// Picture management
	void addPictureToAlbum();
	void removePictureFromAlbum();
	void listPicturesInAlbum();
	void showPicture();

	// tags related
	void tagUserInPicture();
	void untagUserInPicture();
	void listUserTags();

	// users management
	void addUser();
	void removeUser();
	void openUser();
	void userStatistics();

	void topTaggedUser();
	void topTaggedPicture();
	void picturesTaggedUser();

	void setReadonly();
	void unsetReadonly();
	void exit();
	/*
	getting the input from the console
	@param message
	@return output from console
	*/
	std::string getInputFromConsole(const std::string& message);
	/*
	checking if file in disk
	@param filename
	@return is In disk
	@return is Not In Disk
	*/
	bool fileExistsOnDisk(const std::string& filename);
	//refreshing the album checking if its still the same
	void refreshOpenAlbum();
	/*
	checking if the album name is not null
	@return true
	@return false
	*/
    bool isCurrentAlbumSet() const;
	/*
	getting the full path of a file
	@param relative path
	@return full path
	*/
	std::string getFullPath(const std::string& relativePath) const;
	/*
	handling the signal when getting will set the isPressed to true
	@param signal
	*/
	static void signalHandle(const int signal);
	/*
	thread lisening to the ctrl c(the signal) if is pressed is true
	will terminate the process of the picture
	@param process info
	*/
	void ctrlCListener(PROCESS_INFORMATION& processInfo);
	/*
	getting the paramaters
	@return command id
	@return command
	*/
	std::pair<int, std::string> getParms();
	/*
	getting the command message
	@return commandType
	@return string of command
	*/
	std::pair<CommandType, std::string> getCommandType();

	static std::atomic<bool> _isPressed;
	static const std::vector<struct CommandGroup> m_prompts;
	static const std::map<CommandType, handler_func_t> m_commands;

};

