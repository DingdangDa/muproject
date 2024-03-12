/*
 * Software and its documentation is provided by iC-Haus GmbH or contributors "AS IS" and is
 * subject to the ZVEI General Conditions for the Supply of Products and Services with iC-Haus
 * amendments and the ZVEI Software clause with iC-Haus amendments (http://www.ichaus.de/EULA).
 */

#include "MU_3SL_defs.h"
#include "MU_3SL_interface.h"

#include "csv_file.h"
#include "mu_3sl_calibration_adjustments.h"
#include "mu_3sl_nonius_curve.h"

#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>


int main(int argc, char** argv)
{
    const char* filepath = argc <= 1 ? "measure/raw_sample_data_MU_Y_MU2L_82-32N_0.csv" : argv[1];
    const uint32_t masterPeriodCode = argc <= 2 ? 5 : strtol(argv[2], NULL, 0);
    const uint8_t revisionCode      = argc <= 3 ? MU_REV_MU_Y2 : strtol(argv[3], NULL, 0);

    const char* noniusCurveCsvFilePath = argc <= 4 ? "nonius_curve.csv" : argv[4];

    const bool enableMasterPeriodAutoDetection = false;
    const unsigned int numberOfMasterPeriods   = 1u << masterPeriodCode;

    printf("Library version: %s%s\n\n", MU_GetVersionString(), MU_GetVersionSuffixString());

    size_t numberOfSamples  = 0;
    uint16_t* masterRawData = NULL;
    uint16_t* noniusRawData = NULL;
    printf("Read master and nonius track raw data from file: \"%s\"\n\n", filepath);
    if (readMasterNoniusCSVFile(&masterRawData, &noniusRawData, &numberOfSamples, filepath, 1) < 0) {
        fprintf(stderr, "Error: Unable to read data file!\n");
        return EXIT_FAILURE;
    }

    MU_Calibration* calibration = MU_createCalibration(revisionCode);

    if (!enableMasterPeriodAutoDetection) {
        MU_Calibration_preconfigureNumberOfMasterPeriods(calibration, numberOfMasterPeriods);
    }

    MU_Calibration_AnalogTrackAdjustments initialMasterAdjustments = {0, 0, 0, 0, 0};
    MU_Calibration_AnalogTrackAdjustments initialNoniusAdjustments = {0, 0, 0, 0, 0};
    MU_Calibration_setCurrentAnalogTrackAdjustments(
            calibration, &initialMasterAdjustments, &initialNoniusAdjustments);

    printf("Initial iC-MU signal conditioning parameters:\n");
    printAnalogAdjustments(calibration);
    printf("\n\n");

    MU_CalibrationAnalyzeResult* analyzeResult = MU_Calibration_analyzeRawData(
            calibration, masterRawData, noniusRawData, numberOfSamples);
    free(masterRawData);
    free(noniusRawData);

    if (analyzeResult == NULL) {
        fprintf(stderr, "Raw data are not analyzable\n");
        MU_Calibration_delete(calibration);
        return EXIT_FAILURE;
    }

    printAnalyzeResultLog(analyzeResult);

    printf("Relative track adjustments (relative changes in \"LSB\")\n");
    printRelativeAdjustments(analyzeResult);

    printAnalogAnalyzeResultAdjustableLog(calibration, analyzeResult);

    MU_Calibration_adjustAnalogByAnalyzeResult(calibration, analyzeResult);

    printf("iC-MU signal conditioning parameters after calibration:\n");
    printAnalogAdjustments(calibration);
    printf("\n\n");

    int numberOfCalculatedMasterPeriods =
            MU_Calibration_numberOfCalculatedMasterPeriods(analyzeResult);
    uint32_t calculatedMasterPeriodCode = (uint32_t)round(log2(numberOfCalculatedMasterPeriods));
    optionalPrintOptimizedNoniusTrackOffsetTable(
            analyzeResult,
            enableMasterPeriodAutoDetection ? calculatedMasterPeriodCode : masterPeriodCode,
            noniusCurveCsvFilePath);

    MU_CalibrationAnalyzeResult_delete(analyzeResult);
    MU_Calibration_delete(calibration);

    return 0;
}
