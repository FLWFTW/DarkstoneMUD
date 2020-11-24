/***************************************************************************
 *  BabbleMUD, copyright (C) 2003 by Nick Gammon and David Haley           *
 *                                                                         *
 *  Email:  Nick Gammon <nick@gammon.com.au>                               *
 *  Web:    http://www.gammon.com.au                                       *
 *  Mail:   PO Box 124, Ivanhoe VIC 3079, Australia                        *
 *                                                                         *
 *  In order to use any part of this server code you must comply with      *
 *  the licence in 'licence.txt'. In particular, you may not remove this   *
 *  copyright notice.                                                      *
 *                                                                         *
 **************************************************************************/

#ifndef __MANAGED_OBJECTS_H_
#define __MANAGED_OBJECTS_H_

#include "globals.h"

#include <map>
using namespace std;

 /*

OK, what is all this about?

  I am trying to solve the problem of things in a busy environment
  (eg. a MUD) where you might have lots of pointers which are being
  created and deleted (eg.  players joining, leaving, objects being
  created and destroyed), and you want to have things referring to each
  other. eg. a player might be following another player, or an event might
  refer to a player or object.

  Rather than storing pointers to the thing-to-be-referred-to we will 
  get a unique ID and store that instead. Then, before attempting to use
  the thing, we will look up its ID in the ID map. If found, the object
  still exists and we can use the looked-up pointer. If the object has been
  delete for any reason, then the lookup will fail, and we know the object
  no longer exists.

  The constructor is supplied the owner map (eg. a map of rooms), so we 
  know which map to remove ourselves from eventually, and can add ourselves
  to it initially. 

  Later on, other things would use GetId to find the object's ID so they can
  refer to it (by storing in a uint64 field).

  Then, when they want to get the actual pointer they use FindObject (or similar)
  to find the pointer to it. If NULL is returned, the object has gone away.

  A note of caution - because deleting the object also deletes it from the owner
  map you want to be careful with iterators, deleting an item from a map invalidates
  the iterator. Thus the normal for_each would fail if something for_each calls
  deletes the item it found (ie. the item deletes itself). The function provided below:
  safe_for_each increments the iterator before calling the function so it should
  work, even if the function deletes the object.

  Example:

  //  owner map:

  class tPlayer;

  tPointerOwner<tPlayer> PlayerMap;

  // example owned object - passes map name to constructor:

  class tPlayer : public tObject<tPlayer>
    {

    public:

    // constructor
    tPlayer () : tObject<tPlayer> (PlayerMap) {};

    };  // end of class tPlayer

  // create an instance:

  tPlayer * p1 = new tPlayer;

  // is now in map:

  cout << "Map count: " << PlayerMap.size () << endl;

  // get id from pointer:

  tId<tPlayer> i = p1->GetId ();   // get player's ID

  // get pointer back from id:

  tPlayer * p = PlayerMap [i];

  if (p)  // if not null, it exists in map, thus pointer is valid
    {
    // do something with it
    }

  // delete any of the pointers - removes it from the map

  delete p;

  // not in map now:

  cout << "Map count: " << PlayerMap.size () << endl;

*/

// the base type for our pointer identifiers
typedef uint64 id;

// Moved this to being inside the map class
//      -- Ksilyan

// get unique number (adds 1 every time)
//uint64 unique (void);
// defined in utility.cpp

// identifier for a class - this gives type safety over straight integers
template <class T>
class tId
{
	protected:

		id ID;    // the identifier

	public:

		// default constructor for when we don't have an ID yet
		tId () : ID (0) { };

		// normal constructor - creates with supplied id
		tId (const id i) : ID (i) { };

		// copy constructor 
		tId (const tId & rhs) : ID (rhs.ID) { };

		bool operator == (const tId & rhs) const
		{
			return (ID == rhs.ID);
		}
		bool operator != (const tId & rhs) const
		{
			return (ID != rhs.ID);
		}

		bool operator ! () const 
		{
			return (ID == 0);
		}

		bool operator < ( const tId & rhs ) const
		{
			return ID < rhs.ID;
		}

		// operator=  (assignment)
		const tId & operator= (const tId & rhs)
		{
			ID = rhs.ID;
			return *this;
		};

		// cast to id - return internal ID
		// commented out because with it we lose some type-safety
		//   operator id () const { return ID; };

		// return value (number) corresponding to internal ID
		id Value () const { return ID; };

};  // end of class tId

template <class T>
class tManagedObject;

// an owner of pointers 
// it uses a map to store them, for fast-lookup purpose
// insertion and deletion is restricted to the friend class (the pointers)

template <class T>
class tPointerOwner
  {
  public:

    typedef map<id, T *, less<id> > container_type;

    typedef typename container_type::value_type     value_type;
    typedef typename container_type::size_type      size_type;
    typedef typename container_type::iterator       iterator;
    typedef typename container_type::const_iterator const_iterator;

  protected:

    // this is where we put them
    container_type c;     // container
	
	// for unique IDs
	uint64 m_NextUnique;

  public:

	  tPointerOwner() : m_NextUnique(1) { ; }

    bool empty () const                        { return c.empty (); }
    size_type size () const                    { return c.size ();  }
                                              
    iterator find (const id& key)              { return c.find (key); }
    const_iterator find (const id& key) const  { return c.find (key); }

    iterator begin()                           { return c.begin (); }
    const_iterator begin() const               { return c.begin (); }

    iterator end()                             { return c.end (); }
    const_iterator end() const                 { return c.end (); }
                                
    // operator [] gives us back the original pointer

    T * operator[] (const tId<T> id) 
      {
      iterator i = c.find (id.Value ());

      if (i == c.end ())
         return NULL;

      return (T *) i->second;
      }

	uint64 unique (void)
	{
		return m_NextUnique++;
	} // end of unique

  protected:
  friend class tManagedObject<T>;  // so they can erase from us

  // insertion and deletion is protected so only our friend 
  // (the object who we are storing in the list)
  // can delete or insert

    void insert(const tId<T> item, T * ptr)
      {
      c [item.Value ()] = ptr;
      };

    void erase (const tId<T> item)
      {
      iterator i = c.find (item.Value ());
      if (i != c.end ())
        c.erase (i);
      }

  };  // end of class tPointerOwner


// object for a class
template <class T>
class tManagedObject
  {

  public:

  typedef tPointerOwner<T> tOwnerMap;

  private:
    tOwnerMap & m_owner_map;   // the map that points to us
    const tId<T>   m_id;          // our unique ID

  public:

  // constructor remembers which map points to us, gets unique ID
  tManagedObject (tOwnerMap & owner_map)  // owning map
     : m_owner_map (owner_map), 
       m_id (owner_map.unique ())
    {
     // put ourselves into the map
    m_owner_map.insert (m_id, reinterpret_cast <T *> (this)); 
    };  // constructor
  
  // destructor deletes us from the map
  virtual ~tManagedObject ()
    {
    m_owner_map.erase (m_id);
    }

  // return details about this object
  tOwnerMap &  GetMap  (void) const { return m_owner_map; };
  tId<T>       GetId   (void) const { return m_id; };

  };  // end of class tObject


// safe_for_each.  Apply a function to every element of a range.
// should handle correctly an element deleting itself 

template <class ITER, class F>
F safe_for_each (ITER first, ITER last, F func) 
  {
  while (first != last)
    func (*first++);
  return func;
  }

#endif // include guard
