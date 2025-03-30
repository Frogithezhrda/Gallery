#include "DatabaseAccess.h"

DatabaseAccess::DatabaseAccess()
{
	open();
}

DatabaseAccess::~DatabaseAccess()
{
	close();
}

bool DatabaseAccess::open()
{
	int doesFileExist = _access(DB_NAME, 0);
	if (sqlite3_open(DB_NAME, &this->_db) == SQLITE_OK && !doesFileExist)
	{
		return true; 
	}
	else // if not created will be execute
	{
		try
		{
			//creating tables
			executeCommand(BEGIN_TRANS);
			executeCommand(CREATE_TABLE_USERS);
			executeCommand(CREATE_TABLE_ALBUMS);
			executeCommand(CREATE_TABLE_PICTURES);
			executeCommand(CREATE_TABLE_TAGS);
			executeCommand(END_TRANS);
		}
		catch (const SqlException&)
		{
			executeCommand(ROLL);
		}
		catch(const std::exception&)
		{
			std::cerr << "Failed To Create DB" << std::endl;
		}
	}
	return false;
}



/*
@brief adding the count
input: @param callback(data, argc, argv, colName)
output: @return succsses @return unsuccsses
*/
static int callbackCount(void* data, int argc, char** argv, char** azColName)
{
	int* count = static_cast<int*>(data);

	*count = argv[0] ? atoi(argv[0]) : 0;

	return 0;
}
/*
adding the id
input: @param callback(data, argc, argv, colName)
output: @return succsses @return unsuccsses
*/
static int callbackID(void* data, int argc, char** argv, char** azColName)
{
	std::string columnName = "";
	std::string value = "";
	int* id = static_cast<int*>(data);

	for (int i = 0; i < argc; i++)
	{
		columnName = azColName[i];
		value = argv[i] ? argv[i] : NULL_COLUNM;
		//getting the id column
		if (columnName == ID_COLUNM || columnName == MAX_ID_COLUNM)
		{
			*id = stoi(value);
		}
	}
	return 0;
}
/*
adding the users to a list
input: @param callback(data, argc, argv, colName)
output: @return succsses @return unsuccsses
*/
static int callbackUsers(void* data, int argc, char** argv, char** azColName)
{
	std::string columnName = "";
	std::string value = "";
	std::list<User>* users = static_cast<std::list<User>*>(data);
	User user = User(0, "");
	for (int i = 0; i < argc; i++)
	{
		columnName = azColName[i];
		value = argv[i] ? argv[i] : NULL_COLUNM;
		//getting the users needed

		if (columnName == NAME_COLUNM)
		{
			user.setName(value);
		}
		else if (columnName == ID_COLUNM)
		{
			user.setId(stoi(value));
		}
	}
	users->push_back(user);
	return 0;
}

/*
adding the albums list
input: @param callback(data, argc, argv, colName)
output: @return succsses @return unsuccsses
*/
static int callbackList(void* data, int argc, char** argv, char** azColName)
{
	std::string columnName = "";
	std::string value = "";
	std::list<Album>* albums = static_cast<std::list<Album>*>(data);

	Album album;
	for (int i = 0; i < argc; i++)
	{
		columnName = azColName[i];
		value = argv[i] ? argv[i] : NULL_COLUNM;
		//albums parms
		if (columnName == NAME_COLUNM)
		{
			album.setName(value);
		}
		else if (columnName == USER_ID_COLUNM)
		{
			album.setOwner(std::stoi(value));
		}
		else if (columnName == CREATION_DATE_COLUNM)
		{
			album.setCreationDate(value);
		}
	}
	albums->push_back(album);
	return 0;
}

