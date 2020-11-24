
#ifndef SHARED_STR_H_
#define SHARED_STR_H_

// Shared string management
#include <sharedstr.hpp>
// Declare shared string manager wrapper
DECLARE_MANAGER_WRAPPER(HashTableWrapper);

// Shared string type
typedef SharedString::SharedString< HashTableWrapper > shared_str;


#endif

