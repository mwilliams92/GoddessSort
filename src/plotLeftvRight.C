{

	TFile *infile = TFile::Open("../OutputFolder/cal228th_315deg_US_BR.root");
	TTree *data = (TTree*)infile->Get("data");

	char cut[64];
	char draw[64];

	TFile *outfile = TFile::Open("LeftvRight.root","RECREATE");
	outfile->cd();
	TH2D *hLeftvRight[12][4];

	for( int i=0; i<12; i++ )	{

		for ( int j=0; j<4; j++ )	{
		
			sprintf(hname,"SX3StripLeftADC:SX3StripRightADC>>hLeftvRight[%d][%d]", i, j);
			sprintf(draw,"SX3StripLeftADC:SX3StripRightADC>>hLeftvRight[%d][%d]", i, j);
			sprintf(cut,"SX3Upstream==1 && SX3Det==%d && SX3Sector==%d", detector, back);
			TCut Cut = cut;

			hLeftvRight[i][j] = new TH2D(hname, hname, 2000, 0, 2000, 2000, 0, 2000);
	
			data->Draw(draw,cut,"goff");


		}

	}

	outfile->cd();
	outfile->Write();
	outfile->Close();	

}
