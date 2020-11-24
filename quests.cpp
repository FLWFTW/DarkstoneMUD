
#include <fstream>
#include <assert.h>
#include <time.h>

#include "quests.h"
#include "globals.h"
#include "World.h"
#include "lua_util.h"

#include "character.h"
#include "paths.const.h"

#include "mxp.h"

extern "C" {
    #include <lualib.h>
    #include <lauxlib.h>
}

// External functions
void do_help(Character*, const char* );


const int SUB_EDITQUEST_EDITNOTES = SUB_PAUSE + 1;
const int SUB_EDITQUEST_EDITSTEP  = SUB_PAUSE + 2;

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

// Player Commands

void do_questjournal(Character * ch, const char* argument)
{
    char arg[MAX_INPUT_LENGTH];

    // Get the command
    argument = one_argument(argument, arg);
    std::string cmd = strlower(arg);

    if (cmd.length() == 0)
    {
        // Show currently active quests
        return;
    }

    if (cmd == "completed")
    {
        // Show completed quests
        return;
    }

    if (cmd == "succeeded")
    {
        // Show quests that succeeded
        return;
    }

    if (cmd == "failed")
    {
        // Show quests that failed
        return;
    }

    // If we got here, we must be trying to get the details for a quest.
    // Only one special case: if the name is "details", then get an
    // additional argument for the quest name.
    // (This is so that you can still get the quest details for a 
    // quest named e.g. "completed" even if we shouldn't call any of
    // our quests by names like that.)

    argument = one_argument(argument, arg);
    std::string quest = strlower(arg);

    if (quest == "details")
    {
        argument = one_argument(argument, arg);
        quest = strlower(arg);
    }

    // Does the character know about this quest?

}


///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

// Administrator Commands

void do_showquest(Character * ch, const char* argument)
{
    char arg[MAX_INPUT_LENGTH];


    // Get the command argument
    argument = one_argument(argument, arg);
    std::string showWhat = strlower(arg);

    if (showWhat.length() == 0)
    {
        ch->sendText("Show which quest?\n");
        return;
    }
    else if (showWhat == "all" || showWhat == "list")
    {
        Iterator<Quest> quests = 
            gQuestManager->getQuestIterator();

        if (quests.size() == 0)
        {
            ch ->sendText("There are no quests.\n");
            return;
        }

        while (quests.hasNext())
        {
            const Quest & q = quests.next();

            ch->sendText("- ");
            ch->sendText(MXPTAG("ShowQuestItem"));
            ch->sendText(q.getName());
            ch->sendText(MXPTAG("/ShowQuestItem"));
            ch->sendText(": ");
            ch->sendText(q.getTitle());
            ch->sendText(": ");
            ch->sendText(q.getSummary());
            ch->sendText("\n");
        }

        return;
    }
    else
    {
        // Show one specific quest.
        Quest q;
        bool found = gQuestManager->getQuest(showWhat, q);

        if (!found)
        {
            ch->sendText("No such quest.\n");
            return;
        }

        ch->sendText("Quest: ");
        ch->sendText(q.getTitle());
        ch->sendText(" (");
        ch->sendText(q.getName());
        ch->sendText(")\nAuthor: ");
        ch->sendText(q.getAuthor());
        ch->sendText("\nSummary: ");
        ch->sendText(q.getSummary());
        ch->sendText("\nCreated: ");
        time_t t = q.getDateCreated();
        ch->sendText(ctime(&t));
        ch->sendText(", Modified: ");
        t = q.getDateModified();
        ch->sendText(ctime(&t));

        if (q.getNotes().length() > 0)
        {
            ch->sendText("\nNotes:\n");
            ch->sendText("------------------------------------"
                         "------------------------------------");
            ch->sendText("\n");
            ch->sendText(q.getNotes());
        }
        ch->sendText("\n");
        ch->sendText("------------------------------------"
                     "------------------------------------");
        ch->sendText("\n");
        ch->sendText("Steps:\n");

        std::vector<Quest::Step>::const_iterator it;
        for (it = q.getSteps().begin();
             it != q.getSteps().end();
             it++)
        {
            const Quest::Step & s = *it;
            ch->sendText("  - ");
            ch->sendText(s.name_);
            ch->sendText(": ");
            ch->sendText(s.description_);
            ch->sendText("[end]\n");
        }

        return;
    }

    return;
}

