/*
 * Software and its documentation is provided by iC-Haus GmbH or contributors "AS IS" and is
 * subject to the ZVEI General Conditions for the Supply of Products and Services with iC-Haus
 * amendments and the ZVEI Software clause with iC-Haus amendments (http://www.ichaus.de/EULA).
 */

#ifndef IC_CSV_FILE_H
#define IC_CSV_FILE_H

#include "MU_3SL_defs.h"

#include <stddef.h>
#include <stdint.h>


int readMasterNoniusCSVFile(
        uint16_t** masterDataArray,
        uint16_t** noniusDataArray,
        size_t* numberOfSamples,
        const char* filepath,
        unsigned long skipLines);

int readMtSyncCSVFile(
        MU_MtSyncData** syncData,
        size_t* numberOfSamples,
        const char* filepath,
        const unsigned long skipLines);


#endif // IC_CSV_FILE_H
