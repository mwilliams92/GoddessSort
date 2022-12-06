#ifndef Calibrations_h
#define Calibrations_h

#include "json/json.h"
#include "TypeDef.h"
#include "Utilities.h"

#include <cassert>
#include <fstream>
#include <map>
#include <utility>

class Calibrations {
public:
    Calibrations();
    static Calibrations* Instance();

    std::map<int, std::map<int, std::pair<float, float> > > GetQQQ5DownCalibrations() {return fQQQ5DownCalibrations;}
    std::map<int, std::map<int, std::pair<float, float> > > GetQQQ5UpCalibrations() {return fQQQ5UpCalibrations;}
    std::map<int, float> GetQQQ5Angles() {return fQQQ5Angles;}

    std::map<int, std::map<int, std::map<int, std::pair<float, float> > > > GetSuperX3UpCalibrations() {return fSuperX3UpCalibrations;}
    std::map<int, std::map<int, float> > GetSuperX3UpPedestals() {return fSuperX3UpPedestals;}
	std::map<int, std::map<int, std::map<int, std::pair<float, float> > > > GetSuperX3DownCalibrations() {return fSuperX3DownCalibrations;}
    std::map<int, std::map<int, float> > GetSuperX3DownPedestals() {return fSuperX3DownPedestals;}

	std::map<int, std::map<int, std::pair<float, float> > > GetBB10Calibrations() {return fBB10Calibrations;}
    std::map<int, float> GetBB10Pedestals() {return fBB10Pedestals;}

    int GetBB10Threshold() {return BB10Threshold;}
    int GetQQQThreshold() {return QQQThreshold;}
    int GetSX3Threshold() {return SX3Threshold;}
    int GetICTrackingThreshold() {return ICTrackingThreshold;}

private:
    static Calibrations* fInstance;

    void ReadQQQ5DownCalibrations();
    void ReadQQQ5DownPedestals();
    void ReadQQQ5UpCalibrations();
    void ReadQQQ5UpPedestals();
    void ReadQQQ5Angles();

    void ReadSuperX3UpCalibrations();
    void ReadSuperX3UpPedestals();
	void ReadSuperX3DownCalibrations();
    void ReadSuperX3DownPedestals();

	void ReadBB10Calibrations();
    void ReadBB10Pedestals();

    int BB10Threshold;
    int QQQThreshold;
    int SX3Threshold;
    int ICTrackingThreshold;

    std::map<int, std::map<int, std::pair<float, float> > > fQQQ5DownCalibrations;
    std::map<int, float> fQQQ5DownPedestals;
    std::map<int, std::map<int, std::pair<float, float> > > fQQQ5UpCalibrations;
    std::map<int, float> fQQQ5UpPedestals;

    std::map<int, float> fQQQ5Angles;

    std::map<int, std::map<int, std::map<int, std::pair<float, float> > > > fSuperX3UpCalibrations;
    std::map<int, std::map<int, float> > fSuperX3UpPedestals;
	std::map<int, std::map<int, std::map<int, std::pair<float, float> > > > fSuperX3DownCalibrations;
    std::map<int, std::map<int, float> > fSuperX3DownPedestals;

	std::map<int, std::map<int, std::pair<float, float> > > fBB10Calibrations;
    std::map<int, float> fBB10Pedestals;
};

#endif // Calibrations_h