static bool ValidQuestName(const std::string & str)
{
    if (str.length() == 0)
        return false;

    // Only allow: alphanumeric, _
    for (std::string::size_type pos = 0; pos < str.length(); pos++)
    {
        if ( !isalnum(str[pos]) &&
             str[pos] != '_'
             ) {
            return false;
        }
    }

    // Make sure the first character is alpha, _
    if ( !isalpha(str[0]) && str[0] != '_' )
        return false;

    return true;
}

static bool ValidStepName(const std::string & str)
{
    if (str.length() == 0)
        return false;

    // Only allow: alphanumeric, _
    for (std::string::size_type pos = 0; pos < str.length(); pos++)
    {
        if ( !isalnum(str[pos]) &&
             str[pos] != '_'
             ) {
            return false;
        }
    }

    return true;
}

static bool BuiltinStepName(const std::string & str)
{
    return
        str == "success"
        || str == "failure"
        || str == "terminal"
        ;
}
    
static bool ValidSetString(const std::string & str)
{
    if (str.length() == 0)
        return false;

    // the only valid properties to set are the following:
    return
        /*str == "name"*/ false
        || str == "title"
        || str == "author"
        || str == "summary"
        ;

}

static bool EditQuest_GetSharedStr(Character *ch, void *& ptr, shared_str & quest)
{
    if (ptr == NULL) {
        return false;
    }

    quest = *(shared_str*) ptr;
    delete (shared_str*) ptr;
    ptr = NULL;

    return true;
}

