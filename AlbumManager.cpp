#include "AlbumManager.h"

constexpr const char* MSPAINT_PATH = R"(C:/Program Files/WindowsApps/Microsoft.Paint_11.2412.311.0_x64__8wekyb3d8bbwe/PaintApp/mspaint.exe)";
constexpr const char* IRFANVIEW_PATH = "C:\\Program Files\\IrfanView\\i_view64.exe";
//using atomic for safety
std::atomic<bool> AlbumManager::_isPressed(false);

AlbumManager::AlbumManager(IDataAccess& dataAccess) :
	m_dataAccess(dataAccess)
{	
	_pipe = new Pipe(this->_messages);
	
	// Left empty
	m_dataAccess.open();
	//generating the id from the start to work on
	m_nextPictureId = m_dataAccess.generateNewId(PICTURES_COLUNM);
	m_nextUserId = m_dataAccess.generateNewId(USERS_COLUNM);
}

AlbumManager::~AlbumManager()
{
	delete _pipe;
}

void AlbumManager::executeCommand(CommandType command, const std::string& params) 
{
	try 
	{
		if(params != "None") this->_params.push(params);
		AlbumManager::handler_func_t handler = m_commands.at(command);
		(this->*handler)();
	} catch (const std::out_of_range&)
	{
		throw MyException("Error: Invalid command[" + std::to_string(command) + "]\n");
	}
}

void AlbumManager::handleCommands()
{
	while (true)
	{
		try
		{
			_pipe->waitForNewMessage();
			std::pair<CommandType, std::string> message = getCommandType();
			executeCommand(message.first, message.second);
		}
		catch (const std::exception& exception)
		{
			std::cout << exception.what() << std::endl;
		}
	}
}

void AlbumManager::printHelp() const
{
	std::cout << "Supported Album commands:" << std::endl;
	std::cout << "*************************" << std::endl;
	
	for (const struct CommandGroup& group : m_prompts) {
		std::cout << group.title << std::endl;
		std::string space(".  ");
		for (const struct CommandPrompt& command : group.commands) {
			space = command.type < 10 ? ".   " : ".  ";

			std::cout << command.type << space << command.prompt << std::endl;
		}
		std::cout << std::endl;
	}
}


// ******************* Album ******************* 
void AlbumManager::createAlbum()
{
	std::pair<int, std::string> parms = getParms();// getInputFromConsole("Enter user id: ");
	if ( !m_dataAccess.doesUserExists(parms.first) ) 
	{
		throw MyException("Error: Can't create album since there is no user with id [" + std::to_string(parms.first) + "]\n");
	}

	if (m_dataAccess.doesAlbumExists(parms.second, parms.first) )
	{
		throw MyException("Error: Failed to create album, album with the same name already exists\n");
	}

	Album newAlbum(parms.first, parms.second);
	m_dataAccess.createAlbum(newAlbum);
	std::cout << "Album [" << newAlbum.getName() << "] created successfully by user@" << newAlbum.getOwnerId() << std::endl;
	//pipe reseting command
	_pipe->sendMessage("11");
	//sending albums
	for (auto album : m_dataAccess.getAlbumsOfUser(m_dataAccess.getUser(parms.first)))
	{
		_pipe->sendMessage("2," + std::to_string(parms.first) + "," + album.getName());
	}
}

void AlbumManager::openAlbum()
{
	if (isCurrentAlbumSet()) 
	{
		closeAlbum();
	}
	std::pair<int, std::string> parms = getParms();
	if ( !m_dataAccess.doesUserExists(parms.first) ) {
		throw MyException("Error: Can't open album since there is no user with id @" + std::to_string(parms.first) + ".\n");
	}

	if (!m_dataAccess.doesAlbumExists(parms.second, parms.first) ) 
	{
		throw MyException("Error: Failed to open album, since there is no album with name:"+ parms.second +".\n");
	}

	m_openAlbum = m_dataAccess.openAlbum(parms.second, parms.first);
    m_currentAlbumName = parms.second;
	m_currentUserId = parms.first;
	//sending pictures and tags
	for (Picture& picture : m_openAlbum.getPictures())
	{
		_pipe->sendMessage("5," + parms.second + "," + std::to_string(parms.first) + "," + getFullPath(picture.getPath()) + "," + picture.getName());
		const std::set<int>& userTags = picture.getUserTags();
		for (const auto& user : userTags)
		{
			_pipe->sendMessage("12," + parms.second + "," + std::to_string(parms.first) + "," + std::to_string(user) + "," + picture.getName());
		}
	}
	// success
	std::cout << "Album [" << parms.second << "] opened successfully." << std::endl;
}

