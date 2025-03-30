#include "IDataAccess.h"
#include "SqlException.h"
#include "sqlite3.h"
#include <windows.h>
#include <io.h>
#include <queue>
#include "Pipe.h"

#define DB_NAME "galleryDB.sqlite"

/*
entity struct that will contain the data and the callback function
*/
typedef struct Entity
{
public:
	void* data;
	int (*callback)(void*, int, char**, char**);
	Entity(void* data, int (*callback)(void*, int, char**, char**)) : data(data), callback(callback) {}
}Entity;

class DatabaseAccess : public IDataAccess
{
public:
	DatabaseAccess();
	virtual ~DatabaseAccess();

	/*
	getting the ablums
	@return list of albums
	*/
	virtual const std::list<Album> getAlbums() override;
	/*
	getting albums of user
	@param user to get albums
	@return list of albums
	*/
	virtual const std::list<Album> getAlbumsOfUser(const User& user) override;
	/*
	creating an album
	@param album to create
	*/
	virtual void createAlbum(const Album& album) override;
	/*
	deleting the album
	@param album name
	@param user id
	*/
	virtual void deleteAlbum(const std::string& albumName, int userId) override;
	/*
	checking if the albums exists
	@param album name
	@param user id
	@return does exists
	@return does not exists
	*/
	virtual bool doesAlbumExists(const std::string& albumName, int userId) override;
	/*
	opening the album
	@param album name
	@param user id
	@return album that was opened
	*/
	virtual Album openAlbum(const std::string& albumName, const int userId) override; //added user id
	/*
	closing album
	@param album to close
	*/
	virtual void closeAlbum(Album& pAlbum) override;
	/*
	pritning albums
	*/
	virtual void printAlbums() override;

	// picture related
	/*
	adding pictures to the album
	@param album name
	@param picture to add
	*/
	virtual void addPictureToAlbumByName(const std::string& albumName, const Picture& picture) override;
	/*
	removing pictures to the album
	@param album name
	@param picture name to remove
	*/
	virtual void removePictureFromAlbumByName(const std::string& albumName, const std::string& pictureName) override;
	/*
	tagging user in pic
	@param album name
	@param picture to tag
	@param user id to tag
	*/
	virtual void tagUserInPicture(const std::string& albumName, const std::string& pictureName, int userId) override;
	/*
	untagging user in pic
	@param album name
	@param picture to untag
	@param user id to untag
	*/
	virtual void untagUserInPicture(const std::string& albumName, const std::string& pictureName, int userId) override;

	// user related
	/*
	printing the users
	*/
	virtual void printUsers(Pipe* pipe) override;
	/*
	getting the user by id
	@param user id
	@return the user
	*/
	virtual User getUser(int userId) override;
	/*
	creating a user
	@param user to create
	*/
	virtual void createUser(User& user) override;
	/*
	deleting user
	@param user to delete
	*/
	virtual void deleteUser(const User& user) override;
	/*
	checking if user exists
	@param user id
	@return does or not exist
	*/
	virtual bool doesUserExists(int userId) override;


	// user statistics
	/*
	counting the albums that are owned by user
	@param user
	@return amount of albums owned by user
	*/
	virtual int countAlbumsOwnedOfUser(const User& user) override;
	/*
	counting the amount of albums the user is tagged in
	@param user
	@return album tagged of user
	*/
	virtual int countAlbumsTaggedOfUser(const User& user) override;
	/*
	counting the tag of the user
	@param user
	@return counts of tags of the user
	*/
	virtual int countTagsOfUser(const User& user) override;
	/*
	getting the average tags per album of the user
	@param user
	@return average tags of the user per album
	*/
	virtual float averageTagsPerAlbumOfUser(const User& user) override;

	// queries
	/*
	getting to tagged user
	@return most tagged user
	*/
	virtual User getTopTaggedUser() override;
	/*
	getting to tagged picture
	@return most tagged picture
	*/
	virtual Picture getTopTaggedPicture() override;
	/*
	getting the top tagged pictures of user
	@param user
	@return list of the tagged pictures of the user
	*/
	virtual std::list<Picture> getTaggedPicturesOfUser(const User& user) override;
	/*
	generating id for user by table
	@param table to generate to
	@return new id
	*/
	virtual int generateNewId(const std::string& tableName) override;
	/*
	readonly to picture
	@param picture to readonly
	*/
	virtual void setReadonly(std::string& picture) override;
	/*
	unreadonly to picture
	@param picture to unreadonly
	*/
	virtual void unsetReadonly(std::string& picture) override;
	/*
	opening the database or memory
	@return success
	@return failure
	*/
	virtual bool open() override;
	/*
	closing database or memory
	*/
	virtual void close() override;
	/*
	cleaning memory
	*/
	virtual void clear() override;

private:
	sqlite3* _db;
	/*
	executing sql command
	@param command
	@param entity which is callback and data to get
	*/
	void executeCommand(const std::string& command, Entity* entity = nullptr);
};