void do_editquest(Character * ch, const char* argument)
{
    char arg[MAX_STRING_LENGTH];

    if (IS_NPC(ch))
    {
        ch->sendText("PCs only.\n");
        return;
    }

    shared_str quest;
    shared_str stepName;
    std::string notes;
    std::string description;
    std::ostringstream os;
    bool retval;

    // See if we're coming out of a buffer edit
    switch (ch->substate)
    {
        default:
            // Normal editing mode -- leave the switch statement
            break;

        case SUB_EDITQUEST_EDITSTEP:
            // Coming out of a buffer edit.
            if (!EditQuest_GetSharedStr(ch, ch->dest_buf, quest))
            {
                // Erp.
                gTheWorld->LogBugString("ERROR: character has no "
                                        "dest_buf in do_editquest!");
                ch->sendText("Fatal error...\n");
                ch->substate = SUB_NONE;
                return;
            }

            if (!EditQuest_GetSharedStr(ch, ch->spare_ptr, stepName))
            {
                // Erp...
                gTheWorld->LogBugString("ERROR: character has no "
                                        "spare_ptr in do_editquest!");
                ch->sendText("Fatal error...\n");
                ch->substate = SUB_NONE;
                return;
            }
            
            // Get ch's editor buffer
            description = copy_buffer_string(ch);
            // Clear the substate, buffer, etc.
            stop_editing(ch);

            // Sanity check the quest name first.
            if (gQuestManager->hasQuest(quest) == false)
            {
                ch->sendText("UNEXPECTED: No such quest. (Huh?!?)\n");
                return;
            }

            // Sanity check the step name -- even though it should already have been checked
            // before we entered the edit mode
            if (!ValidStepName(stepName.str()))
            {
                ch->sendText("That's not a valid step name.\n");
                return;
            }
            
            // Disallow [=====[ or ]=====] sequences. We use these
            // for the Lua long string delimiters.
            if (description.find("[=====[") != std::string::npos ||
                description.find("]=====]") != std::string::npos) {
                ch->sendText("Notes cannot contain the sequences "
                             "[=====[ or ]=====].\n");
                return;
            }

            // First, create the step sub-table
            // Quests.quest.steps[stepname] = {};
            os << QuestManager::QUESTS_TABLE_NAME << "." << quest.str()
                << ".steps[\"" << stepName.str() << "\"]"
                << " = {};";

            // Next, give it a description
            // Quests.quest.steps[stepname].description = [=====[ ... ]=====]
            os << QuestManager::QUESTS_TABLE_NAME << "." << quest.str()
                << ".steps[\"" << stepName.str() << "\"]"
                << ".description = [=====[" << description << "]=====];";
        
            // Finally, update the last modified time
            os << QuestManager::QUESTS_TABLE_NAME << "." << quest.str()
                << ".dateModified = " << time(NULL) << ";";

            retval = gQuestManager->interpret(os.str());

            if (retval) {
                ch->sendText("Quest step description set.\n");
            }
            else {
                ch->sendText("Failed to set quest step description; see game "
                             "log for Lua bug.\n");
            }

            return;

        case SUB_EDITQUEST_EDITNOTES:
            // Coming out of a buffer edit.
            if (!EditQuest_GetSharedStr(ch, ch->dest_buf, quest))
            {
                // Erp.
                gTheWorld->LogBugString("ERROR: character has no "
                                        "dest_buf in do_editquest!");
                ch->sendText("Fatal error...\n");
                ch->substate = SUB_NONE;
                return;
            }
            
            // Get ch's editor buffer
            notes = copy_buffer_string(ch);
            // Clear the substate, buffer, etc.
            stop_editing(ch);

            // Sanity check the quest name first.
            if (gQuestManager->hasQuest(quest) == false)
            {
                ch->sendText("UNEXPECTED: No such quest. (Huh?!?)\n");
                return;
            }
            
            // Disallow [=====[ or ]=====] sequences. We use these
            // for the Lua long string delimiters.
            if (notes.find("[=====[") != std::string::npos ||
                notes.find("]=====]") != std::string::npos) {
                ch->sendText("Notes cannot contain the sequences "
                             "[=====[ or ]=====].\n");
                return;
            }

            // Quests.quest.notes = [=====[ ... ]=====]
            os << QuestManager::QUESTS_TABLE_NAME << "." << quest.str()
                << ".notes = [=====[" << notes << "]=====];";
            
            // Finally, update the last modified time
            os << QuestManager::QUESTS_TABLE_NAME << "." << quest.str()
                << ".dateModified = " << time(NULL) << ";";

            bool retval = gQuestManager->interpret(os.str());

            if (retval) {
                ch->sendText("Quest notes set.\n");
            }
            else {
                ch->sendText("Failed to set quest notes; see game "
                             "log for Lua bug.\n");
            }

            return;
    }

    // Get the first argument
    argument = one_argument(argument, arg);
    std::string cmd = strlower(arg);

    if (cmd == "" || cmd == "help")
    {
        do_help(ch, "editquest");
        return;
    }


    if (cmd == "create")
    {
        // Get the second argument
        argument = one_argument(argument, arg);
        std::string whichQuest = strlower(arg);

        Quest q;
        if (gQuestManager->getQuest(whichQuest, q) == true)
        {
            ch->sendText("That quest already exists!\n");
            return;
        }

        // Make sure it's a valid quest name
        if (!ValidQuestName(whichQuest))
        {
            ch->sendText("Not a valid quest name.\n");
            return;
        }

        // Create the quest
        std::ostringstream os;
        
        // Quests.quest = {};
        os << QuestManager::QUESTS_TABLE_NAME << "." << whichQuest << " = {};";
        
        // Default values for quest
        os << QuestManager::QUESTS_TABLE_NAME << "." << whichQuest
            << ".name = [[" << whichQuest << "]];";
        os << QuestManager::QUESTS_TABLE_NAME << "." << whichQuest
            << ".title = [[" << whichQuest << " (newly created)]];";
        os << QuestManager::QUESTS_TABLE_NAME << "." << whichQuest
            << ".author = [[" << ch->getName().str() << "]];";
        os << QuestManager::QUESTS_TABLE_NAME << "." << whichQuest
            << ".steps = {};";

        // Create the built-in steps
        os << QuestManager::QUESTS_TABLE_NAME << "." << whichQuest
            << ".steps.success = {};";
        os << QuestManager::QUESTS_TABLE_NAME << "." << whichQuest
            << ".steps.success.description = [[The quest was successful.]];";
        os << QuestManager::QUESTS_TABLE_NAME << "." << whichQuest
            << ".steps.failure = {};";
        os << QuestManager::QUESTS_TABLE_NAME << "." << whichQuest
            << ".steps.failure.description = [[You failed this quest.]];";
        os << QuestManager::QUESTS_TABLE_NAME << "." << whichQuest
            << ".steps.terminal = {};";
        os << QuestManager::QUESTS_TABLE_NAME << "." << whichQuest
            << ".steps.terminal.description = [[The quest was aborted.]];";


        // Set the creation time
        os << QuestManager::QUESTS_TABLE_NAME << "." << whichQuest
            << ".dateCreated = " << time(NULL) << ";";
        // Set the last modified time
        os << QuestManager::QUESTS_TABLE_NAME << "." << whichQuest
            << ".dateModified = " << time(NULL) << ";";


        if (gQuestManager->interpret(os.str())) {
            ch->sendText("Success.\n");
        }
        else {
            ch->sendText("Failure.\n");
        }

        return;
    }

    if (cmd == "deletequest")
    {
        ch->sendText("Not yet implemented.\n");
        // TODO: implement this!
        return;
    }

    if (cmd == "deletestep")
    {
        // Get the second argument
        argument = one_argument(argument, arg);
        std::string whichQuest = strlower(arg);

        Quest q;

        if (gQuestManager->getQuest(whichQuest, q) == false)
        {
            ch->sendText("No such quest...\n");
            return;
        }

        // Get the third argument -- the step to remove
        argument = one_argument(argument, arg);
        std::string whichStep = strlower(arg);

        // Make sure it is a legal step name
        if (!ValidStepName(whichStep))
        {
            ch->sendText("That's not a valid step name.\n");
            return;
        }

        // Make sure we're not deleting a builtin
        if (BuiltinStepName(whichStep))
        {
            ch->sendText("Can't delete a built-in step.\n");
            return;
        }
        
        std::ostringstream os;

        // Quests.quest.steps[stepname] = nil
        os << QuestManager::QUESTS_TABLE_NAME << "." << whichQuest
            << ".steps" << "[\"" << whichStep << "\"] = nil;";
        
        // Set the last modified time
        os << QuestManager::QUESTS_TABLE_NAME << "." << whichQuest
            << ".dateModified = " << time(NULL) << ";";


        if (gQuestManager->interpret(os.str())) {
            ch->sendText("Success.\n");
        }
        else {
            ch->sendText("Failure.\n");
        }

        return;
    }

    if (cmd == "editstep")
    {
        // Get the second argument
        argument = one_argument(argument, arg);
        std::string whichQuest = strlower(arg);

        Quest q;

        if (gQuestManager->getQuest(whichQuest, q) == false)
        {
            ch->sendText("No such quest...\n");
            return;
        }

        // Get the third argument -- the step to create
        argument = one_argument(argument, arg);
        std::string whichStep = strlower(arg);

        // Make sure it is a legal step name
        if (!ValidStepName(whichStep))
        {
            ch->sendText("That's not a valid step name.");
            return;
        }

        ch->substate = SUB_EDITQUEST_EDITSTEP;
        ch->dest_buf  = new shared_str(whichQuest);
        ch->spare_ptr = new shared_str(whichStep);

        const Quest::Step * step;
        step = q.getStep(whichStep);

        if (step) {
            start_editing(ch, step->description_.c_str());
        }
        else {
            start_editing(ch, "");
        }

        return;
    }

    if (cmd == "reload")
    {
        QuestManager * m = new QuestManager();
        bool success = m->initialize(QUESTS_FILE);

        if (success)
        {
            gTheWorld->LogString("Quests reloaded.");
            delete gQuestManager;
            gQuestManager = m;
        }
        else {
            ch->sendText("Failed to reload quests.");
        }

        return;
    }

    if (cmd == "setnotes")
    {
        // Get the second argument
        argument = one_argument(argument, arg);
        std::string whichQuest = strlower(arg);

        Quest q;

        if (gQuestManager->getQuest(whichQuest, q) == false)
        {
            ch->sendText("No such quest...\n");
            return;
        }

        ch->substate = SUB_EDITQUEST_EDITNOTES;
        ch->dest_buf = new shared_str(whichQuest);
        start_editing( ch, q.getNotes().c_str() );
        return;
    }

    if (cmd == "rename")
    {
        ch->sendText("Not yet implemented.\n");
        // TODO: implement this!
        return;
    }

    if (cmd == "set")
    {
        // get the quest whose property to set
        argument = one_argument(argument, arg);
        std::string whichQuest = strlower(arg);

        // Make sure the quest actually exists
        Quest q;
        if (gQuestManager->getQuest(whichQuest, q) == false)
        {
            ch->sendText("No such quest...\n");
            return;
        }

        // get the property to set
        argument = one_argument(argument, arg);
        std::string setWhat = strlower(arg);

        if (setWhat.length() == 0)
        {
            ch->sendText("Set what?\n");
            return;
        }

        // sanity check whatever we're trying to set
        if (!ValidSetString(setWhat))
        {
            ch->sendText("You can't set that...\n");
            return;
        }

        std::string value = argument;

        // Disallow [=====[ or ]=====] sequences. We use these
        // for the Lua long string delimiters.
        if (value.find("[=====[") != std::string::npos ||
            value.find("]=====]") != std::string::npos) {
            ch->sendText("Value cannot contain the sequences "
                         "[=====[ or ]=====].\n");
            return;
        }

        std::ostringstream os;
        os << QuestManager::QUESTS_TABLE_NAME << "." << whichQuest
            << "." << setWhat << " = [=====[" << value << "]=====]";
        
        // Set the last modified time
        os << QuestManager::QUESTS_TABLE_NAME << "." << whichQuest
            << ".dateModified = " << time(NULL) << ";";

        if (gQuestManager->interpret(os.str())) {
            ch->sendText("Success.\n");
        }
        else {
            ch->sendText("Failure.\n");
        }

        return;
    }

    if (cmd == "save")
    {
        if (gQuestManager->save(QUESTS_FILE)) {
            ch->sendText("Done.\n");
        }
        else {
            ch->sendText("Failed to save quests.\n");
        }
        return;
    }

    if (cmd == "interpret")
    {
        // Make sure the person is authorized...
        if (get_trust(ch) < LEVEL_STONE_MASTER)
        {
            ch->sendText("You can't do that...\n");
            return;
        }

        // Interpret the rest of the argument string
        if (gQuestManager->interpret(argument)) {
            ch->sendText("Success.\n");
        }
        else {
            ch->sendText("Failure.\n");
        }

        return;
    }
    
    
    // If we got here, the command wasn't valid
    do_help(ch, "editquest");
    return;
}

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