/*
adding the tags
input: @param callback(data, argc, argv, colName)
output: @return succsses @return unsuccsses
*/
static int callbackTags(void* data, int argc, char** argv, char** azColName)
{
	std::string columnName = "";
	std::string value = "";
	//tags are pair
	std::list<std::pair<int, int>>* tags = static_cast<std::list<std::pair<int, int>>*>(data);

	std::pair<int, int> tag;
	for (int i = 0; i < argc; i++)
	{
		columnName = azColName[i];
		value = argv[i] ? argv[i] : NULL_COLUNM;
		//getting the tags
		if (columnName == USER_ID_COLUNM)
		{
			tag.first = std::stoi(value);
		}
		else if (columnName == PICTURE_ID_COLUNM)
		{
			tag.second = std::stoi(value);
		}
	}
	tags->push_back(tag);
	return 0;
}
/*
adding the picturs list
input: @param callback(data, argc, argv, colName)
output: @return succsses @return unsuccsses
*/
static int callbackPics(void* data, int argc, char** argv, char** azColName)
{
	std::string columnName = "";
	std::string value = "";
	std::list<Picture>* albums = static_cast<std::list<Picture>*>(data);

	Picture pic(0, "");
	for (int i = 0; i < argc; i++)
	{
		columnName = azColName[i];
		value = argv[i] ? argv[i] : NULL_COLUNM;
		//pic parms
		if (columnName == NAME_COLUNM)
		{
			pic.setName(value);
		}
		else if (columnName == LOCATION_COLUNM)
		{
			pic.setPath(value);
		}
		else if (columnName == CREATION_DATE_COLUNM)
		{
			pic.setCreationDate(value);
		}
		else if (columnName == ID_COLUNM)
		{
			pic.setId(stoi(value));
		}
	}
	albums->push_back(pic);
	return 0;
}

void DatabaseAccess::close()
{
	sqlite3_close(this->_db); //closing database
	this->_db = nullptr;
}

void DatabaseAccess::clear()
{
	close(); //not really useful
}

void DatabaseAccess::deleteAlbum(const std::string& albumName, int userId)
{
	std::list<Picture> pics;
	Entity entity = Entity((void*)&pics, callbackPics);
	executeCommand(PICTURES_ALBUMS + albumName + ALBUM_PARM + std::to_string(userId) + ENDING, &entity);
	//removing the pictures
	for (Picture& pic : pics)
	{
		removePictureFromAlbumByName(albumName , pic.getName());
	}
	executeCommand(DELETE_FROM_TABLE(ALBUMS_COLUNM, USER_ID_COLUNM) + std::to_string(userId) + NAME_PARM + albumName + ENDING_STR);
}

int DatabaseAccess::generateNewId(const std::string& tableName)
{
	int biggest = 0;
	Entity entity = Entity((void*)&biggest, callbackID);

	executeCommand(MAX_ID(tableName), &entity);

	return biggest;
}

void DatabaseAccess::setReadonly(std::string& picture)
{
	const char* picturePath = picture.c_str();
	SetFileAttributesA(picturePath, GetFileAttributesA(picturePath) | FILE_ATTRIBUTE_READONLY);
}

void DatabaseAccess::unsetReadonly(std::string& picture)
{
	const char* picturePath = picture.c_str();
	SetFileAttributesA(picturePath, GetFileAttributesA(picturePath) & ~FILE_ATTRIBUTE_READONLY);
}

void DatabaseAccess::createUser(User& user)
{
	std::string query = INSERT_INTO_TABLE(USERS_COLUNM, PARMS_USER) + std::to_string(user.getId()) + AND_PARM + user.getName() + ENDING_QUERY_STR;
	executeCommand(query);
}

void DatabaseAccess::deleteUser(const User& user)
{
	std::list<Album> albums;
	Entity entity = Entity((void*)&albums, callbackList);
	//removing the tags
	executeCommand(DELETE_FROM_TABLE(TAGS_COLUNM, USER_ID_COLUNM) + std::to_string(user.getId()) + ENDING);
	executeCommand(QUERY_PARM(ALBUMS_COLUNM, USER_ID_COLUNM) + std::to_string(user.getId()) + ENDING, &entity);
	//removing the albums of the user
	for (Album& album : albums)
	{
		deleteAlbum(album.getName(), user.getId());
	}
	executeCommand(DELETE_FROM_TABLE(USERS_COLUNM, ID_COLUNM) + std::to_string(user.getId()) + ENDING);
}

void DatabaseAccess::printUsers(Pipe* pipe)
{
	std::list<User> users;
	Entity entity = Entity((void*)&users, callbackUsers);
	executeCommand(USERS_QUERY, &entity);
	//printing all
	std::cout << "Users list:" << std::endl << "-----------" << std::endl;
	for (const User& user : users)
	{
		pipe->sendMessage("14,@" + std::to_string(user.getId()) + " - " + user.getName());
	}
}

User DatabaseAccess::getUser(int userId)
{
	std::list<User> users;
	Entity entity = Entity((void*)&users, callbackUsers);
	executeCommand(QUERY_PARM(USERS_COLUNM, ID_COLUNM) + std::to_string(userId) + ENDING, &entity);
	return users.front();
}


bool DatabaseAccess::doesUserExists(int userId)
{
	std::list<User> users;
	Entity entity = Entity((void*)&users, callbackUsers);
	executeCommand(QUERY_PARM(USERS_COLUNM, ID_COLUNM) + std::to_string(userId) + ENDING, &entity);
	return !users.empty();
}

