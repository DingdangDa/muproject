/*
* Software and its documentation is provided by iC-Haus GmbH or contributors "AS IS" and is
* subject to the ZVEI General Conditions for the Supply of Products and Services with iC-Haus
* amendments and the ZVEI Software clause with iC-Haus amendments (http://www.ichaus.de/EULA).
 */

#include "mu_3sl_nonius_curve.h"

#include "MU_3SL_defs.h"
#include "MU_3SL_interface.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


void optionalPrintOptimizedNoniusTrackOffsetTable(
        const MU_CalibrationAnalyzeResult* analyzeResult,
        const uint32_t masterPeriodCode,
        const char* noniusCurveCsvFilePath)
{
    MU_Calibration_NoniusTrackOffsetTable noniusTrackOffsetTable;
    MU_Calibration_getOptimizedNoniusTrackOffsetTable(analyzeResult, &noniusTrackOffsetTable);

    printf("Optimized nonius track offset table:\n");
    printf("SPO_BASE: %2d\n", noniusTrackOffsetTable.spoBase);
    for (int i = 0; i < 16; ++i) {
        if (i >= 10) {
            printf("SPO_%d:   %2d\n", i, noniusTrackOffsetTable.spoN[i]);
        } else {
            printf("SPO_%d:    %2d\n", i, noniusTrackOffsetTable.spoN[i]);
        }
    }
    printf("\n");

    long noniusCalibrationMaxAllowPhaseError = 1u << (14 - 1 - masterPeriodCode);
    long noniusPhaseMarginMax                = MU_Calibration_noniusPhaseMarginMax(analyzeResult);
    long noniusPhaseMarginMin                = MU_Calibration_noniusPhaseMarginMin(analyzeResult);
    double noniusPhaseMarginMax_percent =
            (double)noniusPhaseMarginMax / (double)noniusCalibrationMaxAllowPhaseError * 100;
    double noniusPhaseMarginMin_percent =
            (double)noniusPhaseMarginMin / (double)(-noniusCalibrationMaxAllowPhaseError) * 100;
    printf("Nonius phase margin: Max.: %ld (%.3f%%) Min.: %ld (%.3f%%)\n",
           noniusPhaseMarginMax,
           noniusPhaseMarginMax_percent,
           noniusPhaseMarginMin,
           noniusPhaseMarginMin_percent);

    printf("Nonius phase range limit: %ld\n", MU_Calibration_noniusPhaseRangeLimit(analyzeResult));

    printf("Nonius upper phase margin: %f%%\n",
           MU_Calibration_noniusUpperPhaseMargin(analyzeResult));
    printf("Nonius lower phase margin: %f%%\n",
           MU_Calibration_noniusLowerPhaseMargin(analyzeResult));


    // Copy nonius analyzed curve data to have it after delete analyzeResult

    size_t numberOfNoniusCurveSamples =
            MU_Calibration_numberOfNoniusCurveSamples(analyzeResult);
    const long* noniusPhaseErrorData = MU_Calibration_noniusPhaseError(analyzeResult);
    long* noniusPhaseError = (long*)malloc(numberOfNoniusCurveSamples * sizeof(long));
    memcpy(noniusPhaseError, noniusPhaseErrorData, numberOfNoniusCurveSamples * sizeof(long));

    const long* noniusTrackOffsetCurveData = MU_Calibration_noniusTrackOffsetCurve(analyzeResult);
    long* noniusTrackOffsetCurve = (long*)malloc(numberOfNoniusCurveSamples * sizeof(long));
    memcpy(noniusTrackOffsetCurve,
           noniusTrackOffsetCurveData,
           numberOfNoniusCurveSamples * sizeof(long));

    const long* noniusPhaseMarginData = MU_Calibration_noniusPhaseMargin(analyzeResult);
    long* noniusPhaseMargin = (long*)malloc(numberOfNoniusCurveSamples * sizeof(long));
    memcpy(noniusPhaseMargin,
           noniusPhaseMarginData,
           numberOfNoniusCurveSamples * sizeof(long));

    const size_t numberOfSamples = numberOfNoniusCurveSamples;
    double* noniusPosition = (double*)malloc(numberOfSamples * sizeof(double));
    MU_Calibration_calculateNoniusPosition(
            analyzeResult,
            noniusPosition,
            numberOfSamples,
            MU_CALIBRATION_UNIT_DEGREE,
            false);
    double* continuousNoniusPosition = (double*)malloc(numberOfSamples * sizeof(double));
    MU_Calibration_calculateNoniusPosition(
            analyzeResult,
            continuousNoniusPosition,
            numberOfSamples,
            MU_CALIBRATION_UNIT_DEGREE,
            true);

    FILE* csvFile = NULL;
    if (noniusCurveCsvFilePath != NULL && strcmp(noniusCurveCsvFilePath, "") != 0) {
        csvFile = fopen(noniusCurveCsvFilePath, "w");
    }
    if (csvFile) {
        fprintf(csvFile,
                "phase error in absolute resolution,"
                "track offset curve in absolute resolution,"
                "phase margin in absolute resolution,"
                "single turn position in degree,"
                "continuous single turn position in degree\n");
        for (size_t i = 0; i < numberOfSamples; ++i) {
            fprintf(csvFile,
                    "%ld,%ld,%ld,%f,%f\n",
                    noniusPhaseError[i],
                    noniusTrackOffsetCurve[i],
                    noniusPhaseMargin[i],
                    noniusPosition[i],
                    continuousNoniusPosition[i]);
        }
        fclose(csvFile);
    } else {
        fprintf(stderr,
                "Error: Can't create and/or open CSV file \"%s\"\n",
                noniusCurveCsvFilePath);
    }

    free(noniusPhaseError);
    free(noniusTrackOffsetCurve);
    free(noniusPhaseMargin);
    free(noniusPosition);
    free(continuousNoniusPosition);
}

// HXY
void PrintOptimizedNoniusTrackOffsetTableForCopy(const MU_CalibrationAnalyzeResult* analyzeResult)
{
    MU_Calibration_NoniusTrackOffsetTable noniusTrackOffsetTable;
    MU_Calibration_getOptimizedNoniusTrackOffsetTable(analyzeResult, &noniusTrackOffsetTable);
    printf("SPOForCopy: %2d, ", noniusTrackOffsetTable.spoBase);
    for (int i = 0; i < 14; i++) {
        printf("%2d, ", noniusTrackOffsetTable.spoN[i]);
    }
    printf("%2d\n", noniusTrackOffsetTable.spoN[14]);
    int sum_spo_0_14 = 0;
    for (int i = 0; i<15; i++) {
        sum_spo_0_14 = sum_spo_0_14 + noniusTrackOffsetTable.spoN[i];
    }
    printf("Sum of SPO_0 to SPO_14: %d (valid -7 ~ 7)\n", sum_spo_0_14);
}
