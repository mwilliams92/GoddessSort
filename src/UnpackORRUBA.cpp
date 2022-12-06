#include "UnpackORRUBA.h"

#define BUFFER_LENGTH 8194
#define BUFFER_LENGTHB 32776

UnpackORRUBA::UnpackORRUBA(fileListStruct run) {
    int startClock = clock();
    std::cout << PrintOutput("\tRunning ORRUBA sort:", "yellow") << std::endl;

    std::string DataFileName, RootFileName;

    Calibrations* calibrations = Calibrations::Instance();
    int BB10Threshold = calibrations->GetBB10Threshold();
    int QQQThreshold = calibrations->GetQQQThreshold();
    int SX3Threshold = calibrations->GetSX3Threshold();
    int ICTrackingThreshold = calibrations->GetICTrackingThreshold();

    std::cout << PrintOutput("\t\tBegin data processing loop", "yellow") << std::endl;

    // Open the file. Check whether file opened successfully
    std::ifstream file(run.ldfPath.c_str(), std::ios::binary);
    ASSERT_WITH_MESSAGE(file.is_open(), Form("File not found: %s", run.ldfPath.c_str()));

    //Create and Open Root file to store data in. Check for success.
    TFile *outputFile = new TFile(run.rootPath.c_str(), "recreate");
    ASSERT_WITH_MESSAGE(outputFile->IsOpen(), Form("Root output file did not open: %s", run.rootPath.c_str()));

    // std::cout << PrintOutput(Form("Unpacking data file [%d/%ld]", ++numRuns, fileList.size()), "green") << std::endl;
    std::cout << PrintOutput("\t\tReading .ldf file: ", "cyan") << run.ldfPath << std::endl;

    //Setup Tree
    tree = new TTree("data", "Data Tree");

    // General variables
    tree->Branch("RunNumber", &fRunNumber);

    // BB10 Detectors
    tree->Branch("BB10Mul",     &fBB10Mul,     "BB10Mul/I");
    tree->Branch("BB10Det",     &fBB10Det,     "BB10Det[BB10Mul]/I");
    tree->Branch("BB10Strip",   &fBB10Strip,   "BB10Strip[BB10Mul]/I");
    tree->Branch("BB10Channel", &fBB10Channel, "BB10Channel[BB10Mul]/I");
    tree->Branch("BB10ADC",     &fBB10ADC,     "BB10ADC[BB10Mul]/I");
    tree->Branch("BB10Energy",  &fBB10Energy,  "BB10Energy[BB10Mul]/F");

    // QQQ5 Detectors
    tree->Branch("QQQ5Mul",           &fQQQ5Mul,           "QQQ5Mul/I");
    tree->Branch("QQQ5Upstream",      &fQQQ5Upstream,      "QQQ5Upstream[QQQ5Mul]/I");
    tree->Branch("QQQ5Det",           &fQQQ5Det,           "QQQ5Det[QQQ5Mul]/I");
    tree->Branch("QQQ5Ring",          &fQQQ5Ring,          "QQQ5Ring[QQQ5Mul]/I");
    tree->Branch("QQQ5RingChannel",   &fQQQ5RingChannel,   "QQQ5RingChannel[QQQ5Mul]/I");
    tree->Branch("QQQ5Sector",        &fQQQ5Sector,        "QQQ5Sector[QQQ5Mul]/I");
    tree->Branch("QQQ5SectorChannel", &fQQQ5SectorChannel, "QQQ5SectorChannel[QQQ5Mul]/I");
    tree->Branch("QQQ5RingADC",       &fQQQ5RingADC,       "QQQ5RingADC[QQQ5Mul]/I");
    tree->Branch("QQQ5RingEnergy",    &fQQQ5RingEnergy,    "QQQ5RingEnergy[QQQ5Mul]/F");
    tree->Branch("QQQ5SectorADC",     &fQQQ5SectorADC,     "QQQ5SectorADC[QQQ5Mul]/I");
    tree->Branch("QQQ5SectorEnergy",  &fQQQ5SectorEnergy,  "QQQ5SectorEnergy[QQQ5Mul]/F");
    tree->Branch("QQQ5Angle",         &fQQQ5Angle,         "QQQ5Angle[QQQ5Mul]/F");

    // Super X3 Detectors
    tree->Branch("SX3Mul",               &fSX3Mul,               "SX3Mul/I");
    tree->Branch("SX3Upstream",          &fSX3Upstream,          "SX3Upstream[SX3Mul]/I");
    tree->Branch("SX3Det",               &fSX3Det,               "SX3Det[SX3Mul]/I");
    tree->Branch("SX3Sector",            &fSX3Sector,            "SX3Sector[SX3Mul]/I");
    tree->Branch("SX3SectorChannel",     &fSX3SectorChannel,     "SX3SectorChannel[SX3Mul]/I");
    tree->Branch("SX3SectorADC",         &fSX3SectorADC,         "SX3SectorADC[SX3Mul]/I");
    tree->Branch("SX3SectorEnergy",      &fSX3SectorEnergy,      "SX3SectorEnergy[SX3Mul]/F");
    tree->Branch("SX3Strip",             &fSX3Strip,             "SX3Strip[SX3Mul]/I");
    tree->Branch("SX3StripLeftChannel",  &fSX3StripLeftChannel,  "SX3StripLeftChannel[SX3Mul]/I");
    tree->Branch("SX3StripRightChannel", &fSX3StripRightChannel, "SX3StripRightChannel[SX3Mul]/I");
    tree->Branch("SX3StripLeftADC",      &fSX3StripLeftADC,      "SX3StripLeftADC[SX3Mul]/I");
    tree->Branch("SX3StripRightADC",     &fSX3StripRightADC,     "SX3StripRightADC[SX3Mul]/I");
    tree->Branch("SX3StripEnergy",       &fSX3StripEnergy,       "SX3StripEnergy[SX3Mul]/F");

    // Ionization Chamber
    tree->Branch("icdE",                &fICdE,                "icdE/I");
    tree->Branch("icE",                 &fICE,                 "icE/I");
    tree->Branch("icWireX",             &fICWireX,             "icWireX/I");
    tree->Branch("icWireY",             &fICWireY,             "icWireY/I");
    tree->Branch("icPositionX",         &fICPositionX,         "icPositionX/F");
    tree->Branch("icPositionY",         &fICPositionY,         "icPositionY/F");
    tree->Branch("icPositionWeightedX", &fICPositionWeightedX, "icPositionWeightedX/F");
    tree->Branch("icPositionWeightedY", &fICPositionWeightedY, "icPositionWeightedY/F");

    // TDCs
    tree->Branch("tdcIC",      &fTDCIC,      "tdcIC/I");
    tree->Branch("tdcGRETINA", &fTDCGRETINA, "tdcGRETINA/I");
    tree->Branch("tdcRF",      &fTDCRF,      "tdcRF/I");
    tree->Branch("tdcSilicon", &fTDCSilicon, "tdcSilicon/I");

    // Timestamp
    tree->Branch("timeStamp", &fTimeStamp);

    //Declare some variables
    int NumberBuffer = 0;
    unsigned long int numberEvents = 0;
    int counter = 0 ;
    unsigned int buffer[BUFFER_LENGTH];
    unsigned int word;
    unsigned short halfWord[2];

    std::vector<int> readChannel;
    std::vector<int> readValue;

    //This is the main loop over the ldf file
    while(!file.eof()){

        //Get Buffer
        file.read((char*)buffer, BUFFER_LENGTHB);

        if(buffer[0] == 0x41544144) { //This buffer is physics data type
            for(int i = 0; i < BUFFER_LENGTH; i++) {

                word = buffer[i];
                // Reverse the byte order. Switching between big and little endian
                halfWord[0] = 0x0000ffff & word;
                halfWord[1] = word >> 16;
                word = (halfWord[0] << 16) | (halfWord[1]);

                //Check for the end of the event
                if (word != 0xffffffff) {

                    int channel = ExtractBits(word, 16, 12);
                    int value = ExtractBits(word, 0, 16);

                    //Add values to Read Arrays
                    readChannel.push_back(channel);
                    readValue.push_back(value);

                    counter++;

                } else { //End of the Event
                    std::vector<BB10Hit> BB10Hit_;
                    std::vector<QQQ5Ring> QdRing_;
                    std::vector<QQQ5Sector> QdSector_;
                    std::vector<QQQ5Ring> QuRing_;
                    std::vector<QQQ5Sector> QuSector_;
                    std::vector<SuperX3Back> SX3dBack_;
                    std::vector<SuperX3Front> SX3dFront_;
                    std::vector<SuperX3Back> SX3uBack_;
                    std::vector<SuperX3Front> SX3uFront_;

                    std::vector<ICTracking> ICx_;
                    std::vector<ICTracking> ICy_;

                    int icdE = 0;
                    int icE = 0;

                    int tdcIC = 0;
                    int tdcGRETINA = 0;
                    int tdcRF = 0;
                    int tdcSilicon = 0;

                    unsigned long long timeStamp = 0;

                    for(int k = 0; k < readChannel.size(); k++) {
                        int channel = readChannel[k];
                        int adc = readValue[k];

                        if(channel <= 128 && adc > QQQThreshold) { // QQQ5 upstream front (rings)
                            int detector = static_cast<int>((channel - 1)/32);
                            QQQ5Ring hit = {channel, detector, channel - 1 - detector*32, adc};
                            QuRing_.push_back(hit);
                        } else if(channel > 128 && channel <= 144 && adc > QQQThreshold) { // QQQ5 upstream back (sectors)
                            int detector = static_cast<int>((channel - 129)/4);
                            QQQ5Sector hit = {channel, detector, channel - 129 - detector*4, adc};
                            QuSector_.push_back(hit);
                        } else if(channel > 144 && channel <= 160 && adc > SX3Threshold) { // SuperX3 Upstream Detectors 0-3 (back sides)
                            int detector = static_cast<int>((channel - 145)/4);
                            SuperX3Back hit = {channel, detector, channel - 145 - detector*4, adc};
                            SX3uBack_.push_back(hit);
                        } else if(channel > 160 && channel <= 176 && adc > SX3Threshold) { // SuperX3 Upstream Detectors 6-9 (back sides)
                            int detector = static_cast<int>((channel - 161)/4) + 6;
                            SuperX3Back hit = {channel, detector, channel - 161 - (detector - 6)*4, adc};
                            SX3uBack_.push_back(hit);
                        } else if(channel > 176 && channel <= 184 && adc > SX3Threshold) { // SuperX3 Upstream Detectors 4-5 (back sides)
                            int detector = static_cast<int>((channel - 177)/4) + 4;
                            SuperX3Back hit = {channel, detector, channel - 177 - (detector - 4)*4, adc};
                            SX3uBack_.push_back(hit);
                        } else if(channel > 184 && channel <= 192 && adc > SX3Threshold) { // SuperX3 Upstream Detectors 10-11 (back sides)
                            int detector = static_cast<int>((channel - 185)/4) + 10;
                            SuperX3Back hit = {channel, detector, channel - 185 - (detector - 10)*4, adc};
                            SX3uBack_.push_back(hit);
                        } else if(channel > 192 && channel <= 288 && adc > SX3Threshold) { // SuperX3 Upstream (front sides)
                            int detector = static_cast<int>((channel - 193)/8);
                            int strip = static_cast<int>((channel - 193 - detector*8)/2);
                            bool leftSide = channel % 2 == 0;
                            SuperX3Front hit = {channel, detector, strip, leftSide, adc};
                            SX3uFront_.push_back(hit);
                        } else if(channel > 288 && channel <= 384 && adc > SX3Threshold) { // SuperX3 Downstream (front sides)
                            int detector = static_cast<int>((channel - 289)/8);
                            int strip = static_cast<int>((channel - 289 - detector*8)/2);
                            bool leftSide = channel % 2 == 0;
                            SuperX3Front hit = {channel, detector, strip, leftSide, adc};
                            SX3dFront_.push_back(hit);
                        } else if(channel > 384 && channel <= 400 && adc > SX3Threshold) { // SuperX3 Downstream Detectors 0-3 (back sides)
                            int detector = static_cast<int>((channel - 385)/4);
                            SuperX3Back hit = {channel, detector, channel - 385 - detector*4, adc};
                            SX3dBack_.push_back(hit);
                        } else if(channel > 400 && channel <= 416 && adc > SX3Threshold) { // SuperX3 Downstream Detectors 6-9 (back sides)
                            int detector = static_cast<int>((channel - 401)/4) + 6;
                            SuperX3Back hit = {channel, detector, channel - 401 - (detector - 6)*4, adc};
                            SX3dBack_.push_back(hit);
                        } else if(channel > 416 && channel <= 424 && adc > SX3Threshold) { // SuperX3 Downstream Detectors 4-5 (back sides)
                            int detector = static_cast<int>((channel - 417)/4) + 4;
                            SuperX3Back hit = {channel, detector, channel - 417 - (detector - 4)*4, adc};
                            SX3dBack_.push_back(hit);
                        } else if(channel > 424 && channel <= 432 && adc > SX3Threshold) { // SuperX3 Downstream Detectors 10-11 (back sides)
                            int detector = static_cast<int>((channel - 425)/4) + 10;
                            SuperX3Back hit = {channel, detector, channel - 425 - (detector - 10)*4, adc};
                            SX3dBack_.push_back(hit);
						// Downstream BB10s
                        } else if(channel > 432 && channel <= 496 && adc > BB10Threshold) {
                            int detector = static_cast<int>((channel - 433)/8);
                            BB10Hit hit = {channel, detector, channel - 433 - detector*8, adc};
                            BB10Hit_.push_back(hit); 
                 		// Downstream dE Layer QQQ5 Rings
                        }  else if(channel > 496 && channel <= 560 && adc > QQQThreshold) {
                            int detector = static_cast<int>((channel - 497)/32);
                            QQQ5Ring hit = {channel, detector, channel - 497 - detector*32, adc};
                            QdRing_.push_back(hit);
						// Downstream dE Layer QQQ5 Detector A Sectors
                        } else if(channel > 560 && channel <= 564 && adc > QQQThreshold) { // QQQ5 deltaE downstream back (sectors)
							int detector = static_cast<int>((channel - 561)/4); // always 0 (A)
							QQQ5Sector hit = {channel, detector, channel - 561, adc}; // I think 569 to 576 "should" be empty...
                            QdSector_.push_back(hit);
						// Downstream dE Layer QQQ5 Detector B Sectors
						} else if(channel > 568 && channel <= 572 && adc > QQQThreshold) { 
                            int detector = static_cast<int>((channel - 569)/4); // always 1 (B)
                            QQQ5Sector hit = {channel, detector+1, channel - 569, adc};
                            QdSector_.push_back(hit);
                        // Downstream E1 Layer QQQ5 Rings
                        } else if(channel > 576 && channel <= 640 && adc > QQQThreshold) { // QQQ5 E1&E2 downstream front (rings)
                            int detector = static_cast<int>((channel - 577)/32); // Either 2 (E1-A) or 3 (E1-B)
                            QQQ5Ring hit = {channel, detector+2, channel - 577 - detector*32, adc};
                            QdRing_.push_back(hit);
						// Downstream E2 Layer QQQ5 Detector B Rings
						} else if(channel > 640 && channel <= 672 && adc > QQQThreshold) { // QQQ5 E1&E2 downstream front (rings)
                            int detector = static_cast<int>((channel - 641)/32); // Always 5 (E2-B)
                            QQQ5Ring hit = {channel, detector+5, channel - 641 - detector*32, adc};
                            QdRing_.push_back(hit);
						// Downstream E1 Layer QQQ5 Detector A Sectors
                        } else if(channel > 672 && channel <= 676 && adc > QQQThreshold) { // QQQ5 E1&E2 downstream back (sectors)
                            int detector = static_cast<int>((channel - 673)/4); // Always 2 (E1-A)
                            QQQ5Sector hit = {channel, detector + 2, channel - 673, adc};  // needs to fill sector 0, 1, 2, 3
                            QdSector_.push_back(hit);
						// Downstream E1 Layer QQQ5 Detector B Sectors
                        } else if(channel > 680 && channel <= 684 && adc > QQQThreshold) { // QQQ5 E1&E2 downstream back (sectors)
                            int detector = static_cast<int>((channel - 681)/4); // Always 3 (E1-B)
                            QQQ5Sector hit = {channel, detector+3, channel - 681, adc};
                            QdSector_.push_back(hit);
                        // Downstream E2 Layer QQQ5 Detector A Sectors + Dummy Ring
                        } else if(channel > 676 && channel <= 680 && adc > QQQThreshold) { // QQQ5 E1&E2 downstream back (sectors)
                            int detector = static_cast<int>((channel - 677)/4); // Always 4 (E2-A)
                            QQQ5Sector hit = {channel, detector+4, channel - 677, adc};
                            QdSector_.push_back(hit);
							QQQ5Ring hit2 = {channel, detector+4, channel - 677, adc};
                            QdRing_.push_back(hit2);	// copy hit into a ring
						// Downstream E2 Layer QQQ5 Detector B Sectors
                        } else if(channel > 684 && channel <= 688 && adc > QQQThreshold) { // QQQ5 E1&E2 downstream back (sectors)
                            int detector = static_cast<int>((channel - 685)/4); // Always 5 (E2-B)
                            QQQ5Sector hit = {channel, detector+5, channel - 685, adc};
                            QdSector_.push_back(hit);
                        }
                        //#####################
                        
                        //else if(channel > 672 && channel <= 704 && adc > ICTrackingThreshold) { // IC x
                        //    int wire = channel - 673;
                        //    ICTracking hit = {true, wire, adc};
                        //    ICx_.push_back(hit);
                        //} else if(channel > 704 && channel <= 736 && adc > ICTrackingThreshold) { // IC y
                        //    int wire = channel - 705;
                        //    ICTracking hit = {false, wire, adc};
                        //    ICy_.push_back(hit);
                        //} else if(channel == 739) { //IC dE
                        //    icdE = adc;
                        //} else if(channel == 740) { //IC E
                        //    icE = adc;
                        //} else if(channel == 818) { // TDC IC
                        //    tdcIC = adc;
                        //}
                         
                        else if(channel == 839) { // TDC GRETINA (was 819)
                            tdcGRETINA = adc;
                        } else if(channel == 836) { // TDC RF (was 821)
                            tdcRF = adc;
                        } else if(channel == 835) { // TDC Silicon (was 822)
                            tdcSilicon = adc;
                        }

                        if(channel >= 1000 && channel <= 1003) {
                            timeStamp |= (unsigned long long) adc << (16*(channel - 1000));
                        }
                    }

                    ///////////////////////////
                    // End of Sub-event loop //
                    ///////////////////////////

                    readChannel.clear();
                    readValue.clear();

                    numberEvents++;

                    // BB10
                    std::vector<BB10Detector> BB10Detect_;
                    if(!BB10Hit_.empty()) BB10Detect_ = ProcessBB10(BB10Hit_);

                    // QQQ5 Downstream
                    std::vector<QQQ5Detector> QdDetect_;
                    if(!QdRing_.empty() && !QdSector_.empty()) QdDetect_ = ProcessQQQ5(QdRing_, QdSector_, false);

                    // QQQ5 Upstream
                    std::vector<QQQ5Detector> QuDetect_;
                    if(!QuRing_.empty() && !QuSector_.empty()) QuDetect_ = ProcessQQQ5(QuRing_, QuSector_, true);

                    // Super X3 Downstream
                    std::vector<SuperX3Detector> SX3dDetect_;
                    if(!SX3dBack_.empty() && !SX3dFront_.empty()) SX3dDetect_ = ProcessSX3(SX3dBack_, SX3dFront_, false);

                    // Super X3 Upstream
                    std::vector<SuperX3Detector> SX3uDetect_;
                    if(!SX3uBack_.empty() && !SX3uFront_.empty()) SX3uDetect_ = ProcessSX3(SX3uBack_, SX3uFront_, true);

                    // IC Tracking
                    std::vector<ICTrackingDetector> ICTrackDetect_;
                    if(!ICx_.empty() && !ICy_.empty()) ICTrackDetect_ = ProcessIC(ICx_, ICy_);

                    bool BB10DetectEmpty = BB10Detect_.empty();
                    bool QQQDetectEmpty = (QdDetect_.empty() && QuDetect_.empty());
                    bool SX3DetectEmpty = (SX3dDetect_.empty() && SX3uDetect_.empty());
                    bool icEmpty = ((icE == 0) && (icdE == 0));

                    if(BB10DetectEmpty && QQQDetectEmpty && SX3DetectEmpty && icEmpty) continue;

                    // BB10 Detectors
                    fBB10Mul = 0;
                    for(auto BB10Detect: BB10Detect_) {
                        fBB10Det[fBB10Mul] = BB10Detect.detector;
                        fBB10Strip[fBB10Mul] = BB10Detect.strip;
                        fBB10Channel[fBB10Mul] = BB10Detect.channel;
                        fBB10ADC[fBB10Mul] = BB10Detect.adc;
                        fBB10Energy[fBB10Mul] = BB10Detect.energy;
                        fBB10Mul++;
                    }

                    // QQQ5 Detectors
                    fQQQ5Mul = 0;
                    for(auto QDetect: QdDetect_) {
                        fQQQ5Upstream[fQQQ5Mul] = QDetect.upstream;
                        fQQQ5Det[fQQQ5Mul] = QDetect.detector;
                        fQQQ5Ring[fQQQ5Mul] = QDetect.ring;
                        fQQQ5RingChannel[fQQQ5Mul] = QDetect.ringChannel;
                        fQQQ5Sector[fQQQ5Mul] = QDetect.sector;
                        fQQQ5SectorChannel[fQQQ5Mul] = QDetect.sectorChannel;
                        fQQQ5RingADC[fQQQ5Mul] = QDetect.ringEnergyADC;
                        fQQQ5RingEnergy[fQQQ5Mul] = QDetect.ringEnergy;
                        fQQQ5SectorADC[fQQQ5Mul] = QDetect.sectorEnergyADC;
                        fQQQ5SectorEnergy[fQQQ5Mul] = QDetect.sectorEnergy;
                        fQQQ5Angle[fQQQ5Mul] = QDetect.angle;
                        fQQQ5Mul++;
                    }
					// why not reset fQQQ5Mul=0 here too? I guess not interested in upstream and downstream multiplicity separately? 
					// if there were and upstream and downstream hit then multiplicity = 2.
					// Also multiplicity = 2 if both layers of a telescope are hit. In the analysis may want loop over multiplicity and sum ring energies
                    for(auto QDetect: QuDetect_) {
                        fQQQ5Upstream[fQQQ5Mul] = QDetect.upstream;
                        fQQQ5Det[fQQQ5Mul] = QDetect.detector;
                        fQQQ5Ring[fQQQ5Mul] = QDetect.ring;
                        fQQQ5RingChannel[fQQQ5Mul] = QDetect.ringChannel;
                        fQQQ5Sector[fQQQ5Mul] = QDetect.sector;
                        fQQQ5SectorChannel[fQQQ5Mul] = QDetect.sectorChannel;
                        fQQQ5RingADC[fQQQ5Mul] = QDetect.ringEnergyADC;
                        fQQQ5RingEnergy[fQQQ5Mul] = QDetect.ringEnergy;
                        fQQQ5SectorADC[fQQQ5Mul] = QDetect.sectorEnergyADC;
                        fQQQ5SectorEnergy[fQQQ5Mul] = QDetect.sectorEnergy;
                        fQQQ5Angle[fQQQ5Mul] = QDetect.angle;
                        fQQQ5Mul++;
                    }

                    // Super X3 Detectors
                    fSX3Mul = 0;
                    for(auto SX3Detect: SX3dDetect_) {
                        fSX3Upstream[fSX3Mul] = SX3Detect.upstream;
                        fSX3Det[fSX3Mul] = SX3Detect.detector;
                        fSX3Sector[fSX3Mul] = SX3Detect.sector;
                        fSX3SectorChannel[fSX3Mul] = SX3Detect.sectorChannel;
                        fSX3SectorADC[fSX3Mul] = SX3Detect.sectorADC;
                        fSX3SectorEnergy[fSX3Mul] = SX3Detect.sectorEnergy;
                        fSX3Strip[fSX3Mul] = SX3Detect.strip;
                        fSX3StripLeftChannel[fSX3Mul] = SX3Detect.stripLeftChannel;
                        fSX3StripRightChannel[fSX3Mul] = SX3Detect.stripRightChannel;
                        fSX3StripLeftADC[fSX3Mul] = SX3Detect.stripLeftADC;
                        fSX3StripRightADC[fSX3Mul] = SX3Detect.stripRightADC;
                        fSX3StripEnergy[fSX3Mul] = SX3Detect.stripEnergy;
                        fSX3Mul++;
                    }
                    for(auto SX3Detect: SX3uDetect_) {
                        fSX3Upstream[fSX3Mul] = SX3Detect.upstream;
                        fSX3Det[fSX3Mul] = SX3Detect.detector;
                        fSX3Sector[fSX3Mul] = SX3Detect.sector;
                        fSX3SectorChannel[fSX3Mul] = SX3Detect.sectorChannel;
                        fSX3SectorADC[fSX3Mul] = SX3Detect.sectorADC;
                        fSX3SectorEnergy[fSX3Mul] = SX3Detect.sectorEnergy;
                        fSX3Strip[fSX3Mul] = SX3Detect.strip;
                        fSX3StripLeftChannel[fSX3Mul] = SX3Detect.stripLeftChannel;
                        fSX3StripRightChannel[fSX3Mul] = SX3Detect.stripRightChannel;
                        fSX3StripLeftADC[fSX3Mul] = SX3Detect.stripLeftADC;
                        fSX3StripRightADC[fSX3Mul] = SX3Detect.stripRightADC;
                        fSX3StripEnergy[fSX3Mul] = SX3Detect.stripEnergy;
                        fSX3Mul++;
                    }

                    fICdE = icdE;
                    fICE = icE;

                    if(!ICTrackDetect_.empty()) {
                        fICWireX = ICTrackDetect_[0].wireX;
                        fICWireY = ICTrackDetect_[0].wireY;
                        fICPositionX = ICTrackDetect_[0].positionX;
                        fICPositionY = ICTrackDetect_[0].positionY;
                        fICPositionWeightedX = ICTrackDetect_[0].positionWeightedX;
                        fICPositionWeightedY = ICTrackDetect_[0].positionWeightedY;
                    }

                    fTDCIC = tdcIC;
                    fTDCGRETINA = tdcGRETINA;
                    fTDCRF = tdcRF;
                    fTDCSilicon = tdcSilicon;

                    fTimeStamp = timeStamp;

                    fRunNumber = run.runNumber;

                    tree->Fill();

                    counter = 0;
                }
            }
        }
        NumberBuffer++;
    } //End of main loop over file

    tree->Write();
    outputFile->Close();

    file.close();
    int runClock = clock();

    std::cout << PrintOutput("\t\tFinished Unpacking Run: ", "cyan") << run.runNumber << '\t';
    std::cout << PrintOutput("Time", "cyan") << " = " << Form("%.02f", (runClock - startClock)/double(CLOCKS_PER_SEC)) << " seconds" << std::flush << std::endl;
    std::cout << PrintOutput("\t\tNumber of events: ", "cyan") << numberEvents << std::flush << std::endl;
    std::cout << PrintOutput("\t\tCreated ROOT file : ", "cyan") << outputFile->GetName() << std::endl;

    if(run.copyCuts) {
        std::ifstream src(run.preCutPath, std::ios::binary);
        std::ofstream dst(run.cutPath, std::ios::binary);
        try {
            dst << src.rdbuf();
            std::cout << PrintOutput("\t\tCopied cut file from run: ", "cyan") << run.runNumber.c_str() << std::endl;
        }
        catch(int e) {
            std::cout << PrintOutput(Form("\t\tDid not copy cut from run %s", run.runNumber.c_str()), "red") << std::endl;
        }
        src.close();
        dst.close();
    }

    completed = true;
}
