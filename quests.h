
#ifndef QUESTS_H_
#define QUESTS_H_

// Ksilyan, 2006-01-28

#include <vector>

#include "shared_str.h"
#include "java_iterator.hpp"

extern "C" {
    #include <lua.h>
}


// Forward declaration
class QuestManager;

// TODO: add:
//   - list of areas

/* Class: Quest
 *
 * A quest definition has the following components:
 *
 * - Name: unique string identifying this quest. (for builder identification)
 * - Title: one-line name of the quest (for display to players)
 * - Author: one-line string for who wrote the quest
 * - Summary: one-line summary of the quest. Meant mainly for players.
 *            Optional.
 * - Notes: notes about the quest, typically set by builders. Used to write
 *          comments about quests; for instance, what mobs are involved in
 *          the quest. Optional. Multi-line.
 * - Steps: a table of steps, keyed by their name, each defining:
 *     - description: Description of what the step's goals are (for display
 *                    to players). Multi-line.
 *
 * Additionally, the following properties exist but are not user-editable:
 * - date_created: The date the quest was created (sec. since Epoch)
 * - date_modified: The date the quest was last modified (sec. since Epoch)
 *
 * The following step names are reserved:
 * - success: the player completed the quest successfully
 * - failure: the player failed the quest
 * - terminal: the player ended the quest with a "neutral" outcome (e.g.,
 *             aborted the quest)
 *
 * Step names "success" and "failure" are reserved. They mean, respectively,
 * that the quest was completed succesfully, and that the player failed the
 * quest. There is no need for quest builders to create or define these steps.
 * It is up to the builder to transition to these states. It is also up to the
 * builder to decide if one can restart the quest from either of these states.
 */

class Quest
{
    public:
        struct Step
        {
            shared_str name_;
            shared_str description_;
        };

        // Default constructor
        Quest() { /* empty */ }

        // Constructor
        Quest(const shared_str & name, const shared_str & title,
              const shared_str & author, const std::vector<Step> & steps,
              time_t dateCreated, time_t dateModified);


        void setName(const shared_str & name) { name_ = name; }
        const shared_str & getName() const { return name_; }

        void setTitle(const shared_str & title) { title_ = title; }
        const shared_str & getTitle() const { return title_; }

        void setAuthor(const shared_str & author) { author_ = author; }
        const shared_str & getAuthor() const { return author_; }

        void setSummary(const shared_str & summary) { summary_=summary; }
        const shared_str & getSummary() const { return summary_; }

        void setNotes(const shared_str & notes) { notes_ = notes; }
        const shared_str & getNotes() const { return notes_; }

        void setSteps(const std::vector<Step> & steps) { steps_ = steps; }
        const std::vector<Step> & getSteps() const { return steps_; }

        void setDateCreated(time_t t) { dateCreated_ = t; }
        time_t getDateCreated() const { return dateCreated_; }
        
        void setDateModified(time_t t) { dateModified_ = t; }
        time_t getDateModified() const { return dateModified_; }

        /* Method: getStep
         *
         * Check a quest for a step by name; return that step if found,
         * or null if not found.
         *
         * Parameters:
         *   name - The name of the step to look up.
         *
         * Returns:
         *   A pointer to the step if found, or NULL if not found.
         */
        const Step * getStep(const shared_str & name);

        friend class QuestManager;

    protected:
        shared_str name_;
        shared_str title_;
        shared_str author_;
        shared_str summary_;
        shared_str notes_;

        std::vector<Step> steps_;

        time_t dateCreated_;
        time_t dateModified_;
};

/* Class: QuestManager
 *
 * Manages the game's quest definitions.
 *
 *
 * Implementation note: the Lua state is owned by the World. We should not
 * leave stuff lying around on the stack, assuming it will be there the
 * next time we enter a public method. All entry points should push the
 * quest manager table, then pop it when they're done.
 */
class QuestManager
{
    public:
        /* Constructor: QuestManager
         *
         * Initializes Lua state.
         */
        QuestManager();

        /* Destructor: ~QuestManager
         *
         * Closes Lua state.
         */
        ~QuestManager();

        /* Method: initialize
         *
         * Initialize quest database from a Lua quest file. If
         * successful, pushes the quest table onto the stack.
         *
         * Parameters:
         *  questFile - path to the file to read from
         *
         * Returns:
         *  True if successful, false on failure.
         */
        bool initialize(const std::string & questFile);

        /* Method: save
         *
         * Save the quest table to the specified file.
         *
         * Parameters:
         *  questFile - path to the file to save
         *
         * Returns:
         *  True if successful, false on failure.
         */
        bool save(const std::string & questFile);

        /* Method: hasQuest
         *
         * Returns true if a quest exists.
         *
         * Parameters:
         *  name - The name of the quest to check for.
         *
         * Returns:
         *  True if the quest exists; false otherwise.
         */
        bool hasQuest(const shared_str & name);

        /* Method: getQuest
         *
         * Get a quest's data by name.
         *
         * Parameters:
         *  name - The name of the quest to look up.
         *  quest - The quest object to fill with the information, if
         *          the quest exists.
         *
         * Returns:
         *
         *  True if the quest was found, and false if it doesn't exist.
         */
        bool getQuest(const shared_str & name, Quest & quest);

        /* Method: getQuestIterator
         *
         * Construct an iterator for the quests.
         *
         * Returns:
         *
         *  An iterator ranging over defined quests.
         */
        Iterator<Quest> getQuestIterator();

        /* Method: getQuestNamesIterator
         *
         * Construct an iterator for the quest names.
         *
         * Returns:
         *
         *  An iterator ranging over the quest names.
         */
        Iterator<shared_str> getQuestNamesIterator();

        /* Method: interpret
         *
         * Interpret an arbitrary Lua string in the quest environment.
         *
         * Parameters:
         *  code - The Lua code to interpret.
         *
         * Returns:
         *  True if the interpretation succeeded (i.e. no error was
         *  returned); false otherwise.
         */
        bool interpret(const std::string & code);

        static const char * QUESTS_TABLE_NAME;

    protected:

        // Variable: luaState_
        //
        // The Lua state global to the world.
        //
        // NOTE:
        // We do not own this pointer!!
        lua_State * luaState_;

        /* Method: findQuest
         *
         * Look up a quest in the global quest table.
         *
         * If the quest is found, its table is put onto the stack. If
         * the quest isn't found, nothing happens, and the function 
         * returns false.
         *
         * Note:
         *  This function assumes that the quest table is already on the
         *  top of the stack.
         *
         * Parameters:
         *  name - The name of the quest to look up.
         *
         * Returns:
         *  True if the quest was found; false otherwise.
         */
        bool findQuest(const shared_str & name);

        /* Method: fillQuestObject
         *
         * Fills up a quest object from a quest table.
         *
         * Assumes that the quest table is on the top of the stack 
         * already.
         *
         * Parameters:
         *  quest - The quest object (by reference) to fill.
         */
        void fillQuestObject(Quest & quest);

        /* Method: getStepObjects
         *
         * Take a Lua table (array) of steps, and convert to a vector.
         *
         * Assumes that the steps table is on the top of the stack 
         * already.
         */
        void getStepObjects(std::vector<Quest::Step> & steps);
};

#endif // include guard