void DatabaseAccess::addPictureToAlbumByName(const std::string& albumName, const Picture& picture)
{
	int id = 0;
	Entity entity = Entity((void*)&id, callbackID);
	//getting id and adding pics
	executeCommand(ALBUMS_QUERY_NAME + albumName + ENDING_STR, &entity);
	executeCommand(INSERT_INTO_TABLE(PICTURES_COLUNM, PARMS_PIC) STR_PARM + picture.getName()
		+ AND_PARM_STR_FULL + picture.getPath() + AND_PARM_STR_FULL + picture.getCreationDate() + AND_PARM_STR + std::to_string(id) + ENDING_QUERY);
}

void DatabaseAccess::removePictureFromAlbumByName(const std::string& albumName, const std::string& pictureName)
{
	int id = 0;
	std::list<Picture> pics;
	Entity entity = Entity((void*)&id, callbackID);
	executeCommand(ALBUMS_QUERY_NAME + albumName + ENDING_STR, &entity);
	entity = Entity((void*)&pics, callbackPics);
	executeCommand(QUERY_PARM(PICTURES_COLUNM, ALBUM_ID_COLUNM) + std::to_string(id) + NAME_PARM + pictureName + ENDING_STR, &entity);
	//removing the tags
	executeCommand(DELETE_FROM_TABLE(TAGS_COLUNM, PICTURE_ID_COLUNM) + std::to_string(pics.front().getId()) + ENDING);
	executeCommand(DELETE_FROM_TABLE(PICTURES_COLUNM, ALBUM_ID_COLUNM) + std::to_string(id) + NAME_PARM + pictureName + ENDING_STR);
}

void DatabaseAccess::tagUserInPicture(const std::string& albumName, const std::string& pictureName, int userId)
{
	std::string query = INSERT_INTO_TABLE(TAGS_COLUNM, PARMS_TAG) + std::to_string(userId) + AND_PARM_CLEAN + PICTURE_ID_QUERY(pictureName, albumName) + ENDING_QUERY;
	executeCommand(query);
}

void DatabaseAccess::untagUserInPicture(const std::string& albumName, const std::string& pictureName, int userId)
{
	std::string query = DELETE_FROM_TAGS PICTURE_ID_QUERY(pictureName, albumName) + TAG_PARM + std::to_string(userId) + ENDING_STR;
	executeCommand(query);
}

const std::list<Album> DatabaseAccess::getAlbums()
{
	std::list<Album> list;
	Entity entity = Entity((void*)&list, callbackList);
	executeCommand(ALBUMS_QUERY, &entity);
	return list;
}

const std::list<Album> DatabaseAccess::getAlbumsOfUser(const User& user)
{
	std::list<Album> list;
	Entity entity = Entity((void*)&list, callbackList);
	executeCommand(QUERY_PARM(ALBUMS_COLUNM, USER_ID_COLUNM) + std::to_string(user.getId()) + ENDING, &entity);
	return list;

}

void DatabaseAccess::createAlbum(const Album& album)
{
	executeCommand(INSERT_INTO_TABLE(ALBUMS_COLUNM, PARMS_ALBUM) STR_PARM + album.getName() + AND_PARM_STR + std::to_string(album.getOwnerId()) + AND_PARM + album.getCreationDate() + ENDING_QUERY_STR);
}

Album DatabaseAccess::openAlbum(const std::string& albumName, const int userId)
{
	std::list<Album> albumList;
	std::list<Picture> picList;
	std::list<std::pair<int, int>> tagList;
	Album album;
	Entity entity = Entity((void*)&albumList, callbackList);
	//getting albums
	executeCommand(ALBUMS_QUERY_NAME + albumName + USER_ID_PARM + std::to_string(userId) + ENDING, &entity);
	album = Album(albumList.front().getOwnerId(), albumList.front().getName(), albumList.front().getCreationDate());
	//getting pics
	entity = Entity((void*)&picList, callbackPics);
	executeCommand(PICTURES_ALBUMS + albumName + ALBUM_PARM + std::to_string(userId) + ENDING, &entity);
	
	entity = Entity((void*)&tagList, callbackTags);
	for (Picture& pic : picList)
	{
		//getting tags
		executeCommand(QUERY_PARM(TAGS_COLUNM, PICTURE_ID_COLUNM) + std::to_string(pic.getId()) + ENDING, &entity);
		for (std::pair<int, int>& tag : tagList)
		{
			pic.tagUser(tag.first);
		}
		tagList = std::list<std::pair<int, int>>(); //reseting tag list
		album.addPicture(pic); //adding pics
	}
	return album;
}