void AlbumManager::closeAlbum()
{
	refreshOpenAlbum();
	std::cout << "Album [" << m_openAlbum.getName() << "] closed successfully." << std::endl;
	m_dataAccess.closeAlbum(m_openAlbum);
	m_currentAlbumName = "";
	m_currentUserId = 0;
	_pipe->sendMessage("3");
}

void AlbumManager::deleteAlbum()
{

	std::pair<int, std::string> parms = getParms();
	if (!m_dataAccess.doesUserExists(parms.first))
	{
		throw MyException("Error: There is no user with id @" + std::to_string(parms.first) + "\n");
	}

	if ( !m_dataAccess.doesAlbumExists(parms.second, parms.first) ) 
	{
		throw MyException("Error: Failed to delete album, since there is no album with name:" + parms.second + ".\n");
	}

	// album exist, close album if it is opened
	if ( (isCurrentAlbumSet() ) &&
		 (m_openAlbum.getOwnerId() == parms.first && m_openAlbum.getName() == parms.second) ) {

		closeAlbum();
	}

	m_dataAccess.deleteAlbum(parms.second, parms.first);
	std::cout << "Album [" << parms.second << "] @"<< parms.first <<" deleted successfully." << std::endl;
	//reseting and sending
	_pipe->sendMessage("11");
	for (auto album : m_dataAccess.getAlbumsOfUser(m_dataAccess.getUser(parms.first)))
	{
		_pipe->sendMessage("2," + std::to_string(parms.first) + "," + album.getName());
	}
}

void AlbumManager::closeUser()
{
	//close user command
	_pipe->sendMessage("9");
	closeAlbum();
}

void AlbumManager::listUsers()
{
	//listing the user
	m_dataAccess.printUsers(_pipe);
	_pipe->sendMessage("6," + getFullPath(m_dataAccess.getTopTaggedPicture().getPath()) + "," + m_dataAccess.getTopTaggedUser().getName() + "," + m_dataAccess.getTopTaggedPicture().getName());
}


// ******************* Picture ******************* 
void AlbumManager::addPictureToAlbum()
{
	refreshOpenAlbum();
	std::string data = getParms().second;
	std::string name = data.substr(0, data.find_first_of(','));
	std::string picPath = data.substr(data.find_first_of(',') + 1);
	if (m_openAlbum.doesPictureExists(name))
	{
		throw MyException("Error: Failed to add picture, picture with the same name already exists.\n");
	}
	
	Picture picture(++m_nextPictureId, name);
	picture.setPath(picPath);

	m_dataAccess.addPictureToAlbumByName(m_openAlbum.getName(), picture);
	std::cout << "Picture [" << picture.getId() << "] successfully added to Album [" << m_openAlbum.getName() << "]." << std::endl;
	_pipe->sendMessage("10");
	for (Picture& picture : m_openAlbum.getPictures())
	{
		_pipe->sendMessage("5," + m_currentAlbumName + "," + std::to_string(m_currentUserId) + "," + getFullPath(picture.getPath()) + "," + picture.getName());
	}
	_pipe->sendMessage("5," + m_currentAlbumName + "," + std::to_string(m_currentUserId) + "," + getFullPath(picture.getPath()) + "," + picture.getName());
}

std::pair<int, std::string> AlbumManager::getParms()
{
	std::string parm = this->_params.front();
	this->_params.pop();
	int id = stoi(parm.substr(0, parm.find_first_of(',')));
	std::string data = parm.substr(parm.find_first_of(',') + 1);
	return {id, data};
}

std::pair<CommandType, std::string> AlbumManager::getCommandType()
{
	//getting messages
	std::string data = this->_messages.front().second;
	CommandType type = this->_messages.front().first;
	this->_messages.pop();
	return { type, data };
}

