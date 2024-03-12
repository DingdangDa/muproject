/*
 * Software and its documentation is provided by iC-Haus GmbH or contributors "AS IS" and is
 * subject to the ZVEI General Conditions for the Supply of Products and Services with iC-Haus
 * amendments and the ZVEI Software clause with iC-Haus amendments (http://www.ichaus.de/EULA).
 */

#include "csv_file.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


ssize_t getLine(char** lineptr, size_t* n, FILE* stream)
{
    const size_t lineBufferInitSize = 512;

    if (lineptr == NULL || n == NULL) {
        return -1;
    }
    if (stream == NULL || ferror(stream) || feof(stream)) {
        return -1;
    }

    if (*lineptr == NULL) {
        *lineptr = (char*)malloc(lineBufferInitSize);
        if (*lineptr == NULL) {
            return -1;
        }
        *n = lineBufferInitSize;
    }

    char* p = *lineptr;
    int c;
    while ((c = fgetc(stream)) != EOF) {
        if (c == '\r') {
            continue;
        }
        if (c == '\n') {
            break;
        }

        if ((p - *lineptr) > ((ssize_t)(*n) - 1)) {
            size_t nsize = *n * 2;
            if ((*lineptr = (char*)realloc(*lineptr, nsize)) == NULL) {
                return -1;
            }
            *n = nsize;
        }

        *p++ = (char)c;
    }

    *p++ = 0;
    return p - *lineptr - 1;
}



int readMasterNoniusCSVFile(
        uint16_t** masterDataArray,
        uint16_t** noniusDataArray,
        size_t* numberOfSamples,
        const char* filepath,
        const unsigned long skipLines)
{
    FILE* rawdataFile = fopen(filepath, "r");
    if (rawdataFile == NULL) {
        fprintf(stderr, "Error: Unable to open data file: \"%s\"\n", filepath);
        return -1;
    }

    *numberOfSamples = 0;
    size_t lineCap   = 1024;
    char* line       = (char*)malloc(lineCap + sizeof(char));
    ssize_t lineLen;
    size_t numberOfValidLines = 0;
    size_t lineNumber         = 0;
    while ((lineLen = getLine(&line, &lineCap, rawdataFile)) >= 0) {
        ++lineNumber;
        if (lineNumber <= skipLines) {
            continue;
        }
        int masterData;
        int noniusData;
        if (sscanf(line, "%d,%d", &masterData, &noniusData) > 0) {
            ++numberOfValidLines;
        }
        if (feof(rawdataFile) || ferror(rawdataFile)) {
            break;
        }
    }

    rewind(rawdataFile);
    fseek(rawdataFile, 0, SEEK_SET);
    fseek(rawdataFile, 0, SEEK_CUR);
    lineNumber = 0;

    *masterDataArray = (uint16_t*)malloc(numberOfValidLines * sizeof(uint16_t));
    *noniusDataArray = (uint16_t*)malloc(numberOfValidLines * sizeof(uint16_t));

    while ((lineLen = getLine(&line, &lineCap, rawdataFile)) >= 0) {
        ++lineNumber;
        if (lineNumber <= skipLines) {
            continue;
        }
        int masterData;
        int noniusData;
        if (sscanf(line, "%d,%d", &masterData, &noniusData) > 0) {
            (*masterDataArray)[*numberOfSamples] = masterData;
            (*noniusDataArray)[*numberOfSamples] = noniusData;
            ++(*numberOfSamples);
        } else if (strlen(line) > 0) {
            fprintf(stderr, "Error read file in line %lu\n", lineNumber);
        }
        if (feof(rawdataFile) || ferror(rawdataFile)) {
            break;
        }
    }
    free(line);

    fclose(rawdataFile);
    return (int)(*numberOfSamples);
}


int readMtSyncCSVFile(
        MU_MtSyncData** syncData,
        size_t* numberOfSamples,
        const char* filepath,
        const unsigned long skipLines)
{
    FILE* rawdataFile = fopen(filepath, "r");
    if (rawdataFile == NULL) {
        fprintf(stderr, "Error: Unable to open data file: \"%s\"\n", filepath);
        return -1;
    }

    *numberOfSamples = 0;
    size_t lineCap   = 1024;
    char* line       = (char*)malloc(lineCap + sizeof(char));
    ssize_t lineLen;
    size_t numberOfValidLines = 0;
    size_t lineNumber         = 0;
    while ((lineLen = getLine(&line, &lineCap, rawdataFile)) >= 0) {
        ++lineNumber;
        if (lineNumber <= skipLines) {
            continue;
        }
        uint32_t singleTurnPosition;
        uint32_t internalSingleTurnPosition;
        uint32_t normalizedExternalMultiTurnSyncBits;
        uint32_t normalizedInternalMultiTurnSyncBits;
        uint32_t externalMultiTurnPosition;
        uint32_t multiTurnPosition;
        if (sscanf(line,
                   "%d,%d,%d,%d,%d,%d",
                   &singleTurnPosition,
                   &internalSingleTurnPosition,
                   &normalizedExternalMultiTurnSyncBits,
                   &normalizedInternalMultiTurnSyncBits,
                   &externalMultiTurnPosition,
                   &multiTurnPosition)
            > 0) {
            ++numberOfValidLines;
        }
        if (feof(rawdataFile) || ferror(rawdataFile)) {
            break;
        }
    }

    rewind(rawdataFile);
    fseek(rawdataFile, 0, SEEK_SET);
    fseek(rawdataFile, 0, SEEK_CUR);
    lineNumber = 0;

    *syncData = (MU_MtSyncData*)malloc(numberOfValidLines * sizeof(MU_MtSyncData));

    while ((lineLen = getLine(&line, &lineCap, rawdataFile)) >= 0) {
        ++lineNumber;
        if (lineNumber <= skipLines) {
            continue;
        }
        uint32_t singleTurnPosition;
        uint32_t internalSingleTurnPosition;
        uint32_t normalizedExternalMultiTurnSyncBits;
        uint32_t normalizedInternalMultiTurnSyncBits;
        uint32_t externalMultiTurnPosition;
        uint32_t multiTurnPosition;
        if (sscanf(line,
                   "%d,%d,%d,%d,%d,%d",
                   &singleTurnPosition,
                   &internalSingleTurnPosition,
                   &normalizedExternalMultiTurnSyncBits,
                   &normalizedInternalMultiTurnSyncBits,
                   &externalMultiTurnPosition,
                   &multiTurnPosition)
            > 0) {
            (*syncData)[*numberOfSamples].singleTurnPosition         = singleTurnPosition;
            (*syncData)[*numberOfSamples].internalSingleTurnPosition = internalSingleTurnPosition;
            (*syncData)[*numberOfSamples].normalizedExternalMultiTurnSyncBits =
                    normalizedExternalMultiTurnSyncBits;
            (*syncData)[*numberOfSamples].normalizedInternalMultiTurnSyncBits =
                    normalizedInternalMultiTurnSyncBits;
            (*syncData)[*numberOfSamples].externalMultiTurnPosition = externalMultiTurnPosition;
            (*syncData)[*numberOfSamples].multiTurnPosition         = multiTurnPosition;
            ++(*numberOfSamples);
        } else if (strlen(line) > 0) {
            fprintf(stderr, "Error read file in line %lu\n", lineNumber);
        }
        if (feof(rawdataFile) || ferror(rawdataFile)) {
            break;
        }
    }
    free(line);

    fclose(rawdataFile);
    return (int)(*numberOfSamples);
}