// Quest object

Quest::Quest(const shared_str & name,
             const shared_str & title,
             const shared_str & author,
             const std::vector<Quest::Step> & steps,
             time_t dateCreated,
             time_t dateModified) :
    name_(name), title_(title), author_(author), steps_(steps),
    dateCreated_(dateCreated), dateModified_(dateModified)
{
    /* empty */
}

const Quest::Step * Quest::getStep(const shared_str & name)
{
    vector<Quest::Step>::iterator it;
    for (it = this->steps_.begin(); it != steps_.end(); it++)
    {
        Quest::Step & s = *it;
        if (s.name_ == name) {
            return &s;
        }
    }

    // not found
    return NULL;
}


///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

// QuestManager

const char * QuestManager::QUESTS_TABLE_NAME = "Quests";

QuestManager::QuestManager()
{
    // Get the state from the World
    luaState_ = gTheWorld->getLuaState();
}

QuestManager::~QuestManager()
{
}

bool QuestManager::initialize(const std::string & questFile)
{
    // Kill the quests table in the Lua state
    std::ostringstream os;
    os << QUESTS_TABLE_NAME << " = nil";
    Lua::Interpret(luaState_, os.str());

    // Now, load the saved file from disk
    int retval = luaL_loadfile(luaState_, questFile.c_str())
        || lua_pcall(luaState_, 0, 0, 0);

    if (retval != 0)
    {
        gTheWorld->LogBugString(lua_tostring(luaState_, -1));
        lua_pop(luaState_, 1); // pop the error message
        return false;
    }

    // Get the quest table
    lua_getglobal(luaState_, QUESTS_TABLE_NAME);

    if ( !lua_istable(luaState_, -1) )
    {
        gTheWorld->LogBugString("Quest table not found!");
        std::string type = lua_typename(luaState_,
                                        lua_type(luaState_, -1));
        gTheWorld->LogBugString(std::string("Instead, got a: ") + type);
        lua_pop(luaState_, 1); // pop whatever we got (e.g. nil)
        return false;
    }

    // OK, now the table of quests is on the top of the stack, now
    // that we've checked its type.
    // Be sure to pop it off.
    lua_pop(luaState_, 1);

    return true;
}

