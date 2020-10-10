#ifndef PEH_TABLE_H
#define PEH_TABLE_H

#include "table.h"

namespace polaris
{
    //!  A custom Table holding an ProcessEventHook.
    /*!
    * A PehTable is a singleton table in Program, which holds the ProcessEventHook a PehTable can subscribes to.
    */
    class PehTable : public Table
    {
    public:
        PehTable();
    };
}

extern polaris::PehTable* gpPehTable;

#endif // !PEH_TABLE_H
