#include "SimulationValidationModule.h"

using namespace std;


DPP_MODULE_REGISTRATION_IMPLEMENT(SimulationValidationModule,"SimulationValidationModule");
SimulationValidationModule::SimulationValidationModule() : dpp::base_module()
{
  filename_output_="SimulationValidation.root";
}

SimulationValidationModule::~SimulationValidationModule() {
  if (is_initialized()) this->reset();
}

void SimulationValidationModule::initialize(const datatools::properties& myConfig,
                                   datatools::service_manager& flServices,
                                   dpp::module_handle_dict_type& /*moduleDict*/){

  // Look for services
  if (flServices.has("geometry")) {
    const geomtools::geometry_service& GS = flServices.get<geomtools::geometry_service> ("geometry");
    geometry_manager_ = &GS.get_geom_manager();
    DT_THROW_IF(!geometry_manager_,
                std::runtime_error,
                "Null pointer to geometry manager return by geometry_service");
  }
  // Extract the filename_out key from the supplied config, if
  // the key exists. datatools::properties throws an exception if
  // the key isn't in the config, so catch this if thrown and don't do
  // anything
  try {
    myConfig.fetch("filename_out",this->filename_output_);
  } catch (std::logic_error& e) {
  }

  // Use the method of PTD2ROOT to create a root file with just the branches we need for the Validation analysis


  hfile_ = new TFile(filename_output_.c_str(),"RECREATE","Output file of Simulation data");
  hfile_->cd();
  tree_ = new TTree("Validation","Validation");
  tree_->SetDirectory(hfile_);
 
  // Some basic counts
  tree_->Branch("h_vertex_x",&validation_.h_vertex_x_);
  tree_->Branch("h_vertex_y",&validation_.h_vertex_y_);
  tree_->Branch("h_vertex_z",&validation_.h_vertex_z_);
  this->_set_initialized(true);
}
//! [ValidationModule::Process]
dpp::base_module::process_status
SimulationValidationModule::process(datatools::things& workItem) {
  

  // We need to run this before we start populating vectors. Put all your vectors in this function to clear them
  ResetVars();
  
  // Grab calibrated data bank
  // Calibrated data will only be present in reconstructed files,
  // so wrap in a try block



  try {
    // Get (true) primary vertex position
      const mctools::simulated_data& simData = workItem.get<mctools::simulated_data>("SD");
      if (simData.has_data())
      {
        validation_.h_vertex_x_= simData.get_vertex().x();
        validation_.h_vertex_y_= simData.get_vertex().y();
        validation_.h_vertex_z_= simData.get_vertex().z();
      }
  }// end try for SD bank
    catch (std::logic_error& e) {
      //std::cerr << "failed to grab SD bank : " << e.what() << std::endl;
      //return dpp::base_module::PROCESS_ERROR;
      // This is OK, if it's data there will be no SD bank
    }
  tree_->Fill();
  // MUST return a status, see ref dpp::processing_status_flags_type
  return dpp::base_module::PROCESS_OK;
}


void SimulationValidationModule::ResetVars()
{
  return;
}

int SimulationValidationModule::EncodeLocation(const snemo::datamodel::calibrated_tracker_hit & hit)
{
  int encodedLocation=hit.get_layer() + 100 * hit.get_row(); // There are fewer than 100 layers so this is OK
  if (hit.get_side()==0) encodedLocation = (-1 * encodedLocation) - 1;
  // Negative numbers are Italy side, positive are France side
  // layers are 0 to 8 with 0 being at the source foil and 8 by the main wall
  // rows go from 0 (mountain) to 112 (tunnel)
  return encodedLocation;
}


string SimulationValidationModule::EncodeLocation(const snemo::datamodel::calibrated_calorimeter_hit & hit)
{ // Returns the geomID as a string ready for decoding
  std::stringstream buffer;
  buffer << hit.get_geom_id();
  return buffer.str();
}


//! [SimulationValidationModule::reset]
void SimulationValidationModule::reset() {
  hfile_->cd();
  tree_->Write();
  hfile_->Close(); //
  std::cout << "In reset: finished conversion, file closed " << std::endl;

  // clean up
  delete hfile_;
  filename_output_ = "SimulationValidation.root";
  this->_set_initialized(false);

}