void AlbumManager::removePictureFromAlbum()
{
	refreshOpenAlbum();
	std::pair<int, std::string> parms = getParms();
	if (!m_openAlbum.doesPictureExists(parms.second)) {
		throw MyException("Error: There is no picture with name <" + parms.second + ">.\n");
	}

	auto picture = m_openAlbum.getPicture(parms.second);
	m_dataAccess.removePictureFromAlbumByName(m_openAlbum.getName(), picture.getName());
	std::cout << "Picture <" << parms.second << "> successfully removed from Album [" << m_openAlbum.getName() << "]." << std::endl;
	_pipe->sendMessage("10");
	for (Picture& picture : m_openAlbum.getPictures())
	{
		if(parms.second != picture.getName()) _pipe->sendMessage("5," + m_currentAlbumName + "," + std::to_string(m_currentUserId) + "," + getFullPath(picture.getPath()) + "," + picture.getName());
	}
}


void AlbumManager::listPicturesInAlbum()
{
	refreshOpenAlbum();

	std::cout << "List of pictures in Album [" << m_openAlbum.getName() 
			  << "] of user@" << m_openAlbum.getOwnerId() <<":" << std::endl;
	
	const std::list<Picture>& albumPictures = m_openAlbum.getPictures();
	for (auto iter = albumPictures.begin(); iter != albumPictures.end(); ++iter) {
		std::cout << "   + Picture [" << iter->getId() << "] - " << iter->getName() << 
			"\tLocation: [" << iter->getPath() << "]\tCreation Date: [" <<
				iter->getCreationDate() << "]\tTags: [" << iter->getTagsCount() << "]" << std::endl;
	}
	std::cout << std::endl;
}
std::string AlbumManager::getFullPath(const std::string& relativePath) const
{
	char fullPath[MAX_PATH] = { 0 };

	if (!GetFullPathNameA(relativePath.c_str(), MAX_PATH, fullPath, NULL))
	{
		throw std::runtime_error("Error getting full path: " + std::to_string(GetLastError()));
	}
	return std::string(fullPath);
}

void AlbumManager::signalHandle(const int signal)
{
	if (signal == SIGINT) 
	{
		std::cout << "\nCTRL+C detected! Closing image viewer...\n";
		_isPressed = true;
	}
}

void AlbumManager::ctrlCListener(PROCESS_INFORMATION& processInfo)
{
	std::signal(SIGINT, AlbumManager::signalHandle);
	while (!_isPressed);
	TerminateProcess(processInfo.hProcess, 0);
}



void AlbumManager::showPicture()
{
	STARTUPINFOA startInfo;
	PROCESS_INFORMATION processInfo;
	std::string option = "";
	std::string path = "";
	refreshOpenAlbum();

	std::string picName = getInputFromConsole("Enter picture name: ");
	if ( !m_openAlbum.doesPictureExists(picName) )
	{
		throw MyException("Error: There is no picture with name <" + picName + ">.\n");
	}
	
	auto pic = m_openAlbum.getPicture(picName);
	if ( !fileExistsOnDisk(pic.getPath()) ) 
	{
		throw MyException("Error: Can't open <" + picName + "> since it doesnt exist on disk.\n");
	}
	//options
	do
	{
		option = getInputFromConsole("Choose Program(1 MsPaint, 2 IfranView): ");
	} while (option != IFRANVIEW && option != MSPAINT);
	//getting the path
	if (option == MSPAINT)
	{
		path = MSPAINT_PATH + std::string(" " + pic.getPath());
		std::replace(path.begin(), path.end(), '/', '\\');  
	}
	else if (option == IFRANVIEW)
	{
		path = IRFANVIEW_PATH + std::string(" " + getFullPath(pic.getPath()));
		std::replace(path.begin(), path.end(), '/', '\\');
	}
	ZeroMemory(&startInfo, sizeof(startInfo));
	startInfo.cb = sizeof(startInfo);
	ZeroMemory(&processInfo, sizeof(processInfo));
	//replacing so it will work
	if (!CreateProcessA(
		NULL, //no need name we use command line
		const_cast<char*>(path.c_str()),
		NULL, //flags currDir etc...
		NULL,
		FALSE,
		0,
		NULL,
		NULL,
		&startInfo, //a must which is start info and process info
		&processInfo))
	{
		throw MyException("Couldn't Open Picture!.\n");
	}

	//signal thread that will listen for ctrl c
	std::thread listenerThread(&AlbumManager::ctrlCListener, this, std::ref(processInfo));
	listenerThread.detach();
	//waiting for objet
	WaitForSingleObject(processInfo.hProcess, INFINITE);

	std::cout << "Closed Picture Succssesfully." << std::endl;
	//setting is pressed
	_isPressed = false;
	CloseHandle(processInfo.hProcess);
	CloseHandle(processInfo.hThread);
}



