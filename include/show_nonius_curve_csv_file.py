#!/usr/bin/python3

import numpy as np
import matplotlib.pyplot as plt
import sys
import getopt


if len(sys.argv) < 3:
    print("Usage:")
    print("{0} <path to nonius curve csv file> <iC-MU master period count parameter code (MPC)>".format(sys.argv[0]))
    print("Example:")
    print("{0} nonius_curve.csv 4".format(sys.argv[0]))
    sys.exit(2)

nonius_curve_file_path = sys.argv[1]
masterPeriodCode = int(sys.argv[2])

output_virtual_error_curve_image_path = None
output_nonius_curve_pdf_path = None

args_optlist, args_ = getopt.getopt(sys.argv[3:], '::', ['output-nonius-curve-pdf='])
for opt, arg in args_optlist:
    if opt in ("--output-nonius-curve-pdf"):
        output_nonius_curve_pdf_path = arg


noniusCalibrationMaxAllowPhaseError = pow(2, 14 - 1 - masterPeriodCode)
noniusCalibrationMinAllowPhaseError = -pow(2, 14 - 1 - masterPeriodCode)


# nonius curve diagram
nonius_curve_data = np.genfromtxt(nonius_curve_file_path, delimiter=",", names=["phase_error", "track_offset_curve", "phase_margin", "single_turn_position", "continuous_single_turn_position"])
phase_error = nonius_curve_data['phase_error'][1:].tolist()
track_offset_curve = nonius_curve_data['track_offset_curve'][1:].tolist()
phase_margin = nonius_curve_data['phase_margin'][1:].tolist()
single_turn_position = nonius_curve_data['single_turn_position'][1:].tolist()
continuous_single_turn_position = nonius_curve_data['continuous_single_turn_position'][1:].tolist()

nonius_curve_fig, (nonius_curve_plot, nonius_curve_continuous_plot) = plt.subplots(2, 1)
nonius_curve_fig.canvas.manager.set_window_title("Nonius Curves")
nonius_curve_fig.set_size_inches((210 - 25 - 15)/25.4, 250/25.4)

nonius_curve_plot.plot(single_turn_position, phase_error, label='phase error')
nonius_curve_plot.plot(single_turn_position, track_offset_curve, label='track offset curve')
nonius_curve_plot.plot(single_turn_position, phase_margin, label='phase margin')
nonius_curve_plot.set_title("Nonius Curve")
nonius_curve_plot.legend()
nonius_curve_plot.set_xlabel("Single turn position in degree")
nonius_curve_plot.set_ylabel("Track error in resolution")
nonius_curve_plot.axhline(noniusCalibrationMaxAllowPhaseError, linewidth=1.0, ls='-', color='m')
nonius_curve_plot.axhline(noniusCalibrationMinAllowPhaseError, linewidth=1.0, ls='-', color='m')

nonius_curve_continuous_plot.plot(continuous_single_turn_position, phase_error, label='phase error')
nonius_curve_continuous_plot.plot(continuous_single_turn_position, track_offset_curve, label='track offset curve')
nonius_curve_continuous_plot.plot(continuous_single_turn_position, phase_margin, label='phase margin')
nonius_curve_continuous_plot.set_title("Continuous Nonius Curve")
nonius_curve_continuous_plot.legend()
nonius_curve_continuous_plot.set_xlabel("Single turn position in degree")
nonius_curve_continuous_plot.set_ylabel("Track error in resolution")
nonius_curve_continuous_plot.axhline(noniusCalibrationMaxAllowPhaseError, linewidth=1.0, ls='-', color='m')
nonius_curve_continuous_plot.axhline(noniusCalibrationMinAllowPhaseError, linewidth=1.0, ls='-', color='m')

nonius_curve_fig.tight_layout()
def nonius_curve_fig_on_resize(event):
    nonius_curve_fig.tight_layout()
    nonius_curve_fig.canvas.draw()
nonius_curve_fig_cid = nonius_curve_fig.canvas.mpl_connect(
    'resize_event', nonius_curve_fig_on_resize)

if output_nonius_curve_pdf_path is not None:
    nonius_curve_fig.savefig(output_nonius_curve_pdf_path, dpi=600, transparent=True, metadata={'Creator': 'iC-Haus MU_3SL example', 'Author': 'iC-Haus example', 'Title': 'Nonius Curve'})


plt.show()