bool QuestManager::save(const std::string & questFile)
{
    // Get the quest table
    lua_getglobal(luaState_, QUESTS_TABLE_NAME);

    int top = lua_gettop(luaState_);

    lua_getglobal(luaState_, "serialize");
    lua_getfield(luaState_, -1, "save");

    if (!lua_isfunction(luaState_, -1))
    {
        gTheWorld->LogBugString("ERROR: value at serialize.save is not "
                                "a function!");

        lua_pop(luaState_, 1); // pop the quest table
        return false;
    }

    // the save function is now on the top of the stack.
    lua_pushstring(luaState_, QUESTS_TABLE_NAME);
    lua_getglobal(luaState_, QUESTS_TABLE_NAME);

    int retval = lua_pcall(luaState_, 2, 1, 0);

    if (retval != 0)
    {
        gTheWorld->LogBugString(lua_tostring(luaState_, -1));
        lua_pop(luaState_, 2); // pop the error message and the quest table
        return false;
    }

    // Make sure we got a string
    if (!lua_isstring(luaState_, -1))
    {
        gTheWorld->LogBugString("ERROR: result of serialize.save was "
                                "not a string!");
        lua_pop(luaState_, 1); // pop the quest table
        return false;
    }

    // get the string value
    const char * str = lua_tostring(luaState_, -1);

    std::ofstream output(questFile.c_str());
    output << str << endl;

    // pop the string
    lua_pop(luaState_, 1);

    // pop the serialize table
    lua_pop(luaState_, 1);

    assert(top == lua_gettop(luaState_) && "changed stack size.");

    lua_pop(luaState_, 1); // pop the quest table

    return true;
}