void AlbumManager::tagUserInPicture()
{
	refreshOpenAlbum();
	//getting the parmaters
	auto parms = getParms();
	int taggedUser = stoi(parms.second.substr(0, parms.second.find_first_of(",")));
	parms.second = parms.second.substr(parms.second.find_first_of(",") + 1);
	if (!m_openAlbum.doesPictureExists(parms.second)) 
	{
		throw MyException("Error: There is no picture with name <" + parms.second + ">.\n");
	}
	
	Picture pic = m_openAlbum.getPicture(parms.second);
	
	if ( !m_dataAccess.doesUserExists(taggedUser) ) {
		throw MyException("Error: There is no user with id @" + std::to_string(taggedUser) + "\n");
	}
	User user = m_dataAccess.getUser(taggedUser);
	try
	{
		m_dataAccess.tagUserInPicture(m_openAlbum.getName(), pic.getName(), user.getId());
		//sending what to tag and what we tagged to the pipe
		_pipe->sendMessage("13," + pic.getName());
		refreshOpenAlbum();
		pic = m_openAlbum.getPicture(parms.second);
		const std::set<int>& userTags = pic.getUserTags();
		for (const auto& user : userTags)
		{
			_pipe->sendMessage("12," + parms.second + "," + std::to_string(parms.first) + "," + std::to_string(user) + "," + pic.getName());
		}
		std::cout << "User @" << std::to_string(taggedUser) << " successfully tagged in picture <" << pic.getName() << "> in album [" << m_openAlbum.getName() << "]" << std::endl;
	}
	catch (const std::exception&)
	{
		std::cerr << "Error: User already tagged." << std::endl;
	}
}

void AlbumManager::untagUserInPicture()
{
	refreshOpenAlbum();

	auto parms = getParms();
	int taggedUser = stoi(parms.second.substr(0, parms.second.find_first_of(",")));
	parms.second = parms.second.substr(parms.second.find_first_of(",") + 1);
	if (!m_openAlbum.doesPictureExists(parms.second)) 
	{
		throw MyException("Error: There is no picture with name <" + parms.second + ">.\n");
	}

	Picture pic = m_openAlbum.getPicture(parms.second);

	if (!m_dataAccess.doesUserExists(taggedUser))
	{
		throw MyException("Error: There is no user with id @" + std::to_string(taggedUser) + "\n");
	}
	User user = m_dataAccess.getUser(taggedUser);
	if (! pic.isUserTagged(user))
	{
		throw MyException("Error: The user was not tagged! \n");
	}
	m_dataAccess.untagUserInPicture(m_currentAlbumName, pic.getName(), taggedUser);
	_pipe->sendMessage("13," + pic.getName());
	refreshOpenAlbum();
	pic = m_openAlbum.getPicture(parms.second);
	const std::set<int>& userTags = pic.getUserTags();
	for (const auto& user : userTags)
	{
		_pipe->sendMessage("12," + parms.second + "," + std::to_string(parms.first) + "," + std::to_string(user) + "," + pic.getName());
	}
}

void AlbumManager::listUserTags()
{
	refreshOpenAlbum();

	std::string picName = getInputFromConsole("Enter picture name: ");
	if ( !m_openAlbum.doesPictureExists(picName) ) {
		throw MyException("Error: There is no picture with name <" + picName + ">.\n");
	}
	auto pic = m_openAlbum.getPicture(picName); 

	const std::set<int> users = pic.getUserTags();

	if ( 0 == users.size() )  {
		throw MyException("Error: There is no user tegged in <" + picName + ">.\n");
	}

	std::cout << "Tagged users in picture <" << picName << ">:" << std::endl;
	for (const int user_id: users)
	{
			const User user = m_dataAccess.getUser(user_id);
			std::cout << user << std::endl;
	}
	std::cout << std::endl;

}


// ******************* User ******************* 
void AlbumManager::addUser()
{
	std::string name = getParms().second;
	User user(++m_nextUserId, name);
	
	m_dataAccess.createUser(user);
	_pipe->sendMessage("4");
	listUsers();
	std::cout << "User " << name << " with id @" << user.getId() << " created successfully." << std::endl;
}


