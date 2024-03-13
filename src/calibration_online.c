/*
 * Software and its documentation is provided by iC-Haus GmbH or contributors "AS IS" and is
 * subject to the ZVEI General Conditions for the Supply of Products and Services with iC-Haus
 * amendments and the ZVEI Software clause with iC-Haus amendments (http://www.ichaus.de/EULA).
 */

#include "mu_3sl_calibration_adjustments.h"
#include "mu_3sl_check_error_return.h"
#include "MU_3SL_defs.h"
#include "MU_3SL_interface.h"
#include "mu_3sl_nonius_curve.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#if defined(_WIN32) || defined(__WIN32__)
    #include <windows.h>
#elif defined(linux) || defined(__linux) || defined(__APPLE__)
    #include <unistd.h>
#endif


inline void sleepMs(unsigned int ms)
{
#if defined(_WIN32) || defined(__WIN32__)
    Sleep(ms);
#elif defined(linux) || defined(__linux) || defined(__APPLE__)
    usleep(ms * 1000);
#endif
}


int main(int argc, char** argv)
{
    const unsigned long numberOfSamples   = 10000; // min. 16 (recommended 128) values per period
    const double acquireFrameCycleTime_s  = 1.0 / 8E3;
    const double acquireClockFrequency_hz = 2E6;

    const char* noniusCurveCsvFilePath = argc <= 1 ? "nonius_curve.csv" : argv[1];

    uint16_t* masterRawData = malloc(numberOfSamples * sizeof(uint16_t));
    uint16_t* noniusRawData = malloc(numberOfSamples * sizeof(uint16_t));

    // Get the version of the library
    printf("Library version: %s%s\n\n", MU_GetVersionString(), MU_GetVersionSuffixString());

    // Construct a new instance of a virtual chip object and obtain a handle for it.
    MU_Handle muHandle;
    CHECK(MU_Open(&muHandle));

    // Connect the interface (USB-BiSS adapter) MB5U
    CHECK(MU_SetInterface(muHandle, MU_MB5U, ""));

    uint8_t revisionCode = MU_REV_MU_Y2;
    CHECK(MU_ReadChipRevision(muHandle, &revisionCode));
    CHECK(MU_UseRevision(muHandle, revisionCode));

    // Read all configuration parameters from the connected iC-MU* in the virtual chip object
    CHECK(MU_ReadParams(muHandle));

    MU_Calibration* calibration = MU_getCalibration(muHandle);

    uint32_t masterPeriodCode;
    CHECK(MU_GetParam(muHandle, MU_MPC, NULL, &masterPeriodCode));


    printf("\n"
           "---------------------------------------------------------------\n"
           "- Reset current analog track adjustments (loaded from EEPROM) -\n"
           "---------------------------------------------------------------\n");

    MU_Calibration_AnalogTrackAdjustments initialMasterAdjustments = {0, 0, 0, 0, 0};
    MU_Calibration_AnalogTrackAdjustments initialNoniusAdjustments = {0, 0, 0, 0, 0};
    MU_Calibration_setCurrentAnalogTrackAdjustments(
            calibration, &initialMasterAdjustments, &initialNoniusAdjustments);
    CHECK(MU_setCalibration(muHandle, calibration));
    CHECK(MU_WriteParams(muHandle, false, NULL));


    printf("Initial iC-MU signal conditioning parameters:\n");
    printAnalogAdjustments(calibration);
    printf("\n\n");


    printf("\n"
           "---------------------------------------------------------------\n"
           "----------------- Analog Calibration of iC-MU -----------------\n"
           "---------------------------------------------------------------\n");

    CHECK(MU_activateCalibrationConfig(muHandle));
    if (MU_acquireRawData(
                muHandle,
                masterRawData,
                noniusRawData,
                numberOfSamples,
                0,
                acquireFrameCycleTime_s,
                acquireClockFrequency_hz)) {
        MU_Error lastError;
        MU_ErrorType errorType;
        char errorText[1024];
        MU_GetLastError(muHandle, &lastError, &errorType, errorText);
        fprintf(stderr, "MU_acquireRawData error: %s\n", errorText);

        MU_deactivateCalibrationConfig(muHandle);
        free(masterRawData);
        free(noniusRawData);
        MU_Calibration_delete(calibration);
        MU_Close(muHandle);
        exit(EXIT_FAILURE);
    }
    CHECK(MU_deactivateCalibrationConfig(muHandle));

    MU_CalibrationAnalyzeResult* analyzeResult = MU_Calibration_analyzeRawData(
            calibration, masterRawData, noniusRawData, numberOfSamples);

    if (analyzeResult == NULL) {
        fprintf(stderr, "Raw data are not analyzable\n");
        MU_deactivateCalibrationConfig(muHandle);
        free(masterRawData);
        free(noniusRawData);
        MU_Calibration_delete(calibration);
        MU_Close(muHandle);
        exit(EXIT_FAILURE);
    }

    printAnalyzeResultLog(analyzeResult);

    printf("Relative track adjustments (relative changes in \"LSB\")\n");
    printRelativeAdjustments(analyzeResult);

    printAnalogAnalyzeResultAdjustableLog(calibration, analyzeResult);

    MU_Calibration_adjustAnalogByAnalyzeResult(calibration, analyzeResult);

    printf("iC-MU signal conditioning parameters after calibration:\n");
    printAnalogAdjustments(calibration);

    MU_setCalibration(muHandle, calibration);
    CHECK(MU_WriteParams(muHandle, false, NULL));

    MU_CalibrationAnalyzeResult_delete(analyzeResult);


    printf("\n"
           "---------------------------------------------------------------\n"
           "----------------- Nonius Calibration of iC-MU -----------------\n"
           "---------------------------------------------------------------\n");

    CHECK(MU_activateCalibrationConfig(muHandle));
    if (MU_acquireRawData(
                muHandle,
                masterRawData,
                noniusRawData,
                numberOfSamples,
                0,
                acquireFrameCycleTime_s,
                acquireClockFrequency_hz)) {
        MU_Error lastError;
        MU_ErrorType errorType;
        char errorText[1024];
        MU_GetLastError(muHandle, &lastError, &errorType, errorText);
        fprintf(stderr, "MU_acquireRawData error: %s\n", errorText);

        MU_deactivateCalibrationConfig(muHandle);
        free(masterRawData);
        free(noniusRawData);
        MU_Calibration_delete(calibration);
        MU_Close(muHandle);
        return EXIT_FAILURE;
    }
    CHECK(MU_deactivateCalibrationConfig(muHandle));

    analyzeResult = MU_Calibration_analyzeRawData(
            calibration, masterRawData, noniusRawData, numberOfSamples);

    if (analyzeResult == NULL) {
        fprintf(stderr, "Raw data are not analyzable\n");
        MU_deactivateCalibrationConfig(muHandle);
        free(masterRawData);
        free(noniusRawData);
        MU_Calibration_delete(calibration);
        MU_Close(muHandle);
        exit(EXIT_FAILURE);
    }

    printAnalyzeResultLog(analyzeResult);

    printf("Residual errors (relative changes in \"LSB\")\n");
    printRelativeAdjustments(analyzeResult);

    optionalPrintOptimizedNoniusTrackOffsetTable(
            analyzeResult, masterPeriodCode, noniusCurveCsvFilePath);

    MU_Calibration_NoniusTrackOffsetTable optimizedNoniusTrackOffsetTable;
    MU_Calibration_getOptimizedNoniusTrackOffsetTable(
            analyzeResult, &optimizedNoniusTrackOffsetTable);
    MU_Calibration_setCurrentNoniusTrackOffsetTable(calibration, &optimizedNoniusTrackOffsetTable);

    MU_setCalibration(muHandle, calibration);
    CHECK(MU_WriteParams(muHandle, false, NULL));

    CHECK(MU_WriteCmdRegister(muHandle, MU_CMD_CRC_CALC));
    CHECK(MU_WriteCmdRegister(muHandle, MU_CMD_ABS_RESET));

    MU_CalibrationAnalyzeResult_delete(analyzeResult);
    MU_Calibration_delete(calibration);


    printf("\n"
           "---------------------------------------------------------------\n"
           "--------------- Send WRITE_ALL Command to iC-MU ---------------\n"
           "---------------------------------------------------------------\n");

    CHECK(MU_WriteCmdRegister(muHandle, MU_CMD_WRITE_ALL));


    free(masterRawData);
    free(noniusRawData);
    MU_Close(muHandle);

    return 0;
}