bool QuestManager::hasQuest(const shared_str & name)
{
    // Get the quest manager table
    lua_getglobal(luaState_, QUESTS_TABLE_NAME);

    if (!findQuest(name)) {
        lua_pop(luaState_, 1); // pop the quest manager table
        return false; 
    }

    // Pop the quest and the quest manager table from the stack
    lua_pop(luaState_, 2);

    return true;
}

bool QuestManager::getQuest(const shared_str & name, Quest & quest)
{
    // Get the quest manager table
    lua_getglobal(luaState_, QUESTS_TABLE_NAME);

    if ( !findQuest(name) ) {
        lua_pop(luaState_, 1); // pop the quest manager table
        return false;
    }

    fillQuestObject(quest);

    // Pop the quest and the quest manager table from the stack
    lua_pop(luaState_, 2);

    return true;
}

Iterator<Quest> QuestManager::getQuestIterator()
{
    // Get the quest manager table
    lua_getglobal(luaState_, QUESTS_TABLE_NAME);

    // FIXME: figure out how to delete this memory!!
    std::vector<Quest> * quests = new std::vector<Quest>();

    // iterate over the quest names
    int index = lua_gettop(luaState_);

    lua_pushnil(luaState_);  /* first key */
    while (lua_next(luaState_, index) != 0)
    {
        // the quest name is at index -2;
        // and the quest object is at index -1

        Quest q;
        fillQuestObject(q);

        quests->push_back(q);

        /* removes `value'; keeps `key' for next iteration */
        lua_pop(luaState_, 1);
    }

    // pop the quest manager table
    lua_pop(luaState_, 1);

    return MakeIterator(*quests);
}

