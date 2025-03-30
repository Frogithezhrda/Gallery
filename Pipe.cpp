#include "Pipe.h"


Pipe::Pipe(std::queue<std::pair<CommandType, std::string>>& messages)
{
    newMessageFlag = false;
    //starting a thread
    std::thread pipeThread = std::thread(&Pipe::pipeInit, this, std::ref(messages));
    pipeThread.detach();
}

void Pipe::waitForNewMessage()
{
    //wating for message
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [this] { return newMessageFlag; });
    newMessageFlag = false; 
}

bool Pipe::sendMessage(std::string message)
{
    DWORD bytesWritten;
    if (_hPipe == INVALID_HANDLE_VALUE)
    {
        std::cerr << "Pipe handle is invalid Cannot send message\n";
        return false;
    }

    FlushFileBuffers(_hPipe);
    bool success = WriteFile(_hPipe, message.c_str(), message.size(), &bytesWritten, NULL);

    if (!success)
    {
        std::cerr << "Failed to send message. Error: " << GetLastError() << std::endl;
    }
    else
    {
        std::cout << "Sended Message" << std::endl;
    }
    return success;
}


Pipe::~Pipe()
{
}

void Pipe::pipeInit(std::queue<std::pair<CommandType, std::string>>& messages)
{
    char buffer[BUFFER_COUNT] = { 0 };
    DWORD bytesRead;
    DWORD bytesWritten;
    std::string send;
    //creating pipe that write and reads
    _hPipe = CreateNamedPipeA(
        pipeName, PIPE_ACCESS_DUPLEX,
        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
        1, BUFFER_COUNT, BUFFER_COUNT, 0, NULL
    );

    if (_hPipe != INVALID_HANDLE_VALUE)
    {
        //waiting
        std::cout << "Waiting for client...\n";
        ConnectNamedPipe(_hPipe, NULL);
        while (true)
        {
            //peeking if there is a message 
            if (PeekNamedPipe(_hPipe, NULL, 0, NULL, &bytesRead, NULL) && bytesRead > 0)
            {
                if (ReadFile(_hPipe, buffer, sizeof(buffer), &bytesRead, NULL))
                {
                    parseCommand(messages, std::string(buffer));
                    std::lock_guard<std::mutex> lock(mtx);
                    newMessageFlag = true;
                    cv.notify_one();
                    memset(buffer, 0, BUFFER_COUNT);
                }
            }
        }
    }
    else
    {
        throw MyException("Error In Creating Pipe...");
    }
    CloseHandle(_hPipe);
}

void Pipe::parseCommand(std::queue<std::pair<CommandType, std::string>>& messages, std::string message)
{
    int seperator = message.find_first_of(':');
    CommandType command;
    std::string data = "";
    std::string commandType = "";
    //parsing the message
    if (seperator == std::string::npos)
    {
        std::cerr << "Invalid command format: " << message << std::endl;
        return;
    }
    commandType = message.substr(0, seperator);
    data = message.substr(seperator + 1);
    command = static_cast<CommandType>(std::stoi(commandType));;

    messages.push({ command, data });
}