void DatabaseAccess::printAlbums()
{
	std::list<Album> list;
	Entity entity = Entity((void*)&list, callbackList);
	executeCommand(ALBUMS_QUERY, &entity);
	//printing albums
	std::cout << "Album list:" << std::endl << "-----------" << std::endl;
	for (const Album& album : list) 
	{
		std::cout << "\t* [" + album.getName() + "] - created by user@" + std::to_string(album.getOwnerId()) << std::endl;
	}
}

bool DatabaseAccess::doesAlbumExists(const std::string& albumName, int userId)
{
	std::list<Album> list;
	Entity entity = Entity((void*)&list, callbackList);
	executeCommand(ALBUMS_QUERY_NAME + albumName + USER_ID_PARM + std::to_string(userId) + ENDING, &entity);
	return !list.empty();;
}

int DatabaseAccess::countAlbumsOwnedOfUser(const User& user)
{
	std::list<Album> list;
	Entity entity = Entity((void*)&list, callbackList);
	executeCommand(QUERY_PARM(ALBUMS_COLUNM, USER_ID_COLUNM) + std::to_string(user.getId()) + ENDING, &entity);
	return list.size();
}

int DatabaseAccess::countTagsOfUser(const User& user)
{
	std::list<std::pair<int, int>> list;
	Entity entity = Entity((void*)&list, callbackTags);
	executeCommand(QUERY_PARM(TAGS_COLUNM, USER_ID_COLUNM) + std::to_string(user.getId()) + ENDING, &entity);
	return list.size();
}

float DatabaseAccess::averageTagsPerAlbumOfUser(const User& user)
{
	int albumsTaggedCount = countAlbumsTaggedOfUser(user);

	if (albumsTaggedCount == 0)
	{
		return 0;
	}
	return static_cast<float>(countTagsOfUser(user)) / albumsTaggedCount;
}

int DatabaseAccess::countAlbumsTaggedOfUser(const User& user)
{
	int count = 0;
	Entity entity = Entity((void*)&count, callbackCount);
	executeCommand(COUNT_ALBUMS_QUERY + std::to_string(user.getId()) + ENDING_QUERY, &entity);
	return count;
}


void DatabaseAccess::closeAlbum(Album& pAlbum)
{
	//none to do
}

User DatabaseAccess::getTopTaggedUser()
{
	std::list<User> users;
	int tags = 0;
	User mostTags = User(0, "");
	int highestTags = 0;
	Entity entity = Entity((void*)&users, callbackUsers);
	executeCommand(USERS_QUERY, &entity);
	//getting users
	//YES i am using some different logic
	for (const User& user : users)
	{
		//counting tags
		tags = countTagsOfUser(user);
		if (tags > highestTags)
		{
			highestTags = tags;
			mostTags = user;
		}
	}
	//getting tags
	return mostTags;
}

Picture DatabaseAccess::getTopTaggedPicture()
{
	std::list<Picture> pics;
	Entity entity = Entity((void*)&pics, callbackPics);
	executeCommand(TAGED_PICTURES_QUERY, &entity);
	return pics.front();
}

std::list<Picture> DatabaseAccess::getTaggedPicturesOfUser(const User& user)
{
	std::list<Picture> pics;
	std::list<std::pair<int, int>> tagList;
	Entity entity = Entity((void*)&pics, callbackPics);
	//pics
	executeCommand(PICTURES_TAGS + std::to_string(user.getId()) + ENDING, &entity);
	entity = Entity((void*)&tagList, callbackTags);
	for (Picture& pic : pics)
	{
		//adding tags
		executeCommand(QUERY_PARM(TAGS_COLUNM, PICTURE_ID_COLUNM) + std::to_string(pic.getId()) + ENDING, &entity);
		for (std::pair<int, int>& tag : tagList)
		{
			pic.tagUser(tag.first);
		}
	}
	//pics with tags
	return pics;
}

void DatabaseAccess::executeCommand(const std::string& command, Entity* entity)
{
	char* errMessage = nullptr;
	std::string errorMsg = "";
	//excuting else exception
	if (sqlite3_exec(this->_db, command.c_str(), entity ? entity->callback : nullptr, entity ? entity->data : nullptr, &errMessage) != SQLITE_OK)
	{
		//freeing error msg memory
		throw SqlException(errMessage);
	}
}