Iterator<shared_str> QuestManager::getQuestNamesIterator()
{
    // Get the quest manager table
    lua_getglobal(luaState_, QUESTS_TABLE_NAME);

    // FIXME: figure out how to delete this memory!!
    std::vector<shared_str> * names = new std::vector<shared_str>();

    // iterate over the quest names
    int index = lua_gettop(luaState_);

    lua_pushnil(luaState_);  /* first key */
    while (lua_next(luaState_, index) != 0)
    {
        // the quest name is at index -2;
        // and the quest object is at index -1

        // we don't care about the quest, so pop it
        lua_pop(luaState_, 1);

        // grab the quest name
        names->push_back(lua_tostring(luaState_, -1));

        // leave the quest name on the stack for next iterator
    }

    // pop the quest manager table
    lua_pop(luaState_, 1);
    return MakeIterator(*names);
}

bool QuestManager::interpret(const std::string & code)
{
    return Lua::Interpret(luaState_, code);
}

bool QuestManager::findQuest(const shared_str & name)
{
    lua_getfield(luaState_, -1, name.c_str());

    // make sure we got a table; report failure if not
    if ( !lua_istable(luaState_, -1) ) {
        // pop off whatever we got
        lua_pop(luaState_, 1);
        return false;
    }

    // OK, the quest table is on the stack now

    return true;
}

#define GET_STRING_FIELD(field, store_in, optional) \
    if ( !Lua::GetStringField(luaState_, field, result) ) { \
        if (!optional) { \
            gTheWorld->LogBugString("Warning: quest had no " \
                                    #field "!"); \
        } \
        result = ""; \
    } \
    else { \
        store_in = result; \
    }

#define GET_INT_FIELD(field, store_in, optional) \
    if ( !Lua::GetIntField(luaState_, field, intResult) ) { \
        if (!optional) { \
            gTheWorld->LogBugString("Warning: quest had no " \
                                    #field "!"); \
        } \
        intResult = 0; \
    } \
    else { \
        store_in = intResult; \
    }

void QuestManager::fillQuestObject(Quest & quest)
{
    std::string result;
    int intResult;

    // Get the string fields
    GET_STRING_FIELD("name", quest.name_, false);
    GET_STRING_FIELD("title", quest.title_, false);
    GET_STRING_FIELD("author", quest.author_, false);
    GET_STRING_FIELD("summary", quest.summary_, true);
    GET_STRING_FIELD("notes", quest.notes_, true);

    // Get the numeric fields
    GET_INT_FIELD("dateCreated", quest.dateCreated_, false);
    GET_INT_FIELD("dateModified", quest.dateModified_, false);

    // Get the quest steps

    lua_getfield(luaState_, -1, "steps");

    // Empty the steps vector, and prepare it to hold the new steps
    quest.steps_.clear();
    quest.steps_.reserve(lua_objlen(luaState_, -1));

    // Fill the steps
    getStepObjects(quest.steps_);
    lua_pop(luaState_, 1); // pop the steps table
}

void QuestManager::getStepObjects(std::vector<Quest::Step> & steps)
{
    // iterate over the steps array (which is on the top of the stack)
    // and grab each step
    int index = lua_gettop(luaState_);

    lua_pushnil(luaState_);  /* first key */
    while (lua_next(luaState_, index) != 0)
    {
        // the quest name is at index -2;
        // and the step table is at index -1
        
        Quest::Step step;
        step.name_ = lua_tostring(luaState_, -2);

        // now grab the description
        lua_getfield(luaState_, -1, "description");
        step.description_ = lua_tostring(luaState_, -1);
        // pop the description
        lua_pop(luaState_, 1);

        // pop the step table
        lua_pop(luaState_, 1);

        steps.push_back(step);

        // leave the step name on the stack for next iterator
    }

    // All done
}



