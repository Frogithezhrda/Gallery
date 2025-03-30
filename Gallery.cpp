#include <iostream>
#include <string>
#include "MemoryAccess.h"
#include "AlbumManager.h"
#include "DatabaseAccess.h"
#include "Pipe.h"

#define AUTHOR "Omer"
/*
printing the start
*/
void printStart();
/*
start program GUI
*/
void initGUI();

//windows main
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) 
{

	// initialization data access
	DatabaseAccess dataAccess;

	// initialize album manager
	AlbumManager albumManager(dataAccess);
	std::string albumName;
	//testMenu(); just tests
	printStart();
    std::thread gui(initGUI);
    gui.detach();
	albumManager.handleCommands();
	
}

void printStart()
{
	time_t now;
	time(&now);
	char* timeStr = ctime(&now); //getting the time
	std::cout << "Welcome to " << AUTHOR << "'s Gallery!" << std::endl;
	std::cout << "Date: " << timeStr;
	std::cout << "===================" << std::endl;
	std::cout << "Type " << HELP << " to a list of all supported commands" << std::endl;
}

void initGUI()
{

    std::string command = "java --module-path \"libs\" --add-modules javafx.controls,javafx.fxml,javafx.graphics -cp \"GalleryGUI.jar;libs/*\" -Djava.library.path=\"C:\\javafx-sdk\\bin\" -Dprism.order=sw -Dprism.verbose=true MainKt";

    //so the dont miss the pipe
    Sleep(100);
    STARTUPINFOA startInfo;
    PROCESS_INFORMATION processInfo;

    ZeroMemory(&startInfo, sizeof(startInfo));
    ZeroMemory(&processInfo, sizeof(processInfo));

    startInfo.cb = sizeof(startInfo); 
    startInfo.dwFlags = STARTF_USESHOWWINDOW;
    startInfo.wShowWindow = SW_HIDE;
    //starting a preccess
    if (!CreateProcessA(
        NULL,                        
        const_cast<char*>(command.c_str()), 
        NULL,                         
        NULL,                        
        FALSE,                        
        0,                           
        NULL,                         
        NULL,                         
        &startInfo,                   
        &processInfo                 
    )) {
        throw std::runtime_error("Couldn't start GUI process!");
    }

    WaitForSingleObject(processInfo.hProcess, INFINITE);

    //when ended we will exit
    CloseHandle(processInfo.hProcess);
    CloseHandle(processInfo.hThread);
    exit(0);
}
