#include "Unpack.h"

int main(int argc, char *argv[]) {
    auto* unpacker = new Unpack();

    return 0;
}


Unpack::Unpack() {
    int StartClock = clock();
    std::cout << PrintOutput("Running GODDESS sort", "yellow") << std::endl;

    std::cout << PrintOutput("Reading RunList", "yellow") << std::endl;
    auto* runList = new RunList();
    auto fileList = runList->GetListOfRuns();

    std::cout << PrintOutput("Number of files to be sort = ", "yellow") << fileList.size() << std::endl;

    int numRuns = 0;
    for(auto run: fileList) {
        std::cout << PrintOutput(Form("Processing Run %s: \n", run.runNumber.c_str()), "green");

        auto* orruba = new UnpackORRUBA(run);
        bool orrubaCompleted = orruba->GetCompleted();

        bool gretinaCompleted = false;
        if(run.unpackGRETINA) {
            auto* gretina = new UnpackGRETINA(run);
            gretinaCompleted = gretina->GetCompleted();
        }

        if(orrubaCompleted && gretinaCompleted && run.mergeTrees) {
            CombineReader(run);
        }

        // // For debugging merge
        // CombineReader(run);
    }

    std::cout << PrintOutput("************************************************", "yellow") << std::endl;
    std::cout << PrintOutput("Finished Unpacking ", "yellow") << fileList.size() << PrintOutput(" files!", "yellow") <<  std::endl;
}

