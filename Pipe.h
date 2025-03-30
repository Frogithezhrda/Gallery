#pragma once
#include <string>
#include <windows.h>
#include <iostream>
#include "MyException.h"
#include "Constants.h"
#include <queue>
#include <thread>
#include <condition_variable>

#define BUFFER_COUNT 1024

constexpr const char* pipeName = R"(\\.\pipe\OmerPipe)";

class Pipe
{
public:
	/*
	constructor getting messages
	@param messages(command type messages)
	*/
	Pipe(std::queue<std::pair<CommandType, std::string>>& messages);
	/*
	waiting for message to arrive and handlling it
	*/
	void waitForNewMessage();
	/*
	sending a message
	@param message
	@return sended
	*/
	bool sendMessage(std::string message);
	~Pipe();

private:
	HANDLE _hPipe;
	std::condition_variable cv;
	std::mutex mtx;
	bool newMessageFlag;
	/*
	init the pipe using message queue
	@param message queue
	*/
	void pipeInit(std::queue<std::pair<CommandType, std::string>>& messages);
	/*
	handling a command to the message list
	@param message queue
	@param message
	*/
	void parseCommand(std::queue<std::pair<CommandType, std::string>>& messages, std::string message);
};
