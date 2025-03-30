#include "DatabaseAccessTest.h"

int myIds = 2000;

void testMenu()
{
	DatabaseAccess DB;
	try
	{
		addUsers(&DB);
		//removing the pic
		DB.removePictureFromAlbumByName("Album14", "Pic2");
		//adding a new 1
		DB.addPictureToAlbumByName("Album14", Picture(++myIds, "Pic3"));
		//deleting user 13
		DB.deleteUser(User(START_INDEX, "13"));
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what(); //printing whats the error
	}
}

void addUsers(DatabaseAccess* DBAccess)
{
	User user = User(0, "");
	//adding user
	for (int i = START_INDEX; i < END_INDEX; i++)
	{
		user.setId(i);
		user.setName(std::to_string(i));
		//adding users and album
		DBAccess->createUser(user);
		DBAccess->createAlbum(Album(user.getId(), "Album" + std::to_string(i)));
		//adding pics
		DBAccess->addPictureToAlbumByName("Album" + std::to_string(i), Picture(++myIds, "Pic1"));
		DBAccess->addPictureToAlbumByName("Album" + std::to_string(i), Picture(++myIds, "Pic2"));
		//1 tag 
		DBAccess->tagUserInPicture("Album" + std::to_string(i), "Pic2", i);
		DBAccess->tagUserInPicture("Album" + std::to_string(i), "Pic1", i);

	}
	//git printing user and albums
	//DBAccess->printUsers();
	DBAccess->printAlbums();

}
