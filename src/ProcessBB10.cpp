#include "ProcessBB10.h"

std::vector<BB10Detector> ProcessBB10(std::vector<BB10Hit> BB10Hit_) {
    std::vector<BB10Detector> outputBB10_;

	Calibrations* calibrations = Calibrations::Instance();
    auto BGains = calibrations->GetBB10Calibrations();

    for(auto BB10: BB10Hit_) {
		int detector = BB10.detector;
		int strip = BB10.strip;
		int stripADC = BB10.adc;
		float energy = BGains[detector][strip].first + static_cast<float>(stripADC)*BGains[detector][strip].second;
        BB10Detector hit = {BB10.channel, BB10.detector, BB10.strip, BB10.adc, energy};
        outputBB10_.push_back(hit);
    }

    return outputBB10_;
}
