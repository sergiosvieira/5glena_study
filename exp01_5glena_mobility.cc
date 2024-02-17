#include "ns3/rng-seed-manager.h"
#include "ns3/mobility-helper.h"
#include "ns3/ns2-mobility-helper.h"
#include "ns3/config.h"
#include "ns3/uinteger.h"
#include "ns3/boolean.h"
#include "ns3/nr-point-to-point-epc-helper.h"
#include "ns3/ideal-beamforming-helper.h"
#include "ns3/nr-helper.h"
#include <filesystem>

using namespace ns3;

int main (int argc, char *argv[]) {
    static const uint8_t gNB_total = 2;
    // 1. Randomize
    LogComponentEnable("RngSeedManager", LOG_LEVEL_ALL);
	RngSeedManager::SetSeed (1);
	RngSeedManager::SetRun (1);
    // 2. Create nodes to attach UEs
    NodeContainer ues;
    ues.Create(50);
    // 3. Load mobility from tcl file
    LogComponentEnable("Ns2MobilityHelper", LOG_LEVEL_ALL);
    std::filesystem::path home_path = "./scratch/one_v2x";
    std::filesystem::path mobi_path = home_path / "mobility";
	Ns2MobilityHelper ns2 = Ns2MobilityHelper (mobi_path / "mob01.tcl");
	ns2.Install (ues.Begin(), ues.End());
    // 4. Create gNBs
    NodeContainer gnbs;
    gnbs.Create(gNB_total);
    // 5. Define position of gNBs
	MobilityHelper gnbs_mobility;
    gnbs_mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    Ptr<ListPositionAllocator> gnbs_pos_allocator = CreateObject<ListPositionAllocator>();
    gnbs_pos_allocator->Add(Vector(1000.0, 995.0, 0.0));
    gnbs_pos_allocator->Add(Vector(3000.0, 995.0, 0.0));
    gnbs_mobility.SetPositionAllocator(gnbs_pos_allocator);
    gnbs_mobility.Install(gnbs);
    // 6. 5G-LENA
    /**
    * LTE RLC (Radio Link Control) Unacknowledged Mode (UM), see 3GPP TS 36.322
    * PDU - Protocol Data Unit
    * O RLC (Radio Link Control) é um protocolo de camada de enlace de rádio usado 
    * em redes móveis, como as redes LTE (Long Term Evolution) e UMTS (Universal 
    * Mobile Telecommunications System). Ele é responsável pela segmentação, 
    * retransmissão e controle de fluxo de dados entre a camada de enlace de rádio e 
    * a camada de rede.
    * Uma PDU (Protocol Data Unit) é uma unidade de dados definida em um protocolo 
    * de comunicação. No contexto do RLC, uma RLC PDU é uma unidade de dados 
    * específica do protocolo RLC que contém informações de controle e/ou carga útil 
    * de usuário. Essas PDUs podem incluir dados de usuário transmitidos sobre o 
    * enlace de rádio, bem como informações de controle para gerenciamento de conexão 
    * e controle de fluxo.
    * Em UM (Unacknowledged Mode), os PDUs (Protocol Data Units) são transmitidos do 
    * transmissor para o receptor sem que haja confirmação explícita de sua entrega. 
    * Isso significa que não há controle de erro ou retransmissão de pacotes perdidos. 
    * Este modo é geralmente utilizado para transmissão de dados em tempo real, como 
    * streaming de mídia ou voz, onde a latência é crítica e pequenas perdas de dados 
    * podem ser toleradas.
    */
    /*
     * In general, attributes for the NR module are typically configured in NrHelper.  
     * However, some attributes need to be configured globally through the 
     * Config::SetDefault() method. Below is an example: if you want to make the RLC 
     * buffer very large, you can pass a very large integer here.
        uint32_t m_maxTxBufferSize; ///< maximum transmit buffer status
        uint32_t m_txBufferSize;    ///< transmit buffer size
     */
    Config::SetDefault("ns3::LteRlcUm::MaxTxBufferSize", UintegerValue(999999999));
    Ptr<NrPointToPointEpcHelper> epcHelper = CreateObject<NrPointToPointEpcHelper>();
    Ptr<IdealBeamformingHelper> idealBeamformingHelper = CreateObject<IdealBeamformingHelper>();
    Ptr<NrHelper> nrHelper = CreateObject<NrHelper>();
    // Put the pointers inside nrHelper
    nrHelper->SetBeamformingHelper(idealBeamformingHelper);
    nrHelper->SetEpcHelper(epcHelper);
    /*
     * Spectrum division. We create two operational bands, each of them containing
     * one component carrier, and each CC containing a single bandwidth part
     * centered at the frequency specified by the input parameters.
     * Each spectrum part length is, as well, specified by the input parameters.
     * Both operational bands will use the StreetCanyon channel modeling.
     */
    BandwidthPartInfoPtrVector allBwps;
    CcBwpCreator ccBwpCreator;
    const uint8_t numCcPerBand = 1; // in this example, both bands have a single CC
    // Create the configuration for the CcBwpHelper. SimpleOperationBandConf creates
    // a single BWP per CC
    uint16_t numerologyBwp1 = 4;
    double centralFrequencyBand1 = 28e9;
    double bandwidthBand1 = 100e6;
    double totalTxPower = 4;
    CcBwpCreator::SimpleOperationBandConf bandConf1(centralFrequencyBand1,
                                                    bandwidthBand1,
                                                    numCcPerBand,
                                                    BandwidthPartInfo::UMi_StreetCanyon);
    // By using the configuration created, it is time to make the operation bands
    OperationBandInfo band1 = ccBwpCreator.CreateOperationBandContiguousCc(bandConf1);
    Config::SetDefault("ns3::ThreeGppChannelModel::UpdatePeriod", TimeValue(MilliSeconds(0)));
    nrHelper->SetChannelConditionModelAttribute("UpdatePeriod", TimeValue(MilliSeconds(0)));
    nrHelper->SetPathlossAttribute("ShadowingEnabled", BooleanValue(false));
    /*
     * Initialize channel and pathloss, plus other things inside band1. If needed,
     * the band configuration can be done manually, but we leave it for more
     * sophisticated examples. For the moment, this method will take care
     * of all the spectrum initialization needs.
     */
    nrHelper->InitializeOperationBand(&band1);
    /*
     * Start to account for the bandwidth used by the example, as well as
     * the total power that has to be divided among the BWPs.
     */
    double x = pow(10, totalTxPower / 10);
    double totalBandwidth = bandwidthBand1;
    allBwps = CcBwpCreator::GetAllBwps({band1});
   /*
     * allBwps contains all the spectrum configuration needed for the nrHelper.
     *
     * Now, we can setup the attributes. We can have three kind of attributes:
     * (i) parameters that are valid for all the bandwidth parts and applies to
     * all nodes, (ii) parameters that are valid for all the bandwidth parts
     * and applies to some node only, and (iii) parameters that are different for
     * every bandwidth parts. The approach is:
     *
     * - for (i): Configure the attribute through the helper, and then install;
     * - for (ii): Configure the attribute through the helper, and then install
     * for the first set of nodes. Then, change the attribute through the helper,
     * and install again;
     * - for (iii): Install, and then configure the attributes by retrieving
     * the pointer needed, and calling "SetAttribute" on top of such pointer.
     *
     */    
    return 0;
}