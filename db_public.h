

// Public interface for accessing the database.

// This header file should not include anything that is
// DB specific.

#ifndef __DB_PUBLIC_H_
#define __DB_PUBLIC_H_

#include <string>

// forward declarations
class DatabaseEnvironment;
class VaultDatabase;
class ObjectStorage;
class Object;

// The class responsible for creating
// and maintaining the database (by
// updating entries, etc.)
class DatabaseController
{
protected:
	////////////////////////
	// Database environments
	////////////////////////
	VaultDatabase * vaultDatabase_;

	///////////////////////////
	// Initialization functions
	///////////////////////////
	void LoadVaultDB();

public:
	DatabaseController();
	virtual ~DatabaseController();

	///////////////////////////
	// Initialization functions
	///////////////////////////
	void Initialize();

	//////////////////
	// Query functions
	//////////////////

	// Vaults
	ObjectStorage * GetVault(const std::string & playerName); // Create and return playerName's vault object
	void SaveVault(const std::string & playerName, ObjectStorage * newVault); // Update playerName's vault object
	string ShowObject(Object * object); // retrieve XML storage string for one object
	string ShowVault(const std::string & playerName); // retrieve playerName's vault
};



#endif

