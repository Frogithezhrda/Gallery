#pragma once
#include <list>
#include "Album.h"
#include "User.h"
#include "Constants.h"
#include "Pipe.h"

class IDataAccess
{
public:
	virtual ~IDataAccess() = default;

	// album related
	/*
	getting the ablums
	@return list of albums
	*/
	virtual const std::list<Album> getAlbums() = 0;
	/*
	getting albums of user
	@param user to get albums
	@return list of albums
	*/
	virtual const std::list<Album> getAlbumsOfUser(const User& user) = 0;
	/*
	creating an album
	@param album to create
	*/
	virtual void createAlbum(const Album& album) = 0;
	/*
	deleting the album
	@param album name 
	@param user id
	*/
	virtual void deleteAlbum(const std::string& albumName, int userId) = 0;
	/*
	checking if the albums exists
	@param album name
	@param user id
	@return does exists
	@return does not exists
	*/
	virtual bool doesAlbumExists(const std::string& albumName, int userId) = 0;
	/*
	opening the album
	@param album name
	@param user id
	@return album that was opened
	*/
	virtual Album openAlbum(const std::string& albumName, const int userId) = 0; //added user id
	/*
	closing album
	@param album to close
	*/
	virtual void closeAlbum(Album& pAlbum) = 0;
	/*
	pritning albums
	*/
	virtual void printAlbums() = 0;

    // picture related
	/*
	adding pictures to the album
	@param album name
	@param picture to add
	*/
	virtual void addPictureToAlbumByName(const std::string& albumName, const Picture& picture) = 0;
	/*
	removing pictures to the album
	@param album name
	@param picture name to remove
	*/
	virtual void removePictureFromAlbumByName(const std::string& albumName, const std::string& pictureName) = 0;
	/*
	tagging user in pic
	@param album name
	@param picture to tag
	@param user id to tag
	*/
	virtual void tagUserInPicture(const std::string& albumName, const std::string& pictureName, int userId) = 0;
	/*
	untagging user in pic
	@param album name
	@param picture to untag
	@param user id to untag
	*/
	virtual void untagUserInPicture(const std::string& albumName, const std::string& pictureName, int userId) = 0;

	// user related
	/*
	printing the users
	*/
	virtual void printUsers(Pipe* pipe) = 0;
	/*
	getting the user by id
	@param user id
	@return the user
	*/
	virtual User getUser(int userId) = 0;
	/*
	creating a user
	@param user to create
	*/
	virtual void createUser(User& user ) = 0;
	/*
	deleting user 
	@param user to delete
	*/
	virtual void deleteUser(const User& user) = 0;
	/*
	checking if user exists
	@param user id
	@return does or not exist
	*/
	virtual bool doesUserExists(int userId) = 0;
	

	// user statistics
	/*
	counting the albums that are owned by user
	@param user
	@return amount of albums owned by user
	*/
	virtual int countAlbumsOwnedOfUser(const User& user) = 0;
	/*
	counting the amount of albums the user is tagged in
	@param user
	@return album tagged of user
	*/
	virtual int countAlbumsTaggedOfUser(const User& user) = 0;
	/*
	counting the tag of the user
	@param user
	@return counts of tags of the user
	*/
	virtual int countTagsOfUser(const User& user) = 0;
	/*
	getting the average tags per album of the user
	@param user
	@return average tags of the user per album
	*/
	virtual float averageTagsPerAlbumOfUser(const User& user) = 0;

	// queries
	/*
	getting to tagged user
	@return most tagged user
	*/
	virtual User getTopTaggedUser() = 0;
	/*
	getting to tagged picture
	@return most tagged picture
	*/
	virtual Picture getTopTaggedPicture() = 0;
	/*
	getting the top tagged pictures of user
	@param user
	@return list of the tagged pictures of the user
	*/
	virtual std::list<Picture> getTaggedPicturesOfUser(const User& user) = 0;
	/*
	generating id for user by table
	@param table to generate to
	@return new id
	*/
	virtual int generateNewId(const std::string& tableName) = 0;

	/*
	readonly to picture
	@param picture to readonly
	*/
	virtual void setReadonly(std::string& picture) = 0;
	/*
	unreadonly to picture
	@param picture to unreadonly
	*/
	virtual void unsetReadonly(std::string& picture) = 0;
	/*
	opening the database or memory
	@return success
	@return failure
	*/
	virtual bool open() = 0;
	/*
	closing database or memory
	*/
	virtual void close() = 0;
	/*
	cleaning memory
	*/
	virtual void clear() = 0;
};