void Unpack::CombineReader(fileListStruct run) {
    std::cout << PrintOutput("\tCombining ORRUBA and GRETINA trees based on timestamp:", "yellow") << std::endl;

    std::cout << PrintOutput("\t\tOpening ORRUBA file: ", "blue") << run.rootPath.c_str() << std::endl;
    auto f_ORRUBA = TFile::Open(Form("%s", run.rootPath.c_str()));
    if(!f_ORRUBA) {
        std::cout << PrintOutput("\t\tCould not open Combined file: ", "red") << run.rootPath.c_str() << std::endl;
        return;
    }
    TTreeReader t_ORRUBA("data", f_ORRUBA);
    Long64_t nentriesORRUBA = t_ORRUBA.GetEntries(true);
    if(nentriesORRUBA == 0) {
        std::cout << PrintOutput("\t\tCould not open TTree 'data' in ORRBUA file", "red") << std::endl;
        return;
    }
    TTree *tree_ORRUBA = (TTree*)f_ORRUBA->Get("data");

    std::cout << PrintOutput("\t\tOpening GRETINA file: ", "blue") << run.gretinaPath.c_str() << std::endl;
    auto f_GRETINA = TFile::Open(Form("%s", run.gretinaPath.c_str()));
    if(!f_GRETINA) {
        std::cout << PrintOutput("\t\tCould not open GRETINA file: ", "red") << run.gretinaPath.c_str() << std::endl;
        return;
    }

    TTree *tree_GRETINA = (TTree*)f_GRETINA->Get("teb");
    Long64_t nentriesGRETINA = tree_GRETINA->GetEntries();
    g2OUT *g2 = 0;
    tree_GRETINA->SetBranchAddress("g2", &g2);

    std::cout << PrintOutput("\t\t\tTotal ORRUBA Entries: ", "yellow") << nentriesORRUBA << PrintOutput("; Total GRETINA Entries: ", "yellow") << nentriesGRETINA << std::endl;

    TTreeReaderValue<ULong64_t> orrubaTimeStamp(t_ORRUBA, "timeStamp");

    std::vector<std::pair<Int_t, Long64_t> > orrubaTimeStamps_;
    std::vector<std::pair<Int_t, Long64_t> > gretinaTimeStamps_;

    // Loop through ORRUBA events and get the timestamps
    Int_t i = 0;
    while(t_ORRUBA.Next()) {
        orrubaTimeStamps_.push_back(std::make_pair(i, *orrubaTimeStamp));
        i++;
    }

    for (int i = 0; i < nentriesGRETINA; i++) {
        tree_GRETINA->GetEntry(i);
        for (auto g2Event: g2->xtals) {
            gretinaTimeStamps_.push_back(std::make_pair(i, g2Event.timestamp));
            break;
        }
    }

    // Loop through ORRUBA events and for each event, match to the corresponding GRETINA event based on the timestamp
    // The difference of timestamps is to be < 1000 which is a lot considering the timestamps between two ORRUBA events
    // are generally on the order of 100,000.
    std::vector<matchedEvents> matchedEvents_;
    //Use this for data runs
     // comment / un-comment point
    for(size_t i = 0; i < orrubaTimeStamps_.size(); i++) { 
        size_t found_j = 0;
        int found_index = 0;
        Long64_t orrubaTime = orrubaTimeStamps_[i].second;
        Bool_t found = false;

        size_t best_j;
        Long64_t closestTime = 100000; 
        Long64_t gretinaTime;
        for(size_t j = 0; j < gretinaTimeStamps_.size(); j++) {
            size_t timeDiff = fabs(orrubaTime - gretinaTimeStamps_[j].second);
            if((timeDiff < 2000) && (timeDiff < closestTime)) { // original set to 2000
                closestTime = fabs(orrubaTime - gretinaTimeStamps_[j].second);
                gretinaTime = gretinaTimeStamps_[j].second;
                if (timeDiff > closestTime) std::cout << orrubaTime << '\t' << gretinaTime << std::endl; //RG Added

                best_j = gretinaTimeStamps_[j].first;
                found_index = j;
                found = true;

            }

            if(timeDiff > 5000 && found) break;
            if(timeDiff > 50000 && (gretinaTimeStamps_[j].second > orrubaTime)) break;
        }

        // Record ORRUBA + GRETINA timestamps
        if(found) {
            matchedEvents hit = {i, best_j, orrubaTime, gretinaTime};
            matchedEvents_.push_back(hit);
        }
        // Record ORRUBA hits that do not have a GRETINA timestamp
        else {
            matchedEvents hit = {i, 0, orrubaTime, 0};
            matchedEvents_.push_back(hit);
        }  //Comment-Uncomment Point
    /*
    //Use this for GRETINA Calibration runs
    for(size_t i = 0; i < 1; i++) { 
        size_t found_j = 0;
        int found_index = 0;
        Long64_t orrubaTime = orrubaTimeStamps_[i].second;
        Bool_t found = false;

        size_t best_j;
        Long64_t gretinaTime;
        for(size_t j = 0; j < gretinaTimeStamps_.size(); j++) {
            gretinaTime = gretinaTimeStamps_[j].second;
            best_j = gretinaTimeStamps_[j].first;
            found_index = j;
            found = true;

            // Record ORRUBA + GRETINA timestamps
            if(found) {
                matchedEvents hit = {i, best_j, orrubaTime, gretinaTime};
                matchedEvents_.push_back(hit);
            }
            // Record ORRUBA hits that do not have a GRETINA timestamp
            else {
                matchedEvents hit = {i, 0, orrubaTime, 0};
                matchedEvents_.push_back(hit);
            }
        } */ //Comment-Uncomment Point
         
        // Remove the first found_index elements and shift everything else down by found_index indices
        // This is so that we don't have to loop through GRETINA events that have already been matched
        // Don't do it for every event as it will slow it down too much. Every 1000 seems to work well
        if((i % 1000 == 0) && found) gretinaTimeStamps_.erase(gretinaTimeStamps_.begin(), gretinaTimeStamps_.begin() + found_index);
    }

    std::cout << PrintOutput("\t\tMatched ORRUBA and GRETINA time stamps", "blue") << std::endl;

    // Reset ORRUBA TTreeReaders
    t_ORRUBA.Restart();

    // Create Combined TTree
    TFile* f_Combined = new TFile(run.combinedPath.c_str(), "recreate");
    TTree* tree_Combined = new TTree("data_combined", "Combined ORRUBA and GRETINA data");

    //Test Histogram
    TH1F *TS_diff = new TH1F("TS_diff","Time Between GRETINA Events",2000,-1000,1000);
    TH1F *TS_diff2 = new TH1F("TS_diff2","ORRUBA - GRETINA Time-Stamps",2000,-1000,1000);


    // It appears g2 is the important branch in the GRETINA dataset
    tree_GRETINA->SetBranchAddress("g2", &g2);

    // Get ORRUBA branches from ORRUBA tree
    std::string *RunNumber = 0;
    int BB10Mul, BB10Det[128], BB10Strip[128], BB10Channel[128], BB10ADC[128];
    float BB10Energy[128];
    int QQQ5Mul, QQQ5Upstream[128], QQQ5Det[128], QQQ5Ring[128], QQQ5RingChannel[128], QQQ5Sector[128], QQQ5SectorChannel[128], QQQ5RingADC[128], QQQ5SectorADC[128];
    float QQQ5RingEnergy[128], QQQ5SectorEnergy[128], QQQ5Angle[128];
    int SX3Mul, SX3Upstream[128], SX3Det[128], SX3Sector[128], SX3SectorChannel[128], SX3SectorADC[128], SX3Strip[128], SX3StripLeftChannel[128], SX3StripRightChannel[128], SX3StripLeftADC[128], SX3StripRightADC[128];
    float SX3SectorEnergy[128], SX3StripEnergy[128], SX3StripRightEnergy[128], SX3StripLeftEnergy[128], SX3StripPosition[128], SX3StripPositionCal[128];
    int ICdE, ICE, ICWireX, ICWireY;
    float ICPositionX, ICPositionY, ICPositionWeightedX, ICPositionWeightedY;
    int TDCIC, TDCGRETINA, TDCRF, TDCSilicon, TDCSilicon_Div, TDCSilicon_GRETINA, TDCSilicon_Delay, TDCSilicon_Upstream;
    unsigned long long TimeStamp;

    tree_ORRUBA->SetBranchAddress("RunNumber", &RunNumber);

    tree_ORRUBA->SetBranchAddress("BB10Mul",     &BB10Mul);
    tree_ORRUBA->SetBranchAddress("BB10Det",     &BB10Det);
    tree_ORRUBA->SetBranchAddress("BB10Strip",   &BB10Strip);
    tree_ORRUBA->SetBranchAddress("BB10Channel", &BB10Channel);
    tree_ORRUBA->SetBranchAddress("BB10ADC",     &BB10ADC);
    tree_ORRUBA->SetBranchAddress("BB10Energy",  &BB10Energy);

    tree_ORRUBA->SetBranchAddress("QQQ5Mul",           &QQQ5Mul);
    tree_ORRUBA->SetBranchAddress("QQQ5Upstream",      &QQQ5Upstream);
    tree_ORRUBA->SetBranchAddress("QQQ5Det",           &QQQ5Det);
    tree_ORRUBA->SetBranchAddress("QQQ5Ring",          &QQQ5Ring);
    tree_ORRUBA->SetBranchAddress("QQQ5RingChannel",   &QQQ5RingChannel);
    tree_ORRUBA->SetBranchAddress("QQQ5Sector",        &QQQ5Sector);
    tree_ORRUBA->SetBranchAddress("QQQ5SectorChannel", &QQQ5SectorChannel);
    tree_ORRUBA->SetBranchAddress("QQQ5RingADC",       &QQQ5RingADC);
    tree_ORRUBA->SetBranchAddress("QQQ5RingEnergy",    &QQQ5RingEnergy);
    tree_ORRUBA->SetBranchAddress("QQQ5SectorADC",     &QQQ5SectorADC);
    tree_ORRUBA->SetBranchAddress("QQQ5SectorEnergy",  &QQQ5SectorEnergy);
    tree_ORRUBA->SetBranchAddress("QQQ5Angle",         &QQQ5Angle);

    tree_ORRUBA->SetBranchAddress("SX3Mul",               &SX3Mul);
    tree_ORRUBA->SetBranchAddress("SX3Upstream",          &SX3Upstream);
    tree_ORRUBA->SetBranchAddress("SX3Det",               &SX3Det);
    tree_ORRUBA->SetBranchAddress("SX3Sector",            &SX3Sector);
    tree_ORRUBA->SetBranchAddress("SX3SectorChannel",     &SX3SectorChannel);
    tree_ORRUBA->SetBranchAddress("SX3SectorADC",         &SX3SectorADC);
    tree_ORRUBA->SetBranchAddress("SX3SectorEnergy",      &SX3SectorEnergy);
    tree_ORRUBA->SetBranchAddress("SX3Strip",             &SX3Strip);
    tree_ORRUBA->SetBranchAddress("SX3StripLeftChannel",  &SX3StripLeftChannel);
    tree_ORRUBA->SetBranchAddress("SX3StripRightChannel", &SX3StripRightChannel);
    tree_ORRUBA->SetBranchAddress("SX3StripLeftADC",      &SX3StripLeftADC);
    tree_ORRUBA->SetBranchAddress("SX3StripRightADC",     &SX3StripRightADC);
    tree_ORRUBA->SetBranchAddress("SX3StripEnergy",       &SX3StripEnergy);
    tree_ORRUBA->SetBranchAddress("SX3StripRightEnergy",  &SX3StripRightEnergy);
    tree_ORRUBA->SetBranchAddress("SX3StripLeftEnergy",   &SX3StripLeftEnergy);
    tree_ORRUBA->SetBranchAddress("SX3StripPosition",     &SX3StripPosition);
    tree_ORRUBA->SetBranchAddress("SX3StripPositionCal",  &SX3StripPositionCal);

    tree_ORRUBA->SetBranchAddress("icdE",                &ICdE);
    tree_ORRUBA->SetBranchAddress("icE",                 &ICE);
    tree_ORRUBA->SetBranchAddress("icWireX",             &ICWireX);
    tree_ORRUBA->SetBranchAddress("icWireY",             &ICWireY);
    tree_ORRUBA->SetBranchAddress("icPositionX",         &ICPositionX);
    tree_ORRUBA->SetBranchAddress("icPositionY",         &ICPositionY);
    tree_ORRUBA->SetBranchAddress("icPositionWeightedX", &ICPositionWeightedX);
    tree_ORRUBA->SetBranchAddress("icPositionWeightedY", &ICPositionWeightedY);

    tree_ORRUBA->SetBranchAddress("tdcIC",      &TDCIC);
    tree_ORRUBA->SetBranchAddress("tdcGRETINA", &TDCGRETINA);
    tree_ORRUBA->SetBranchAddress("tdcRF",      &TDCRF);
    tree_ORRUBA->SetBranchAddress("tdcSilicon", &TDCSilicon);
	tree_ORRUBA->SetBranchAddress("tdcSilicon_Div",		&TDCSilicon_Div);
	tree_ORRUBA->SetBranchAddress("tdcSilicon_GRETINA",	&TDCSilicon_GRETINA);
	tree_ORRUBA->SetBranchAddress("tdcSilicon_Delay",	&TDCSilicon_Delay);
	tree_ORRUBA->SetBranchAddress("tdcSilicon_Upstream",&TDCSilicon_Upstream);

    tree_ORRUBA->SetBranchAddress("timeStamp", &TimeStamp);

    // Set ORRUBA branches in Combined tree
    std::string fRunNumber;
    int fBB10Mul, fBB10Det[128], fBB10Strip[128], fBB10Channel[128], fBB10ADC[128];
    float fBB10Energy[128];
    int fQQQ5Mul, fQQQ5Upstream[128], fQQQ5Det[128], fQQQ5Ring[128], fQQQ5RingChannel[128], fQQQ5Sector[128], fQQQ5SectorChannel[128], fQQQ5RingADC[128], fQQQ5SectorADC[128];
    float fQQQ5RingEnergy[128], fQQQ5SectorEnergy[128], fQQQ5Angle[128];
    int fSX3Mul, fSX3Upstream[128], fSX3Det[128], fSX3Sector[128], fSX3SectorChannel[128], fSX3SectorADC[128], fSX3Strip[128], fSX3StripLeftChannel[128], fSX3StripRightChannel[128], fSX3StripLeftADC[128], fSX3StripRightADC[128];
    float fSX3SectorEnergy[128], fSX3StripEnergy[128], fSX3StripRightEnergy[128], fSX3StripLeftEnergy[128], fSX3StripPosition[128], fSX3StripPositionCal[128];
    int fICdE, fICE, fICWireX, fICWireY;
    float fICPositionX, fICPositionY, fICPositionWeightedX, fICPositionWeightedY;
    int fTDCIC, fTDCGRETINA, fTDCRF, fTDCSilicon, fTDCSilicon_Div, fTDCSilicon_GRETINA, fTDCSilicon_Delay, fTDCSilicon_Upstream;
    unsigned long long fTimeStamp, fGRETINA_TimeStamp;

    tree_Combined->Branch("RunNumber", &fRunNumber);

    tree_Combined->Branch("BB10Mul",     &fBB10Mul,     "BB10Mul/I");
    tree_Combined->Branch("BB10Det",     &fBB10Det,     "BB10Det[BB10Mul]/I");
    tree_Combined->Branch("BB10Strip",   &fBB10Strip,   "BB10Strip[BB10Mul]/I");
    tree_Combined->Branch("BB10Channel", &fBB10Channel, "BB10Channel[BB10Mul]/I");
    tree_Combined->Branch("BB10ADC",     &fBB10ADC,     "BB10ADC[BB10Mul]/I");
    tree_Combined->Branch("BB10Energy",  &fBB10Energy,  "BB10Energy[BB10Mul]/F");

    tree_Combined->Branch("QQQ5Mul",           &fQQQ5Mul,           "QQQ5Mul/I");
    tree_Combined->Branch("QQQ5Upstream",      &fQQQ5Upstream,      "QQQ5Upstream[QQQ5Mul]/I");
    tree_Combined->Branch("QQQ5Det",           &fQQQ5Det,           "QQQ5Det[QQQ5Mul]/I");
    tree_Combined->Branch("QQQ5Ring",          &fQQQ5Ring,          "QQQ5Ring[QQQ5Mul]/I");
    tree_Combined->Branch("QQQ5RingChannel",   &fQQQ5RingChannel,   "QQQ5RingChannel[QQQ5Mul]/I");
    tree_Combined->Branch("QQQ5Sector",        &fQQQ5Sector,        "QQQ5Sector[QQQ5Mul]/I");
    tree_Combined->Branch("QQQ5SectorChannel", &fQQQ5SectorChannel, "QQQ5SectorChannel[QQQ5Mul]/I");
    tree_Combined->Branch("QQQ5RingADC",       &fQQQ5RingADC,       "QQQ5RingADC[QQQ5Mul]/I");
    tree_Combined->Branch("QQQ5RingEnergy",    &fQQQ5RingEnergy,    "QQQ5RingEnergy[QQQ5Mul]/F");
    tree_Combined->Branch("QQQ5SectorADC",     &fQQQ5SectorADC,     "QQQ5SectorADC[QQQ5Mul]/I");
    tree_Combined->Branch("QQQ5SectorEnergy",  &fQQQ5SectorEnergy,  "QQQ5SectorEnergy[QQQ5Mul]/F");
    tree_Combined->Branch("QQQ5Angle",         &fQQQ5Angle,         "QQQ5Angle[QQQ5Mul]/F");

    tree_Combined->Branch("SX3Mul",               &fSX3Mul,               "SX3Mul/I");
    tree_Combined->Branch("SX3Upstream",          &fSX3Upstream,          "SX3Upstream[SX3Mul]/I");
    tree_Combined->Branch("SX3Det",               &fSX3Det,               "SX3Det[SX3Mul]/I");
    tree_Combined->Branch("SX3Sector",            &fSX3Sector,            "SX3Sector[SX3Mul]/I");
    tree_Combined->Branch("SX3SectorChannel",     &fSX3SectorChannel,     "SX3SectorChannel[SX3Mul]/I");
    tree_Combined->Branch("SX3SectorADC",         &fSX3SectorADC,         "SX3SectorADC[SX3Mul]/I");
    tree_Combined->Branch("SX3SectorEnergy",      &fSX3SectorEnergy,      "SX3SectorEnergy[SX3Mul]/F");
    tree_Combined->Branch("SX3Strip",             &fSX3Strip,             "SX3Strip[SX3Mul]/I");
    tree_Combined->Branch("SX3StripLeftChannel",  &fSX3StripLeftChannel,  "SX3StripLeftChannel[SX3Mul]/I");
    tree_Combined->Branch("SX3StripRightChannel", &fSX3StripRightChannel, "SX3StripRightChannel[SX3Mul]/I");
    tree_Combined->Branch("SX3StripLeftADC",      &fSX3StripLeftADC,      "SX3StripLeftADC[SX3Mul]/I");
    tree_Combined->Branch("SX3StripRightADC",     &fSX3StripRightADC,     "SX3StripRightADC[SX3Mul]/I");
    tree_Combined->Branch("SX3StripLeftEnergy",   &fSX3StripLeftEnergy,   "SX3StripLeftEnergy[SX3Mul]/F");
	tree_Combined->Branch("SX3StripRightEnergy",  &fSX3StripRightEnergy,  "SX3StripRightEnergy[SX3Mul]/F");
	tree_Combined->Branch("SX3StripEnergy",   	  &fSX3StripEnergy,       "SX3StripEnergy[SX3Mul]/F");
	tree_Combined->Branch("SX3StripPosition",     &fSX3StripPosition,     "SX3StripPosition[SX3Mul]/F");
	tree_Combined->Branch("SX3StripPositionCal",  &fSX3StripPositionCal,  "SX3StripPositionCal[SX3Mul]/F");

    tree_Combined->Branch("icdE",                &fICdE,                "icdE/I");
    tree_Combined->Branch("icE",                 &fICE,                 "icE/I");
    tree_Combined->Branch("icWireX",             &fICWireX,             "icWireX/I");
    tree_Combined->Branch("icWireY",             &fICWireY,             "icWireY/I");
    tree_Combined->Branch("icPositionX",         &fICPositionX,         "icPositionX/F");
    tree_Combined->Branch("icPositionY",         &fICPositionY,         "icPositionY/F");
    tree_Combined->Branch("icPositionWeightedX", &fICPositionWeightedX, "icPositionWeightedX/F");
    tree_Combined->Branch("icPositionWeightedY", &fICPositionWeightedY, "icPositionWeightedY/F");

    tree_Combined->Branch("tdcIC",              &fTDCIC,                "tdcIC/I");
    tree_Combined->Branch("tdcGRETINA",         &fTDCGRETINA,           "tdcGRETINA/I");
    tree_Combined->Branch("tdcRF",              &fTDCRF,                "tdcRF/I");
    tree_Combined->Branch("tdcSilicon",         &fTDCSilicon,           "tdcSilicon/I");
    tree_Combined->Branch("tdcSilicon_Div",		&fTDCSilicon_Div,   	"tdcSilicon_Div/I");
	tree_Combined->Branch("tdcSilicon_GRETINA",	&fTDCSilicon_GRETINA,   "tdcSilicon_GRETINA/I");
	tree_Combined->Branch("tdcSilicon_Delay",	&fTDCSilicon_Delay,   	"tdcSilicon_Delay/I");
	tree_Combined->Branch("tdcSilicon_Upstream",&fTDCSilicon_Upstream,  "tdcSilicon_Upstream/I");

    tree_Combined->Branch("timeStamp", &fTimeStamp, "timeStamp/L");
    tree_Combined->Branch("GRETINA_TimeStamp", &fGRETINA_TimeStamp, "GRETINA_TimeStamp/L");

    // Set GRETINA branches in Combined tree
    bool foundGRETINA;
    Int_t xtalsMul;
    Float_t xtals_xlab[128];
    Float_t xtals_ylab[128];
    Float_t xtals_zlab[128];
    Float_t xtals_cc[128];
    Float_t xtals_cc1[128];
    Float_t xtals_cc2[128];
    Float_t xtals_cc3[128];
    Float_t xtals_cc4[128];
    Float_t xtals_edop[128];
    Float_t xtals_edopMaxInt[128];
    Float_t xtals_edopSeg[128];
    Float_t xtals_edopXtal[128];
    Int_t xtals_crystalNum[128];
    Int_t xtals_quadNum[128];
    Float_t xtals_t0[128];
    Long64_t xtals_timestamp[128];

    tree_Combined->Branch("foundGRETINA", &foundGRETINA);

    tree_Combined->Branch("xtalsMul", &xtalsMul, "xtalsMul/I");
    tree_Combined->Branch("xtals_xlab", &xtals_xlab, "xtals_xlab[xtalsMul]/F");
    tree_Combined->Branch("xtals_ylab", &xtals_ylab, "xtals_ylab[xtalsMul]/F");
    tree_Combined->Branch("xtals_zlab", &xtals_zlab, "xtals_zlab[xtalsMul]/F");
    tree_Combined->Branch("xtals_cc", &xtals_cc, "xtals_cc[xtalsMul]/F");
    tree_Combined->Branch("xtals_cc1", &xtals_cc1, "xtals_cc1[xtalsMul]/F");
    tree_Combined->Branch("xtals_cc2", &xtals_cc2, "xtals_cc2[xtalsMul]/F");
    tree_Combined->Branch("xtals_cc3", &xtals_cc3, "xtals_cc3[xtalsMul]/F");
    tree_Combined->Branch("xtals_cc4", &xtals_cc4, "xtals_cc4[xtalsMul]/F");
    tree_Combined->Branch("xtals_edop", &xtals_edop, "xtals_edop[xtalsMul]/F");
    tree_Combined->Branch("xtals_edopMaxInt", &xtals_edopMaxInt, "xtals_edopMaxInt[xtalsMul]/F");
    tree_Combined->Branch("xtals_edopSeg", &xtals_edopSeg, "xtals_edopSeg[xtalsMul]/F");
    tree_Combined->Branch("xtals_edopXtal", &xtals_edopXtal, "xtals_edopXtal[xtalsMul]/F");
    tree_Combined->Branch("xtals_crystalNum", &xtals_crystalNum, "xtals_crystalNum[xtalsMul]/I");
    tree_Combined->Branch("xtals_quadNum", &xtals_quadNum, "xtals_quadNum[xtalsMul]/I");
    tree_Combined->Branch("xtals_t0", &xtals_t0, "xtals_t0[xtalsMul]/F");
    tree_Combined->Branch("xtals_timestamp", &xtals_timestamp, "xtals_timestamp[xtalsMul]/L");

    Long64_t count = 0;
    for(auto matchedEvent: matchedEvents_) {
        // Handle ORRUBA
        tree_ORRUBA->GetEntry(matchedEvent.orrubaNumber);

        fRunNumber = *RunNumber;

        fBB10Mul = BB10Mul;
        for(int i = 0; i < fBB10Mul; i++) {
            fBB10Det[i] = BB10Det[i];
            fBB10Strip[i] = BB10Strip[i];
            fBB10Channel[i] = BB10Channel[i];
            fBB10ADC[i] = BB10ADC[i];
            fBB10Energy[i] = BB10Energy[i];
        }

        fQQQ5Mul = QQQ5Mul;
        for(int i = 0; i < fQQQ5Mul; i++) {
            fQQQ5Upstream[i] = QQQ5Upstream[i];
            fQQQ5Det[i] = QQQ5Det[i];
            fQQQ5Ring[i] = QQQ5Ring[i];
            fQQQ5RingChannel[i] = QQQ5RingChannel[i];
            fQQQ5Sector[i] = QQQ5Sector[i];
            fQQQ5SectorChannel[i] = QQQ5SectorChannel[i];
            fQQQ5RingADC[i] = QQQ5RingADC[i];
            fQQQ5RingEnergy[i] = QQQ5RingEnergy[i];
            fQQQ5SectorADC[i] = QQQ5SectorADC[i];
            fQQQ5SectorEnergy[i] = QQQ5SectorEnergy[i];
            fQQQ5Angle[i] = QQQ5Angle[i];
        }

        fSX3Mul = SX3Mul;
        for(int i = 0; i < fSX3Mul; i++) {
            fSX3Upstream[i] = SX3Upstream[i];
            fSX3Det[i] = SX3Det[i];
            fSX3Sector[i] = SX3Sector[i];
            fSX3SectorChannel[i] = SX3SectorChannel[i];
            fSX3SectorADC[i] = SX3SectorADC[i];
            fSX3SectorEnergy[i] = SX3SectorEnergy[i];
            fSX3Strip[i] = SX3Strip[i];
            fSX3StripLeftChannel[i] = SX3StripLeftChannel[i];
            fSX3StripRightChannel[i] = SX3StripRightChannel[i];
            fSX3StripLeftADC[i] = SX3StripLeftADC[i];
            fSX3StripRightADC[i] = SX3StripRightADC[i];
            fSX3StripLeftEnergy[i] = SX3StripLeftEnergy[i];
            fSX3StripRightEnergy[i] = SX3StripRightEnergy[i];
		    fSX3StripEnergy[i] = SX3StripEnergy[i];
			fSX3StripPosition[i] = SX3StripPosition[i];
			fSX3StripPositionCal[i] = SX3StripPositionCal[i];      
        }

        fICdE = ICdE; fICE = ICE; fICWireX = ICWireX; fICWireY = ICWireY;
        fICPositionX = ICPositionX; fICPositionY = ICPositionY; fICPositionWeightedX = ICPositionWeightedX; fICPositionWeightedY = ICPositionWeightedY;

        fTDCIC = TDCIC; fTDCGRETINA = TDCGRETINA; fTDCRF = TDCRF; fTDCSilicon = TDCSilicon;
        fTDCSilicon_Div = TDCSilicon_Div; fTDCSilicon_GRETINA = TDCSilicon_GRETINA; fTDCSilicon_Delay = TDCSilicon_Delay; fTDCSilicon_Upstream = TDCSilicon_Upstream;

        fTimeStamp = TimeStamp; fGRETINA_TimeStamp = matchedEvent.gretinaTimeStamp;
        
        Long64_t delta_TS; // need to make new variable for time stamp difference since TimeStamp is an unsigned integer, so won't go negative.

        // Handle GRETINA
        xtalsMul = 0;
        if(matchedEvent.gretinaTimeStamp > 1) {
            foundGRETINA = true;
            tree_GRETINA->GetEntry(matchedEvent.gretinaNumber);
            for(auto g2Event: g2->xtals) {
                xtals_xlab[xtalsMul] = g2Event.maxIntPtXYZLab().X();
                xtals_ylab[xtalsMul] = g2Event.maxIntPtXYZLab().Y();
                xtals_zlab[xtalsMul] = g2Event.maxIntPtXYZLab().Z();
                xtals_cc[xtalsMul] = g2Event.cc;
                xtals_cc1[xtalsMul] = g2Event.cc1;
                xtals_cc2[xtalsMul] = g2Event.cc2;
                xtals_cc3[xtalsMul] = g2Event.cc3;
                xtals_cc4[xtalsMul] = g2Event.cc4;
                xtals_edop[xtalsMul] = g2Event.edop;
                xtals_edopMaxInt[xtalsMul] = g2Event.edop_maxInt;
                xtals_edopSeg[xtalsMul] = g2Event.edopSeg;
                xtals_edopXtal[xtalsMul] = g2Event.edopXtal;
                xtals_crystalNum[xtalsMul] = g2Event.crystalNum;
                xtals_quadNum[xtalsMul] = g2Event.quadNum;
                xtals_t0[xtalsMul] = g2Event.t0;
                xtals_timestamp[xtalsMul] = g2Event.timestamp;
                //std::cout << xtalsMul << '\t' << xtals_crystalNum[xtalsMul] << '\t' << xtals_cc[xtalsMul] << '\t' << xtals_timestamp[xtalsMul] << std::endl;
                //if(xtalsMul==1) std::cout << xtalsMul << '\t' << xtals_timestamp[xtalsMul] - xtals_timestamp[xtalsMul-1] << std::endl;
                if(xtalsMul>0) TS_diff->Fill(xtals_timestamp[xtalsMul] - xtals_timestamp[xtalsMul-1]);
                delta_TS = TimeStamp - xtals_timestamp[xtalsMul]; // need to make new variable since TimeStamp is an unsigned integer.
                TS_diff2->Fill(delta_TS);
                xtalsMul++;
                
            }
            count++;
        }
        else {
            foundGRETINA = false;
        }

        tree_Combined->Fill();
    }
    size_t nentriesMATCHED = matchedEvents_.size(); //tree_Combined->GetEntries();
    tree_Combined->Write();
    TS_diff->Write(); //Test Histogram
    TS_diff2->Write(); //Test Histogram
    f_Combined->Close();

    f_ORRUBA->Close();
    f_GRETINA->Close();

    delete RunNumber;

    std::cout << PrintOutput("\t\t\tTotal Matched Entries: ", "yellow") << count << std::endl;

    std::cout << PrintOutput("\t\tFinished combining ORRUBA and GRETINA Trees based on time stamps", "blue") << std::endl;
    std::cout << PrintOutput("\t\tCombined TTree 'data' written to file: ", "blue") << run.combinedPath << std::endl;
}

