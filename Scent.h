#ifndef _SCENT_H_
#define _SCENT_H_

#include <iostream>
#include <string>

class Scent
{
	private:

	int scentID_;
	std::string scentName_;
	std::string scentDescription_;

	public:

	Scent(int aScentID, const std::string & aScentName, const std::string & aScentDescription);
	~Scent();

	// Sets and Gets

	int getID() const { return scentID_; }
	const std::string & getName() const { return scentName_; }
	const std::string & getDescription() const { return scentDescription_; }

	void setID(int aID) { scentID_ = aID; }
	void setName(const std::string & aScentName) { scentName_ = aScentName; }
	void setDescription(const std::string & aScentDescription) { scentDescription_ = aScentDescription; }
};

#endif