void AlbumManager::removeUser()
{
	// get user name
	int userId = stoi(getParms().second);
	if ( !m_dataAccess.doesUserExists(userId) ) {
		throw MyException("Error: There is no user with id @" + std::to_string(userId) + "\n");
	}
	const User& user = m_dataAccess.getUser(userId);
	if (isCurrentAlbumSet() && userId == m_openAlbum.getOwnerId()) 
	{
		closeAlbum();
	}
	_pipe->sendMessage("4");
	m_dataAccess.deleteUser(user);
	listUsers();
	std::cout << "User @" << userId << " deleted successfully." << std::endl;
}

void AlbumManager::openUser()
{
	int userId = stoi(getParms().second);

	if (m_dataAccess.doesUserExists(userId))
	{
		
		_pipe->sendMessage("1," + std::to_string(userId) + "," + m_dataAccess.getUser(userId).getName());
		for (auto album : m_dataAccess.getAlbumsOfUser(m_dataAccess.getUser(userId)))
		{
			_pipe->sendMessage("2," + std::to_string(userId) + "," + album.getName());
		}
	}
}

void AlbumManager::userStatistics()
{
	int userId = stoi(getParms().second);
	if ( !m_dataAccess.doesUserExists(userId) )
	{
		throw MyException("Error: There is no user with id @" + std::to_string(userId) + "\n");
	}

	const User& user = m_dataAccess.getUser(userId);

	std::cout << "user @" << userId << " Statistics:" << std::endl << "--------------------" << std::endl <<
		"  + Count of Albums Tagged: " << m_dataAccess.countAlbumsTaggedOfUser(user) << std::endl <<
		"  + Count of Tags: " << m_dataAccess.countTagsOfUser(user) << std::endl <<
		"  + Average Tags per Album: " << m_dataAccess.averageTagsPerAlbumOfUser(user) << std::endl <<
		"  + Count of Albums Owned: " << m_dataAccess.countAlbumsOwnedOfUser(user) << std::endl;
}


// ******************* Queries ******************* 
void AlbumManager::topTaggedUser()
{
	const User& user = m_dataAccess.getTopTaggedUser();

	std::cout << "The top tagged user is: " << user.getName() << std::endl;
}

void AlbumManager::topTaggedPicture()
{
	const Picture& picture = m_dataAccess.getTopTaggedPicture();

	std::cout << "The top tagged picture is: " << picture.getName() << std::endl;
}

void AlbumManager::picturesTaggedUser()
{
	std::string userIdStr = getInputFromConsole("Enter user id: ");
	int userId = std::stoi(userIdStr);
	if ( !m_dataAccess.doesUserExists(userId) ) {
		throw MyException("Error: There is no user with id @" + userIdStr + "\n");
	}

	auto user = m_dataAccess.getUser(userId);

	auto taggedPictures = m_dataAccess.getTaggedPicturesOfUser(user);

	std::cout << "List of pictures that User@" << user.getId() << " tagged :" << std::endl;
	for (const Picture& picture: taggedPictures) {
		std::cout <<"   + "<< picture << std::endl;
	}
	std::cout << std::endl;
}


// ******************* Help & exit ******************* 
void AlbumManager::exit()
{
	std::exit(EXIT_SUCCESS);
}

void AlbumManager::help()
{
	system("CLS");
	printHelp();
}

std::string AlbumManager::getInputFromConsole(const std::string& message)
{
	std::string input;
	do {
		std::cout << message;
		std::getline(std::cin, input);
	} while (input.empty());
	
	return input;
}

bool AlbumManager::fileExistsOnDisk(const std::string& filename)
{
	struct stat buffer;   
	return (stat(filename.c_str(), &buffer) == 0); 
}

void AlbumManager::refreshOpenAlbum()
{
	if (!isCurrentAlbumSet()) 
	{
		throw AlbumNotOpenException();
	}
    m_openAlbum = m_dataAccess.openAlbum(m_currentAlbumName, m_currentUserId);
}

bool AlbumManager::isCurrentAlbumSet() const
{
    return !m_currentAlbumName.empty();
}

