////////////////////////////////////////////////////////////////////////
// Class:       VtxBiasAna
// Plugin Type: analyzer (Unknown Unknown)
// File:        VtxBiasAna_module.cc
//
// Tue Mar 11 2025 Alex Wilkinson
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "canvas/Utilities/InputTag.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include "nusimdata/SimulationBase/MCTruth.h"
#include "lardataobj/RecoBase/Slice.h"
#include "lardataobj/RecoBase/Vertex.h"
#include "lardataobj/RecoBase/PFParticle.h"

#include "art_root_io/TFileService.h"
#include "art_root_io/TFileDirectory.h"
#include "canvas/Persistency/Common/FindManyP.h"

#include <TTree.h>

namespace extrapolation {
  class VtxBiasAna;
}

class extrapolation::VtxBiasAna : public art::EDAnalyzer {
public:
  explicit VtxBiasAna(fhicl::ParameterSet const& p);

  VtxBiasAna(VtxBiasAna const&) = delete;
  VtxBiasAna(VtxBiasAna&&) = delete;
  VtxBiasAna& operator=(VtxBiasAna const&) = delete;
  VtxBiasAna& operator=(VtxBiasAna&&) = delete;

  void analyze(art::Event const& e) override;

  void beginJob() override;
  void endJob() override;

  void reset();

private:
  // Labels
  std::string fVertexLabel;
  std::string fPfpLabel;
  std::string fTruthLabel;

  // TTree stuff
  TTree *fTree;
  double fTrueThetaX;
  double fTrueThetaY;
  double fTrueThetaZ;
  double fTrueNuE;
  double fTrueY;
  double fTrueCC;
  double fTrueMode;
  double fTrueVtxX;
  double fTrueVtxY;
  double fTrueVtxZ;
  double fRecoVtxX;
  double fRecoVtxY;
  double fRecoVtxZ;
};

extrapolation::VtxBiasAna::VtxBiasAna(fhicl::ParameterSet const& p)
  : EDAnalyzer{p},
    fVertexLabel (p.get<std::string>("VertexLabel")),
    fPfpLabel    (p.get<std::string>("PfpLabel")),
    fTruthLabel  (p.get<std::string>("TruthLabel"))
{
}

void extrapolation::VtxBiasAna::analyze(art::Event const& e)
{
  reset();

  //-- Find the neutrino pfp, expect each event ot have exactly one neutrino pfp
  // Get the pfps
  art::ValidHandle<std::vector<recob::PFParticle>> pfpHandle
    {e.getValidHandle<std::vector<recob::PFParticle>>(fPfpLabel)};
  std::vector<art::Ptr<recob::PFParticle>> pfpVector;
  if (pfpHandle.isValid())
  {
    art::fill_ptr_vector(pfpVector, pfpHandle);
  }
  else
  {
    std::cout << "Failed to get pfps!\n";
    return;
  }

  // Find the neutrino pfp
  art::Ptr<recob::PFParticle> nuPfp;
  bool foundNuPfp {false};
  for (const art::Ptr<recob::PFParticle> &pfp : pfpVector)
  {
    const bool isPrimary {pfp->IsPrimary()};
    const bool isNu {(std::abs(pfp->PdgCode()) == 12) || (std::abs(pfp->PdgCode()) == 14)};
    if (!(isPrimary && isNu))
      continue;

    if (foundNuPfp)
    {
      std::cout << "Found more than one neutrino pfp!\n";
      return;
    }
    nuPfp = pfp;
    foundNuPfp = true;
  }
  if (!foundNuPfp)
  {
    std::cout << "Did not find a neutrino pfp!\n";
    return;
  }

  //-- Get the vertex of the reco pfp
  art::FindManyP<recob::Vertex> pfpVertexAssoc(pfpHandle, e, fVertexLabel);
  std::vector<art::Ptr<recob::Vertex>> vertices {pfpVertexAssoc.at(nuPfp.key())};
  if (vertices.size() != 1)
  {
    std::cout << "Expected one vertex, got " << vertices.size() << "\n";
    return;
  }
  const art::Ptr<recob::Vertex> vertex {vertices.at(0)};
  fRecoVtxX = vertex->position().X();
  fRecoVtxY = vertex->position().Y();
  fRecoVtxZ = vertex->position().Z();


  //-- Get the true neutrino vertex
  // Get the mc truth
  art::ValidHandle<std::vector<simb::MCTruth>> truthHandle
    {e.getValidHandle<std::vector<simb::MCTruth>>(fTruthLabel)};
  std::vector<art::Ptr<simb::MCTruth>> truthVector;
  if (truthHandle.isValid())
    art::fill_ptr_vector(truthVector, truthHandle);
  if (truthVector.size() != 1)
  {
    std::cout << "Expected one MCTruth, got " << truthVector.size() << "\n";
    return;
  }

  // Get the true vertex
  const art::Ptr<simb::MCTruth> truth {truthVector.at(0)};
  const simb::MCNeutrino trueNu {truth->GetNeutrino()};
  const simb::MCParticle trueNuParticle {trueNu.Nu()};
  fTrueVtxX = trueNuParticle.Position().X();
  fTrueVtxY = trueNuParticle.Position().Y();
  fTrueVtxZ = trueNuParticle.Position().Z();

  //-- Get the true neutrino properties
  fTrueNuE = trueNuParticle.E(); // GeV I assume
  fTrueY = trueNu.Y();
  fTrueCC = trueNu.CCNC();
  fTrueMode = trueNu.Mode();

  //-- Get the angles w.r.t detector coordinate unit vectors
  fTrueThetaX = std::acos(trueNuParticle.Px() / trueNuParticle.P());
  fTrueThetaY = std::acos(trueNuParticle.Py() / trueNuParticle.P());
  fTrueThetaZ = std::acos(trueNuParticle.Pz() / trueNuParticle.P());

  fTree->Fill();
}

void extrapolation::VtxBiasAna::reset()
{
  fTrueThetaX = -999.;
  fTrueThetaY = -999.;
  fTrueThetaZ = -999.;
  fTrueNuE    = -999.;
  fTrueVtxX   = -999.;
  fTrueVtxY   = -999.;
  fTrueVtxZ   = -999.;
  fRecoVtxX   = -999.;
  fRecoVtxY   = -999.;
  fRecoVtxZ   = -999.;
}

void extrapolation::VtxBiasAna::beginJob()
{
  art::ServiceHandle<art::TFileService> tfs;
  fTree = tfs->make<TTree>("vtx_tree", "vtx_tree");

  fTree->Branch("true_thetax", &fTrueThetaX);
  fTree->Branch("true_thetay", &fTrueThetaY);
  fTree->Branch("true_thetaz", &fTrueThetaZ);
  fTree->Branch("true_nu_energy", &fTrueNuE);
  fTree->Branch("true_Y", &fTrueY);
  fTree->Branch("true_CC", &fTrueCC);
  fTree->Branch("true_mode", &fTrueMode);
  fTree->Branch("true_vtx_x", &fTrueVtxX);
  fTree->Branch("true_vtx_y", &fTrueVtxY);
  fTree->Branch("true_vtx_z", &fTrueVtxZ);
  fTree->Branch("reco_vtx_x", &fRecoVtxX);
  fTree->Branch("reco_vtx_y", &fRecoVtxY);
  fTree->Branch("reco_vtx_z", &fRecoVtxZ);
}

void extrapolation::VtxBiasAna::endJob()
{
}


DEFINE_ART_MODULE(extrapolation::VtxBiasAna)

