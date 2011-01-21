#include "graphs.h"

#include <root/TImage.h>
#include <root/TGraph.h>
#include <root/TCanvas.h>
#include <root/TStyle.h>

#include <boost/foreach.hpp>

#include <vector>
#include <fstream>

namespace ajres
{

void
Graphs::plotGraph1D(
	std::vector<dt> const & ybins, uint32 const xSize, uint32 const ySize,
	std::string const & graphTitle, std::string const & fileName,
	bool const dumpBinaryData, bool const logScale
)
{
	std::string const fileNameWithDirectory = std::string("out/") + fileName;

	uint32 const binsCount = ybins.size();
	std::vector<dt> xbins;
	xbins.reserve(binsCount);

	for (uint16 i = 0; i < binsCount; ++i)
	{
		xbins.push_back(i);
	}

	TGraph* g = new TGraph(binsCount, xbins.data(), ybins.data());
	//g->SetXTitle("f * \Delta t");
	//g->SetYTitle("p_{zz}(f)/\sigma^2 a");
	//g->SetStats(1);
	g->SetTitle(graphTitle.c_str()); // "p_{zz}/p_{aa}(f*#Deltat)"

	TCanvas* cv = new TCanvas("Lukasz", "Slusarczyk", xSize, ySize);
	gStyle->SetPalette(1);
	if (logScale) gStyle->SetOptLogy();
	g->Draw("AC");

	::TImage * img = ::TImage::Create();
	img->FromPad(cv);
	img->WriteImage(fileNameWithDirectory.c_str());

	delete img;
	delete cv;
	delete g;

	if (dumpBinaryData)
	{
		std::string const dumpFileName = fileNameWithDirectory + std::string(".txt");
		std::ofstream fStream(dumpFileName.c_str());
		uint32 idx = 0;
		BOOST_FOREACH(dt const graphValue, ybins)
		{
			fStream << idx << " " << graphValue << "\n";
			++ idx;
		}
	}
}

}