const std::vector<struct CommandGroup> AlbumManager::m_prompts  = {
	{
		"Supported Albums Operations:\n----------------------------",
		{
			{ CREATE_ALBUM        , "Create album" },
			{ OPEN_ALBUM          , "Open album" },
			{ CLOSE_ALBUM         , "Close album" },
			{ DELETE_ALBUM        , "Delete album" },
			{ LIST_ALBUMS         , "List albums" },
			{ LIST_ALBUMS_OF_USER , "List albums of user" }
		}
	},
	{
		"Supported Album commands (when specific album is open):",
		{
			{ ADD_PICTURE    , "Add picture." },
			{ REMOVE_PICTURE , "Remove picture." },
			{ SHOW_PICTURE   , "Show picture." },
			{ LIST_PICTURES  , "List pictures." },
			{ TAG_USER		 , "Tag user." },
			{ UNTAG_USER	 , "Untag user." },
			{ LIST_TAGS		 , "List tags." }
		}
	},
	{
		"Supported Users commands: ",
		{
			{ ADD_USER         , "Add user." },
			{ REMOVE_USER      , "Remove user." },
			{ LIST_OF_USER     , "List of users." },
			{ USER_STATISTICS  , "User statistics." },
		}
	},
	{
		"Supported Queries:",
		{
			{ TOP_TAGGED_USER      , "Top tagged user." },
			{ TOP_TAGGED_PICTURE   , "Top tagged picture." },
			{ PICTURES_TAGGED_USER , "Pictures tagged user." },
		}
	},
		{
		"Supported Bonuses:",
		{
			{ SET_READONLY      , "Set picture as readonly." },
			{ UNSET_READONLY   , "Unset picture as readonly." },
		}
	},
	{
		"Supported Operations:",
		{
			{ HELP , "Help (clean screen)" },
			{ EXIT , "Exit." },
		}
	}
};

void AlbumManager::setReadonly()
{
	refreshOpenAlbum();

	std::string picName = getInputFromConsole("Enter picture name: ");
	if (!m_openAlbum.doesPictureExists(picName))
	{
		throw MyException("Error: There is no picture with name <" + picName + ">.\n");
	}

	auto pic = m_openAlbum.getPicture(picName);
	std::string path = getFullPath(pic.getPath());
	m_dataAccess.setReadonly(path);
	std::cout << "Successfully Set Picture As ReadOnly" << std::endl;
}

void AlbumManager::unsetReadonly()
{
	refreshOpenAlbum();

	std::string picName = getInputFromConsole("Enter picture name: ");
	if (!m_openAlbum.doesPictureExists(picName))
	{
		throw MyException("Error: There is no picture with name <" + picName + ">.\n");
	}

	auto pic = m_openAlbum.getPicture(picName);

	std::string path = getFullPath(pic.getPath());
	m_dataAccess.unsetReadonly(path);
	std::cout << "Successfully Unset Picture As ReadOnly" << std::endl;
}
const std::map<CommandType, AlbumManager::handler_func_t> AlbumManager::m_commands = {
	{ CREATE_ALBUM, &AlbumManager::createAlbum},
	{ OPEN_ALBUM, &AlbumManager::openAlbum },
	{ CLOSE_ALBUM, &AlbumManager::closeAlbum },
	{ DELETE_ALBUM, &AlbumManager::deleteAlbum },
	{ LIST_ALBUMS, &AlbumManager::closeUser },
	{ LIST_ALBUMS_OF_USER, &AlbumManager::listUsers },
	{ ADD_PICTURE, &AlbumManager::addPictureToAlbum },
	{ REMOVE_PICTURE, &AlbumManager::removePictureFromAlbum },
	{ LIST_PICTURES, &AlbumManager::listPicturesInAlbum },
	{ SHOW_PICTURE, &AlbumManager::showPicture },
	{ TAG_USER, &AlbumManager::tagUserInPicture, },
	{ UNTAG_USER, &AlbumManager::untagUserInPicture },
	{ LIST_TAGS, &AlbumManager::listUserTags },
	{ ADD_USER, &AlbumManager::addUser },
	{ REMOVE_USER, &AlbumManager::removeUser },
	{ LIST_OF_USER, &AlbumManager::openUser },
	{ USER_STATISTICS, &AlbumManager::userStatistics },
	{ TOP_TAGGED_USER, &AlbumManager::topTaggedUser },
	{ TOP_TAGGED_PICTURE, &AlbumManager::topTaggedPicture },
	{ PICTURES_TAGGED_USER, &AlbumManager::picturesTaggedUser },
	{ SET_READONLY, &AlbumManager::setReadonly },
	{ UNSET_READONLY, &AlbumManager::unsetReadonly },
	{ HELP, &AlbumManager::help },
	{ EXIT, &AlbumManager::exit }
};
