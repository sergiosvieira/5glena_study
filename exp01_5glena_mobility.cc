#include "ns3/rng-seed-manager.h"
#include "ns3/mobility-helper.h"
#include "ns3/ns2-mobility-helper.h"
#include "ns3/config.h"
#include "ns3/uinteger.h"
#include "ns3/boolean.h"
#include "ns3/double.h"
#include "ns3/nr-point-to-point-epc-helper.h"
#include "ns3/ideal-beamforming-helper.h"
#include "ns3/nr-helper.h"
#include <ns3/cc-bwp-helper.h>
#include <ns3/pointer.h>
#include <ns3/isotropic-antenna-model.h> 
#include <filesystem>

using namespace ns3;

int main (int argc, char *argv[]) {
    static const uint8_t gNB_total = 2;
    // 1. Randomize
    // LogComponentEnable("RngSeedManager", LOG_LEVEL_ALL);
	RngSeedManager::SetSeed (1);
	RngSeedManager::SetRun (1);
    // 2. Create nodes to attach UEs
    NodeContainer ues;
    ues.Create(50);
    // 3. Load mobility from tcl file
    // LogComponentEnable("Ns2MobilityHelper", LOG_LEVEL_ALL);
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
    /*
     * In general, attributes for the NR module are typically configured in NrHelper.  However, some
     * attributes need to be configured globally through the Config::SetDefault() method. Below is
     * an example: if you want to make the RLC buffer very large, you can pass a very large integer
     * here.
     */
    Config::SetDefault("ns3::LteRlcUm::MaxTxBufferSize", UintegerValue(999999999));
    /*
     * No contexto de 5G, EPC significa "Evolved Packet Core". 
     * É um componente fundamental da arquitetura de rede 5G, 
     * responsável por gerenciar e controlar o tráfego de dados, 
     * sinalização e mobilidade dos usuários.
     * EPC em 4G vs. 5G:
     * - 4G: O EPC é o núcleo da rede 4G LTE. Ele é responsável por funções 
     * como autenticação de usuário, gerenciamento de sessões, roteamento 
     * de dados e entrega de serviços.
     * - 5G: O 5G introduz um novo núcleo de rede, chamado 5GC 
     * (5G Core Network). O 5GC oferece maior flexibilidade, escalabilidade 
     * e desempenho em comparação ao EPC.
     * Duas opções de implementação:
     * - 5G não autônomo (NSA): O 5G NSA usa o EPC do 4G em conjunto com a 
     * nova RAN (Radio Access Network) 5G. Essa opção oferece uma migração 
     * gradual para o 5G, aproveitando a infraestrutura 4G existente.
     * - 5G autônomo (SA): O 5G SA utiliza um 5GC totalmente novo, 
     * independente do EPC. Essa opção oferece os benefícios completos do 
     * 5G, mas requer uma nova infraestrutura de rede.
     * EPC no futuro:
     * - O EPC continuará a ser usado em redes 4G por muitos anos. 
     * O 5GC gradualmente substituirá o EPC à medida que as redes 5G SA 
     * forem implantadas.
     * O EPC e o 5GC podem coexistir por um período de tempo, permitindo 
     * uma migração suave para o 5G.    
     * 
     * Principais elementos do EPC:
     * - MME (Mobility Management Entity): Responsável por gerenciar a 
     * mobilidade dos usuários, como autenticação, registro, atualização 
     * de localização e entrega de serviços.
     * - S-GW (Serving Gateway): Roteia o tráfego de dados entre o usuário e 
     * a rede de dados.
     * - P-GW (Packet Gateway): Conecta a rede EPC à internet e outros 
     * provedores de serviços.
     * - HSS (Home Subscriber Server): Armazena informações de perfil e 
     * autenticação do usuário.
     * - PCRF (Policy and Charging Rules Function): Define e aplica 
     * políticas de uso de dados e cobrança.
     * 
     * Funcionamento do EPC:
     * - Autenticação e registro: Quando um usuário se conecta à rede, 
     * o MME autentica o dispositivo e o registra na rede.
     * - Gerenciamento de mobilidade: O MME monitora a localização do 
     * usuário e atualiza a rede quando o usuário se move entre diferentes 
     * células.
     * - Roteamento de dados: O S-GW roteia o tráfego de dados entre o 
     * usuário e a rede de dados.
     * - Conexão à internet: O P-GW conecta a rede EPC à internet e outros 
     * provedores de serviços. 
     * - Controle de acesso e cobrança: O PCRF define e aplica políticas 
     * de uso de dados e cobrança.
     
     * Benefícios do EPC:
     * - Gerenciamento eficiente de tráfego: O EPC otimiza o uso de 
     * recursos da rede e garante um bom desempenho para todos os usuários.
     * - Segurança aprimorada: O EPC oferece mecanismos de segurança 
     * robustos para proteger os dados dos usuários contra acessos não 
     * autorizados.
     * - Escalabilidade: O EPC pode ser facilmente dimensionado para 
     * atender à crescente demanda por serviços móveis.
    * This helper will then used to create the links between the GNBs 
    * and the EPC.
    * All links will be point-to-point, with some properties.
    * The user can set the point-to-point links properties by using:
    */
    LogComponentEnable("NrPointToPointEpcHelper", LOG_LEVEL_ALL);
    Ptr<NrPointToPointEpcHelper> epcHelper = CreateObject<NrPointToPointEpcHelper>();
    /**
    *  
    * Beamforming Method: 
    * - Descrição: O Beamforming é uma técnica para focar o sinal 
    * de rádio de uma torre de celular (estação base) em um 
    * dispositivo específico, em vez de transmiti-lo em todas 
    * as direções. Isso é feito usando múltiplas antenas na estação 
    * base e manipulando a fase e amplitude dos sinais emitidos por 
    * cada antena.
    * - Benefícios: O Beamforming melhora a relação sinal-ruído (SNR) 
    * para o dispositivo alvo, aumentando a velocidade de transmissão 
    * de dados, reduzindo a interferência e aumentando a eficiência 
    * energética.
    * Tipos: 
    * - 4G: Beamforming baseado em ângulo de chegada (AoA) estima a 
    * direção do dispositivo e orienta o feixe nessa direção.
    * - 5G: Beamforming baseado em canal utiliza informações do 
    * canal de comunicação para adaptar o feixe de forma dinâmica.
    - Impacto: O Beamforming é uma tecnologia crítica para as 
    altas taxas de dados e baixa latência prometidas pelo 5G.
    *
    * Cell Scan:
    * - Descrição: O Cell Scan é um processo realizado pelo 
    * dispositivo móvel para descobrir e selecionar a melhor torre 
    * de celular (estação base) para se conectar. O dispositivo 
    * escuta os sinais de rádio de todas as estações base próximas 
    * e avalia fatores como força do sinal, qualidade do canal e 
    * carga da rede.
    * - Benefícios: O Cell Scan garante que o dispositivo esteja 
    * sempre conectado à melhor estação base disponível, otimizando 
    * a qualidade da conexão e a duração da bateria.
    * 
    * The BeamformingHelperBase class that is being
    * used as the general interface for beamforming helper
    * classes. Currently, there are two beamforming helper classes:
    * `IdealBeamformingHelper` and `RealisticBeamformingHelper`
    * that inherit this base beamforming helper class

    */
    Ptr<IdealBeamformingHelper> idealBeamformingHelper = CreateObject<IdealBeamformingHelper>();

    /* This class will help you in setting up a single- or multi-cell scenario
    * with NR. Most probably, you will interact with the NR module only through
    * this class, so make sure everthing that is written in the following is
    * clear enough to start creating your own scenario.
    *
    * \section helper_pre Pre-requisite: Node creation and placement
    *
    * We assume that you read the ns-3 tutorials, and you're able to create
    * your own node placement, as well as the mobility model that you prefer
    * for your scenario. For simple cases, we provide a class that can help you
    * in setting up a grid-like scenario. Please take a look at the
    * GridScenarioHelper documentation in that case.
    * 
    */
    Ptr<NrHelper> nrHelper = CreateObject<NrHelper>();
    // Put the pointers inside nrHelper
    nrHelper->SetBeamformingHelper(idealBeamformingHelper);
    nrHelper->SetEpcHelper(epcHelper);
    /* The spectrum management is delegated to the class CcBwpHelper. Please
    * refer to its documentation to have more information. After having divided
    * the spectrum in component carriers and bandwidth part, use the method
    * InitializeOperationBand() to create the channels and other things that will
    * be used by the channel modeling part.
    */
//    Ptr<CcBwpHelper> ccBwpHelper = CreateObject<CcBwpHelper>;

    /**
     * \ingroup helper
     * \brief Spectrum part - BandwidthPartInfo
     *
     * This is the minimum unit of usable spectrum by a PHY class. For creating
     * any GNB or UE, you will be asked to provide a list of BandwidthPartInfo
     * to the methods NrHelper::InstallGnbDevice() and NrHelper::InstallUeDevice().
     * The reason is that the helper will, for every GNB and UE in the scenario,
     * create a PHY class that will be attached to the channels included in this struct.
     *
     * For every bandwidth part (in this context, referred to a spectrum part) you
     * have to indicate the central frequency and the higher/lower frequency, as
     * well as the entire bandwidth plus the modeling.
     *
     * The pointers to the channels, if left empty, will be initialized by
     * NrHelper::InitializeOperationBand().
     * 
     * 
    struct BandwidthPartInfo {
        uint8_t m_bwpId{0};             //!< BWP id
        double m_centralFrequency{0.0}; //!< BWP central frequency
        double m_lowerFrequency{0.0};   //!< BWP lower frequency
        double m_higherFrequency{0.0};  //!< BWP higher frequency
        double m_channelBandwidth{0.0}; //!< BWP bandwidth
    }
    * 
    * No contexto de 5G, BWP significa Bandwidth Part (Parte de Largura de Banda). 
    * É um conceito fundamental que permite a alocação flexível de recursos de canal 
    * para dispositivos individuais. Vamos destrinchar isso:
    * O que é um BWP?
    * - Imagine a largura de banda total disponível em uma célula 5G como uma estrada larga.
    * Um BWP é como uma pista específica dentro dessa estrada, dedicada a um determinado 
    * dispositivo ou grupo de dispositivos. O tamanho do BWP pode ser ajustado de acordo 
    * com as necessidades desses dispositivos, variando de uma pequena fração da largura 
    * de banda total até toda a largura disponível.
    *
    * Por que usar BWPs? 
    * - Flexibilidade: Permite adaptar a alocação de recursos de acordo com as diferentes 
    * demandas dos dispositivos (alta velocidade, baixa latência, IoT, etc.). 
    * - Eficiência: Evita o desperdício de recursos alocados a dispositivos ociosos.
    * - Suporte a múltiplos dispositivos: Permite atender às necessidades de um grande 
    * número de dispositivos simultaneamente, mesmo com requisitos variados.
    * - QoS garantida: Garante qualidade de serviço diferenciada para diferentes tipos 
    * de tráfego.
    * 
    * Tipos de BWP:
    * - DL BWP (Downlink): Alocado para tráfego recebido pelo dispositivo (downloads).
    * - UL BWP (Uplink): Alocado para tráfego enviado pelo dispositivo (uploads).
    * - Contiguos: Todos os recursos do BWP estão agrupados em bloco.
    * - Não contiguos: Recursos do BWP podem estar espalhados por partes diferentes 
    * da largura de banda total.
    * 
    * Como funciona o BWP?
    * 
    * A rede 5G analisa as necessidades de cada dispositivo conectado. Com base nessas 
    * necessidades, a rede atribui um ou mais BWPs adequados a cada dispositivo. O 
    * dispositivo usa apenas os recursos alocados pelo BWP atribuído. A rede pode 
    * dinamicamente ajustar os BWPs dos dispositivos de acordo com a mudança das demandas.
    * 
    * Benefícios do BWP no 5G:
    * - Suporta uma ampla variedade de aplicativos e serviços.
    * - Permite uma melhor utilização dos recursos de rede.
    * - Garante uma melhor experiência de usuário para todos os dispositivos conectados.
    * 
    * Component Carrier (CC) no Contexto de 5G
    * 
    * O que é um CC?
    * 
    * Um Component Carrier (CC) é uma porção individual de espectro dentro de uma banda 
    * de frequência mais ampla utilizada em redes de comunicação móvel, como 5G NR 
    * (New Radio). É similar a um canal individual dentro de uma estrada de múltiplas 
    * faixas.
    * 
    * Características:
    * 
    * - Tamanho: A largura de banda de um CC pode variar, geralmente de alguns MHz até 
    * 100 MHz ou mais.
    * - Frequência: A frequência central do CC pode ser definida dentro da banda de 
    * frequência disponível.
    * - Agregação: Vários CCs podem ser agregados para aumentar a largura de banda total 
    * disponível para um dispositivo ou serviço.
    * - Flexibilidade: Permite alocação dinâmica de recursos de acordo com as necessidades do tráfego.
    * Benefícios:
    * - Maior capacidade: Permite que mais dispositivos se conectem à rede e transmitam dados 
    * simultaneamente.
    * - Melhor desempenho: Aumento da velocidade de dados e da taxa de transferência.
    * - Menor latência: Redução do tempo de espera para comunicação entre dispositivos.
    * - Maior confiabilidade: Maior robustez contra interferências e melhor cobertura.
    * 
    * Aplicações:
    * 
    * - 5G NR: Essencial para a implementação de 5G NR, permitindo explorar diferentes 
    * bandas de frequência e otimizar o uso de recursos.
    * - IoT: Suporte a um grande número de dispositivos IoT com diferentes requisitos de 
    * comunicação.
    * - Veículos conectados: Permite comunicação de alta velocidade e baixa latência para 
    * veículos autônomos e conectados.
    * 
    * Exemplo:
    * 
    * Imagine uma banda de frequência de 500 MHz a 600 MHz. Um operador de rede pode dividir 
    * essa banda em dois CCs:
    * - CC1: 500 MHz a 550 MHz (50 MHz de largura de banda)
    * - CC2: 550 MHz a 600 MHz (50 MHz de largura de banda)
    * Esses CCs podem ser usados para diferentes tipos de serviços, como:
    * - CC1: Transmissão de dados para smartphones
    * - CC2: Conexões de IoT para dispositivos em uma fábrica
    * 
    * Considerações:
    * 
    * O número e a largura de banda dos CCs dependem da disponibilidade de espectro e 
    * das necessidades da rede. A agregação de CCs exige suporte do dispositivo e da rede.
    * O uso eficiente de CCs requer técnicas avançadas de gerenciamento de recursos de rede.
    * 
    * Conclusão:
    * 
    * Component Carrier é uma tecnologia fundamental para o 5G NR, permitindo maior 
    * flexibilidade, capacidade e desempenho nas redes móveis do futuro.
    * 
    */

    /*
     * Spectrum division. We create two operational bands, each of them containing
     * one component carrier, and each CC containing a single bandwidth part
     * centered at the frequency specified by the input parameters.
     * Each spectrum part length is, as well, specified by the input parameters.
     * Both operational bands will use the StreetCanyon channel modeling.
     */
    BandwidthPartInfoPtrVector allBwps;
    /**
     * \ingroup helper
     * \brief Manages the correct creation of operation bands, component carriers (CC)
     * and bandwidth parts (BWP)
     *
     * This class can be used to setup in an easy way the operational bands needed
     * for a simple scenario. The first thing is to setup a simple configuration,
     * specified by the struct SimpleOperationBandConf. Then, this configuration can
     * be passed to CreateOperationBandContiguousCc.
     */    
    CcBwpCreator ccBwpCreator;
    const uint8_t numCcPerBand = 1; // in this example, both bands have a single CC
    // Create the configuration for the CcBwpHelper. SimpleOperationBandConf creates
    // a single BWP per CC
    /* 
    * Numerology:
    *
    * Refere-se ao conjunto de parâmetros que define as características básicas da 
    * transmissão em 5G NR, como: 
    * - Subcarrier spacing: Distância entre cada subcarrier de frequência na banda 
    * de transmissão.
    * - Cyclic prefix length: Inserção de símbolos redundantes no início de cada 
    * pacote para compensar atrasos de canal.
    * 
    * Numerology BWP igual a 4:
    * - Significa que o BWP específico utiliza uma numerologia com subcarrier spacing 
    * de 120 kHz e cyclic prefix length de 1,4 μs.
    * 
    * Esta é uma das numerologias padrão definidas no 5G NR, frequentemente empregada 
    * em cenários que buscam balancear capacidade de dados, latência e cobertura.
    * 
    * Benefícios da numerologia BWP igual a 4:
    * 
    * - Taxas de dados moderadas: Permite atingir velocidades de dados decentes, 
    * adequadas para muitos aplicativos como streaming de vídeo e navegação na web.
    * - Latência aceitável: Oferece latência moderada, suportando aplicações 
    * interativas como jogos online e videochamadas.
    * - Boa cobertura: Funciona bem em diversas condições de propagação, garantindo 
    * cobertura ampla.
    * 
    * Limitações:
    * 
    * Não é a opção mais rápida: Numerologias com menor subcarrier spacing (ex: 30 kHz) 
    * podem atingir velocidades mais altas, mas com menor cobertura e maior sensibilidade 
    * à interferência. Latência não é a mais baixa: Para aplicações críticas em tempo real, 
    * numerologias com menor prefixo cíclico podem oferecer latência ainda menor.
    * 
    * Em resumo:
    * "Numerology BWP igual a 4" indica um BWP em 5G NR utilizando uma numerologia específica 
    * que oferece um bom equilíbrio entre taxas de dados, latência e cobertura, sendo adequada 
    * para diversas aplicações.
    * 
    */
    uint16_t numerologyBwp1 = 4;
    double centralFrequencyBand1 = 28e9;
    double bandwidthBand1 = 1e8;
    double totalTxPower = 4;
    /** 
     * \param centralFreq Central Frequency
     * \param channelBw Bandwidth
     * \param numCc number of component carriers in this operation band
     * \param scenario which 3gpp scenario path loss model will be installed for this band
     */    
    CcBwpCreator::SimpleOperationBandConf bandConf1(centralFrequencyBand1,
                                                    bandwidthBand1,
                                                    numCcPerBand,
                                                    BandwidthPartInfo::UMi_StreetCanyon);
    /**
     * \ingroup utils
     * \brief Operation band information structure
     *
     * Defines the range of frequencies of an operation band and includes a list of
     * component carriers (CC) and their contiguousness
     */                                                    
    // By using the configuration created, it is time to make the operation bands
    LogComponentEnable("CcBwpHelper", LOG_LEVEL_ALL);
    OperationBandInfo band1 = ccBwpCreator.CreateOperationBandContiguousCc(bandConf1);
    /*
    * Período de atualização de canal no contexto de 5G NR
    * No contexto de 5G NR, o período de atualização de canal se refere à 
    * frequência com que a informação sobre as condições do canal de 
    * comunicação é atualizada.
    * 
    * Explicação:
    * 
    * O canal de comunicação pode ser afetado por diversos fatores, como 
    * interferência, obstruções e mudanças no ambiente. Para garantir 
    * uma comunicação eficiente, a rede precisa monitorar as condições 
    * do canal e ajustar seus parâmetros de acordo. O período de 
    * atualização de canal define a frequência com que essa monitoração 
    * e ajuste são realizados.
    * 
    * Parâmetros do canal:
    * 
    * - Condições de propagação: Força do sinal, interferência, ruído, etc.
    * - Estimativa do canal: Informações sobre as características do canal,
    * como resposta em frequência e tempo.
    * - Matriz de pré-codificação: Técnica para otimizar a transmissão de 
    * dados, levando em conta as condições do canal.
    * 
    * Período de atualização de canal curto:
    * 
    * - Permite que a rede se adapte rapidamente às mudanças nas condições 
    * do canal, garantindo um desempenho mais constante. Útil em ambientes 
    * com alta dinâmica, como áreas urbanas com grande número de 
    * dispositivos móveis. Pode aumentar o consumo de energia e a carga 
    * de processamento na rede.
    * 
    * Período de atualização de canal longo:
    * 
    * Reduz o consumo de energia e a carga de processamento na rede. 
    * Pode levar a um desempenho menos constante em ambientes com alta 
    * dinâmica. Adequado para ambientes estáveis, como áreas rurais com 
    * baixa densidade de dispositivos. A escolha do período de 
    * atualização de canal ideal depende de diversos fatores:
    * - Tipo de ambiente
    * - Densidade de dispositivos
    * - Tipo de tráfego
    * - Recursos da rede
    * 
    * Em resumo:
    * 
    * O período de atualização de canal é um parâmetro importante no 
    * 5G NR que permite que a rede se adapte às mudanças nas condições 
    * do canal e garanta um bom desempenho da comunicação.
    */
    
    /* \brief Channel Matrix Generation following 3GPP TR 38.901
    *
    * The class implements the channel matrix generation procedure
    * described in 3GPP TR 38.901.    
    * 
    */
    Config::SetDefault("ns3::ThreeGppChannelModel::UpdatePeriod", TimeValue(MilliSeconds(0)));
    nrHelper->SetChannelConditionModelAttribute("UpdatePeriod", TimeValue(MilliSeconds(0)));
    /*
    * Shadowing Enabled em 5G NR
    * No contexto de 5G NR, "Shadowing Enabled" indica que a simulação 
    * leva em consideração o fenômeno de sombreamento.
    * 
    * O que é sombreamento?
    * 
    * Sombreamento é um efeito de atenuação do sinal sem fio causado 
    * por obstruções no ambiente, como edifícios, árvores ou colinas. 
    * Ele pode causar variações significativas na força do sinal 
    * recebido por um dispositivo móvel em diferentes locais, mesmo que 
    * estejam relativamente próximos.
    * 
    * Por que simular o sombreamento?
    * 
    * Para avaliar a cobertura real da rede 5G NR em diferentes condições 
    * de propagação. Para identificar áreas com potencial deficiência de 
    * sinal devido ao sombreamento. Para otimizar o planejamento e o 
    * dimensionamento da rede, como a localização de estações base. Para 
    * desenvolver e testar algoritmos de compensação de sombreamento, como 
    * a diversidade de antenas.
    * 
    * Benefícios de ativar o sombreamento:
    * 
    * - Resultados mais realistas: A simulação considera um fator 
    * importante que afeta a propagação do sinal, tornando os resultados 
    * mais confiáveis.
    * - Melhora na previsão de desempenho: Permite identificar potenciais 
    * problemas de cobertura antes da implementação da rede.
    * - Otimização do planejamento: Auxilia na alocação eficiente de 
    * recursos para garantir uma cobertura uniforme.
    * 
    * Limitações de ativar o sombreamento:
    * 
    * - Aumenta a complexidade da simulação: Requer modelos matemáticos 
    * mais complexos, aumentando o tempo e o custo de simulação. 
    * - Necessita de dados precisos: A precisão dos resultados depende 
    * da disponibilidade de dados topográficos e ambientais detalhados.
    * 
    * Conclusão:
    * 
    * "Shadowing Enabled" é uma opção importante em simulações de redes 
    * 5G NR, pois permite avaliar o impacto do sombreamento na cobertura 
    * e desempenho da rede. No entanto, é preciso considerar a 
    * complexidade e a necessidade de dados precisos antes de habilitar 
    * esta opção.
    */
    nrHelper->SetPathlossAttribute("ShadowingEnabled", BooleanValue(false));
    /*
     * Initialize channel and pathloss, plus other things inside band1. 
     * If needed, the band configuration can be done manually, but we leave 
     * it for more sophisticated examples. For the moment, this method 
     * will take care of all the spectrum initialization needs.
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
     * allBwps contains all the spectrum configuration needed for the 
     * nrHelper.
     *
     * Now, we can setup the attributes. We can have three kind of 
     * attributes:
     * (i) parameters that are valid for all the bandwidth parts and 
     * applies to all nodes, 
     * (ii) parameters that are valid for all the bandwidth parts
     * and applies to some node only, and 
     * (iii) parameters that are different for every bandwidth parts. 
     * The approach is:
     *
     * - for (i): Configure the attribute through the helper, and then 
     * install;
     * - for (ii): Configure the attribute through the helper, and then 
     * install
     * for the first set of nodes. Then, change the attribute through 
     * the helper, and install again;
     * - for (iii): Install, and then configure the attributes by 
     * retrieving the pointer needed, and calling "SetAttribute" on top 
     * of such pointer.
     *
     */

    /**
     * \ingroup packet
     * \brief network packets
     *
     * Each network packet contains a byte buffer, a set of byte tags, a set of
     * packet tags, and metadata.
     *
     * - The byte buffer stores the serialized content of the headers and trailers
     * added to a packet. The serialized representation of these headers is expected
     * to match that of real network packets bit for bit (although nothing
     * forces you to do this) which means that the content of a packet buffer
     * is expected to be that of a real packet.
     *
     * - The metadata describes the type of the headers and trailers which
     * were serialized in the byte buffer. The maintenance of metadata is
     * optional and disabled by default. To enable it, you must call
     * Packet::EnablePrinting and this will allow you to get non-empty
     * output from Packet::Print. If you wish to only enable
     * checking of metadata, and do not need any printing capability, you can
     * call Packet::EnableChecking: its runtime cost is lower than
     * Packet::EnablePrinting.
     *
     * - The set of tags contain simulation-specific information which cannot
     * be stored in the packet byte buffer because the protocol headers or trailers
     * have no standard-conformant field for this information. So-called
     * 'byte' tags are used to tag a subset of the bytes in the packet byte buffer
     * while 'packet' tags are used to tag the packet itself. The main difference
     * between these two kinds of tags is what happens when packets are copied,
     * fragmented, and reassembled: 'byte' tags follow bytes while 'packet' tags
     * follow packets. Another important difference between these two kinds of tags
     * is that byte tags cannot be removed and are expected to be written once,
     * and read many times, while packet tags are expected to be written once,
     * read many times, and removed exactly once. An example of a 'byte'
     * tag is a FlowIdTag which contains a flow id and is set by the application
     * generating traffic. An example of a 'packet' tag is a cross-layer
     * qos class id set by an application and processed by a lower-level MAC
     * layer.
     *
     * Implementing a new type of Header or Trailer for a new protocol is
     * pretty easy and is a matter of creating a subclass of the ns3::Header
     * or of the ns3::Trailer base class, and implementing the methods
     * described in their respective API documentation.
     *
     * Implementing a new type of Tag requires roughly the same amount of
     * work and this work is described in the ns3::Tag API documentation.
     *
     * The performance aspects copy-on-write semantics of the
     * Packet API are discussed in \ref packetperf
     */
    Packet::EnableChecking();
    Packet::EnablePrinting();    
   /*
     *  Case (i): Attributes valid for all the nodes
     */
    /*
    * A frase "The delay to be used for the next S1-U link to be created" se refere ao tempo de atraso que será atribuído à próxima conexão entre o dispositivo móvel e a rede central (EPC) em uma rede 4G LTE. Esse atraso é configurado no Point-to-Point EpcHelper, uma ferramenta usada para simular redes móveis em softwares como o ns-3.
    * Aqui está uma explicação mais detalhada do contexto:
    * Comunicações móveis 4G LTE:
    * 
    * S1-U interface: Conecta o dispositivo móvel (UE) à rede central 
    * (EPC) para serviços de dados do usuário. EPC (Evolved Packet Core): 
    * É o "cérebro" da rede 4G, responsável por gerenciar o tráfego de 
    * dados, chamadas e mensagens. 
    * 
    * Point-to-Point EpcHelper: Classe no ns3 que permite 
    * configurar uma rede EPC básica.
    * 
    * Significância do atraso:
    * 
    * O atraso na interface S1-U afeta a latência geral da comunicação 
    * entre o dispositivo e a rede. Um atraso baixo é desejável para 
    * aplicações sensíveis à latência, como jogos online e realidade 
    * virtual. Um atraso alto pode prejudicar a experiência do usuário, 
    * aumentando o tempo de resposta e causando lentidão.
    * 
    * Como o valor do atraso é definido:
    * 
    * O Point-to-Point EpcHelper permite configurar um valor específico 
    * para o atraso da interface S1-U. Esse valor pode ser determinado 
    * com base em:
    * 
    * - Requisitos de latência da aplicação.
    * - Características da rede física (ex: distância entre o 
    * dispositivo e a estação base).
    * - Modelos de atraso baseados em medições reais.
    * 
    * Exemplo:
    * 
    * Se você estiver simulando uma rede 4G para jogos online, um valor 
    * de atraso baixo (ex: 20ms) seria importante para garantir uma 
    * jogabilidade fluida. Ao simular uma rede rural com longas 
    * distâncias entre dispositivos e estações base, um valor de 
    * atraso maior (ex: 50ms) poderia ser mais realista.
    */
    // Core latency
    epcHelper->SetAttribute("S1uLinkDelay", TimeValue(MilliSeconds(0)));

    /*
     * Antennas for all the UEs
     * We are not using beamforming in SL, rather we are using
     * quasi-omnidirectional transmission and reception, 
     * which is the default configuration of the beams.
     */
    nrHelper->SetUeAntennaAttribute("NumRows", UintegerValue(1));
    nrHelper->SetUeAntennaAttribute("NumColumns", UintegerValue(2));
    /**
     * \ingroup antenna
     *
     * \brief Isotropic antenna model
     *
     * This is the simplest antenna model. The gain of this antenna 
     * is the same in all directions.
     */    
    nrHelper->SetUeAntennaAttribute("AntennaElement",
        PointerValue(CreateObject<IsotropicAntennaModel>()));
    double txPower = 23;
    nrHelper->SetUePhyAttribute("TxPower", DoubleValue(txPower));

    // NR Sidelink attribute of UE MAC, which are would be common for 
    // all the UEs
    /*
    * EnableSensing	BooleanValue(false): Desabilita o recurso de 
    detecção e identificação de sinais de outras redes celulares.
    * T1 - UintegerValue(2): Define o valor de T1, que controla o 
    tempo mínimo que um UE deve esperar antes de iniciar uma 
    transmissão Sidelink após detectar um canal disponível.
    * T2 - UintegerValue(33): Define o valor de T2, que determina 
    o tempo máximo que um UE pode continuar tentando transmitir 
    Sidelink em um canal específico.
    * ActivePoolId - UintegerValue(0): Seleciona o pool de SID ativo 
    que o UE usará para descoberta de serviços e comunicação Sidelink. 
    O valor 0 normalmente se refere ao pool padrão.
    * ReservationPeriod	- TimeValue(MilliSeconds(100)): Define o período 
    máximo que um UE pode reservar um recurso Sidelink antes de iniciá-lo.
    100 milissegundos é um valor relativamente curto.
    * NumSidelinkProcess - UintegerValue(4): Define o número de processos 
    paralelos usados para processar transmissões e recepções Sidelink.    
    Aumentar o número pode melhorar o desempenho em cenários complexos, 
    mas também aumenta o consumo de recursos.
    * EnableBlindReTx - BooleanValue(true):	Habilita retransmissões cegas 
    para transmissões Sidelink. Retransmissões cegas são enviadas sem 
    confirmação de recebimento do destinatário, podendo ser úteis em 
    cenários com alta perda de pacotes, mas aumentam o tráfego na rede.
    *
    * Alguns pontos a considerar:
    * 
    * Desabilitar o EnableSensing simplifica a simulação, mas não reflete 
    * como o Sidelink funciona na realidade. Os valores de T1 e T2 afetam 
    * o equilíbrio entre agressividade de transmissão e eficiência do uso 
    * de canal. Valores menores podem reduzir colisões, mas aumentar o 
    * tempo de espera e atrasos. A escolha do ActivePoolId depende do 
    * cenário simulado e da configuração da rede. Um período de reserva 
    * curto (ReservationPeriod) pode ser útil para cenários dinâmicos com 
    * tráfego imprevisível, mas um período maior pode garantir estabilidade
    * no uso de recursos. Aumentar o NumSidelinkProcess pode ajudar com 
    * paralelismo, mas considere o impacto no consumo de recursos de 
    * simulação. Habilitar EnableBlindReTx pode melhorar a entrega de 
    * dados em condições desfavoráveis, mas aumenta a carga na rede e 
    * pode não ser adequado para todos os cenários.
    */
    nrHelper->SetUeMacAttribute("EnableSensing", BooleanValue(false));
    nrHelper->SetUeMacAttribute("T1", UintegerValue(2));
    nrHelper->SetUeMacAttribute("T2", UintegerValue(33));
    nrHelper->SetUeMacAttribute("ActivePoolId", UintegerValue(0));
    nrHelper->SetUeMacAttribute("ReservationPeriod", TimeValue(MilliSeconds(100)));
    nrHelper->SetUeMacAttribute("NumSidelinkProcess", UintegerValue(4));
    nrHelper->SetUeMacAttribute("EnableBlindReTx", BooleanValue(true));

    uint8_t bwpIdForGbrMcptt = 0;
    /**
     * \brief Set the bandwidth part manager TypeId. Works only before it is created.
     * \param typeId Bandwidth part manager type id
     *
     * \see ns3::BwpManagerUe
     * \see ns3::NrSlBwpManagerUe
     */
    nrHelper->SetBwpManagerTypeId(TypeId::LookupByName("ns3::NrSlBwpManagerUe"));
    // following parameter has no impact at the moment because:
    // 1. No support for PQI based mapping between the application and the LCs
    // 2. No scheduler to consider PQI
    // However, till such time all the NR SL examples should use GBR_MC_PUSH_TO_TALK
    // because we hard coded the PQI 65 in UE RRC.
    /* não há suporte na simulação para um mapeamento baseado em PQI 
    * (Packet Quality Indicator) entre os aplicativos e os LCs 
    * (Logical Channels). PQI é um valor que reflete a qualidade do 
    * link sem fio. o escalonador utilizado não leva em consideração 
    * o PQI ao alocar recursos de rede.
    * Apesar das limitações mencionadas, recomenda-se utilizar o 
    * "GBR_MC_PUSH_TO_TALK" para todos os exemplos de Sidelink NR na simulação.
    * Embora a simulação ns-3 não suporte mapeamento dinâmico de QoS baseado 
    * em PQI, a configuração "GBR_MC_PUSH_TO_TALK" é recomendada como uma 
    * solução alternativa. Este modo aloca uma taxa de bits garantida (GBR) 
    * para as comunicações V2X usando Sidelink NR. O PQI fixo de 65 codificado 
    * no RRC do UE garante que esta taxa de bits seja sempre alocada, 
    * independentemente das condições reais do canal.
    * 
    * Limitações:
    * 
    * Usar GBR_MC_PUSH_TO_TALK com PQI fixo não reflete completamente o 
    * comportamento do 5G NR real, que se adapta dinamicamente às condições 
    * do canal. Esta abordagem simplifica a simulação, mas pode não ser 
    * adequada para cenários complexos ou para avaliar o desempenho de 
    * mecanismos avançados de QoS.
    */
    nrHelper->SetUeBwpManagerAlgorithmAttribute("GBR_MC_PUSH_TO_TALK",
                                                UintegerValue(bwpIdForGbrMcptt));

    std::set<uint8_t> bwpIdContainer;
    bwpIdContainer.insert(bwpIdForGbrMcptt);

    /*
     * We miss many other parameters. By default, not configuring them is equivalent
     * to use the default values. Please, have a look at the documentation to see
     * what are the default values for all the attributes you are not seeing here.
     */

    /*
     * Case (ii): Attributes valid for a subset of the nodes
     */

    // NOT PRESENT IN THIS SIMPLE EXAMPLE

    /*
     * We have configured the attributes we needed. Now, install and get the pointers
     * to the NetDevices, which contains all the NR stack:
     */
    NetDeviceContainer ueVoiceNetDev = nrHelper->InstallUeDevice(ueVoiceContainer, allBwps);

    /*
     * Case (iii): Go node for node and change the attributes we have to setup
     * per-node.
     */

    // When all the configuration is done, explicitly call UpdateConfig ()
    for (auto it = ueVoiceNetDev.Begin(); it != ueVoiceNetDev.End(); ++it)
    {
        DynamicCast<NrUeNetDevice>(*it)->UpdateConfig();
    }

    /*
     * Configure Sidelink. We create the following helpers needed for the
     * NR Sidelink, i.e., V2X simulation:
     * - NrSlHelper, which will configure the UEs protocol stack to be ready to
     *   perform Sidelink related procedures.
     * - EpcHelper, which takes care of triggering the call to EpcUeNas class
     *   to establish the NR Sidelink bearer (s). We note that, at this stage
     *   just communicate the pointer of already instantiated EpcHelper object,
     *   which is the same pointer communicated to the NrHelper above.
     */
    Ptr<NrSlHelper> nrSlHelper = CreateObject<NrSlHelper>();
    // Put the pointers inside NrSlHelper
    nrSlHelper->SetEpcHelper(epcHelper);

    /*
     * Set the SL error model and AMC
     * Error model type: ns3::NrEesmCcT1, ns3::NrEesmCcT2, ns3::NrEesmIrT1,
     *                   ns3::NrEesmIrT2, ns3::NrLteMiErrorModel
     * AMC type: NrAmc::ShannonModel or NrAmc::ErrorModel
     */
    std::string errorModel = "ns3::NrEesmIrT1";
    nrSlHelper->SetSlErrorModel(errorModel);
    nrSlHelper->SetUeSlAmcAttribute("AmcModel", EnumValue(NrAmc::ErrorModel));

    /*
     * Set the SL scheduler attributes
     * In this example we use NrSlUeMacSchedulerSimple scheduler, which uses
     * fix MCS value
     */
    nrSlHelper->SetNrSlSchedulerTypeId(NrSlUeMacSchedulerSimple::GetTypeId());
    nrSlHelper->SetUeSlSchedulerAttribute("FixNrSlMcs", BooleanValue(true));
    nrSlHelper->SetUeSlSchedulerAttribute("InitialNrSlMcs", UintegerValue(14));

    /*
     * Very important method to configure UE protocol stack, i.e., it would
     * configure all the SAPs among the layers, setup callbacks, configure
     * error model, configure AMC, and configure ChunkProcessor in Interference
     * API.
     */
    nrSlHelper->PrepareUeForSidelink(ueVoiceNetDev, bwpIdContainer);

    /*
     * Start preparing for all the sub Structs/RRC Information Element (IEs)
     * of LteRrcSap::SidelinkPreconfigNr. This is the main structure, which would
     * hold all the pre-configuration related to Sidelink.
     */

    // SlResourcePoolNr IE
    LteRrcSap::SlResourcePoolNr slResourcePoolNr;
    // get it from pool factory
    Ptr<NrSlCommPreconfigResourcePoolFactory> ptrFactory =
        Create<NrSlCommPreconfigResourcePoolFactory>();
    /*
     * Above pool factory is created to help the users of the simulator to create
     * a pool with valid default configuration. Please have a look at the
     * constructor of NrSlCommPreconfigResourcePoolFactory class.
     *
     * In the following, we show how one could change those default pool parameter
     * values as per the need.
     */
    std::vector<std::bitset<1>> slBitmap = {1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1};
    ptrFactory->SetSlTimeResources(slBitmap);
    ptrFactory->SetSlSensingWindow(100); // T0 in ms
    ptrFactory->SetSlSelectionWindow(5);
    ptrFactory->SetSlFreqResourcePscch(10); // PSCCH RBs
    ptrFactory->SetSlSubchannelSize(50);
    ptrFactory->SetSlMaxNumPerReserve(3);
    // Once parameters are configured, we can create the pool
    LteRrcSap::SlResourcePoolNr pool = ptrFactory->CreatePool();
    slResourcePoolNr = pool;

    // Configure the SlResourcePoolConfigNr IE, which hold a pool and its id
    LteRrcSap::SlResourcePoolConfigNr slresoPoolConfigNr;
    slresoPoolConfigNr.haveSlResourcePoolConfigNr = true;
    // Pool id, ranges from 0 to 15
    uint16_t poolId = 0;
    LteRrcSap::SlResourcePoolIdNr slResourcePoolIdNr;
    slResourcePoolIdNr.id = poolId;
    slresoPoolConfigNr.slResourcePoolId = slResourcePoolIdNr;
    slresoPoolConfigNr.slResourcePool = slResourcePoolNr;

    // Configure the SlBwpPoolConfigCommonNr IE, which hold an array of pools
    LteRrcSap::SlBwpPoolConfigCommonNr slBwpPoolConfigCommonNr;
    // Array for pools, we insert the pool in the array as per its poolId
    slBwpPoolConfigCommonNr.slTxPoolSelectedNormal[slResourcePoolIdNr.id] = slresoPoolConfigNr;

    // Configure the BWP IE
    LteRrcSap::Bwp bwp;
    bwp.numerology = numerologyBwpSl;
    bwp.symbolsPerSlots = 14;
    bwp.rbPerRbg = 1;
    bwp.bandwidth = bandwidthBandSl;

    // Configure the SlBwpGeneric IE
    LteRrcSap::SlBwpGeneric slBwpGeneric;
    slBwpGeneric.bwp = bwp;
    slBwpGeneric.slLengthSymbols = LteRrcSap::GetSlLengthSymbolsEnum(14);
    slBwpGeneric.slStartSymbol = LteRrcSap::GetSlStartSymbolEnum(0);

    // Configure the SlBwpConfigCommonNr IE
    LteRrcSap::SlBwpConfigCommonNr slBwpConfigCommonNr;
    slBwpConfigCommonNr.haveSlBwpGeneric = true;
    slBwpConfigCommonNr.slBwpGeneric = slBwpGeneric;
    slBwpConfigCommonNr.haveSlBwpPoolConfigCommonNr = true;
    slBwpConfigCommonNr.slBwpPoolConfigCommonNr = slBwpPoolConfigCommonNr;

    // Configure the SlFreqConfigCommonNr IE, which hold the array to store
    // the configuration of all Sidelink BWP (s).
    LteRrcSap::SlFreqConfigCommonNr slFreConfigCommonNr;
    // Array for BWPs. Here we will iterate over the BWPs, which
    // we want to use for SL.
    for (const auto& it : bwpIdContainer)
    {
        // it is the BWP id
        slFreConfigCommonNr.slBwpList[it] = slBwpConfigCommonNr;
    }

    // Configure the TddUlDlConfigCommon IE
    LteRrcSap::TddUlDlConfigCommon tddUlDlConfigCommon;
    tddUlDlConfigCommon.tddPattern = "DL|DL|DL|F|UL|UL|UL|UL|UL|UL|";

    // Configure the SlPreconfigGeneralNr IE
    LteRrcSap::SlPreconfigGeneralNr slPreconfigGeneralNr;
    slPreconfigGeneralNr.slTddConfig = tddUlDlConfigCommon;

    // Configure the SlUeSelectedConfig IE
    LteRrcSap::SlUeSelectedConfig slUeSelectedPreConfig;
    slUeSelectedPreConfig.slProbResourceKeep = 0;
    // Configure the SlPsschTxParameters IE
    LteRrcSap::SlPsschTxParameters psschParams;
    psschParams.slMaxTxTransNumPssch = 5;
    // Configure the SlPsschTxConfigList IE
    LteRrcSap::SlPsschTxConfigList pscchTxConfigList;
    pscchTxConfigList.slPsschTxParameters[0] = psschParams;
    slUeSelectedPreConfig.slPsschTxConfigList = pscchTxConfigList;

    /*
     * Finally, configure the SidelinkPreconfigNr This is the main structure
     * that needs to be communicated to NrSlUeRrc class
     */
    LteRrcSap::SidelinkPreconfigNr slPreConfigNr;
    slPreConfigNr.slPreconfigGeneral = slPreconfigGeneralNr;
    slPreConfigNr.slUeSelectedPreConfig = slUeSelectedPreConfig;
    slPreConfigNr.slPreconfigFreqInfoList[0] = slFreConfigCommonNr;

    // Communicate the above pre-configuration to the NrSlHelper
    nrSlHelper->InstallNrSlPreConfiguration(ueVoiceNetDev, slPreConfigNr);

    /****************************** End SL Configuration ***********************/

    /*
     * Fix the random streams
     */
    int64_t stream = 1;
    stream += nrHelper->AssignStreams(ueVoiceNetDev, stream);
    stream += nrSlHelper->AssignStreams(ueVoiceNetDev, stream);

    /*
     * Configure the IP stack, and activate NR Sidelink bearer (s) as per the
     * configured time.
     *
     * This example supports IPV4 and IPV6
     */

    InternetStackHelper internet;
    internet.Install(ueVoiceContainer);
    stream += internet.AssignStreams(ueVoiceContainer, stream);
    uint32_t dstL2Id = 255;
    Ipv4Address groupAddress4("225.0.0.0"); // use multicast address as destination
    Ipv6Address groupAddress6("ff0e::1");   // use multicast address as destination
    Address remoteAddress;
    Address localAddress;
    uint16_t port = 8000;
    Ptr<LteSlTft> tft;
    if (!useIPv6)
    {
        Ipv4InterfaceContainer ueIpIface;
        ueIpIface = epcHelper->AssignUeIpv4Address(ueVoiceNetDev);

        // set the default gateway for the UE
        Ipv4StaticRoutingHelper ipv4RoutingHelper;
        for (uint32_t u = 0; u < ueVoiceContainer.GetN(); ++u)
        {
            Ptr<Node> ueNode = ueVoiceContainer.Get(u);
            // Set the default gateway for the UE
            Ptr<Ipv4StaticRouting> ueStaticRouting =
                ipv4RoutingHelper.GetStaticRouting(ueNode->GetObject<Ipv4>());
            ueStaticRouting->SetDefaultRoute(epcHelper->GetUeDefaultGatewayAddress(), 1);
        }
        remoteAddress = InetSocketAddress(groupAddress4, port);
        localAddress = InetSocketAddress(Ipv4Address::GetAny(), port);
        tft = Create<LteSlTft>(LteSlTft::Direction::BIDIRECTIONAL,
                               LteSlTft::CommType::GroupCast,
                               groupAddress4,
                               dstL2Id);
        // Set Sidelink bearers
        nrSlHelper->ActivateNrSlBearer(finalSlBearersActivationTime, ueVoiceNetDev, tft);
    }
    else
    {
        Ipv6InterfaceContainer ueIpIface;
        ueIpIface = epcHelper->AssignUeIpv6Address(ueVoiceNetDev);

        // set the default gateway for the UE
        Ipv6StaticRoutingHelper ipv6RoutingHelper;
        for (uint32_t u = 0; u < ueVoiceContainer.GetN(); ++u)
        {
            Ptr<Node> ueNode = ueVoiceContainer.Get(u);
            // Set the default gateway for the UE
            Ptr<Ipv6StaticRouting> ueStaticRouting =
                ipv6RoutingHelper.GetStaticRouting(ueNode->GetObject<Ipv6>());
            ueStaticRouting->SetDefaultRoute(epcHelper->GetUeDefaultGatewayAddress6(), 1);
        }
        remoteAddress = Inet6SocketAddress(groupAddress6, port);
        localAddress = Inet6SocketAddress(Ipv6Address::GetAny(), port);
        tft = Create<LteSlTft>(LteSlTft::Direction::BIDIRECTIONAL,
                               LteSlTft::CommType::GroupCast,
                               groupAddress6,
                               dstL2Id);
        // Set Sidelink bearers
        nrSlHelper->ActivateNrSlBearer(finalSlBearersActivationTime, ueVoiceNetDev, tft);
    }

    /*
     * Configure the applications:
     * Client app: OnOff application configure to generate CBR traffic
     * Server app: PacketSink application.
     */

    // Set Application in the UEs
    OnOffHelper sidelinkClient("ns3::UdpSocketFactory", remoteAddress);
    sidelinkClient.SetAttribute("EnableSeqTsSizeHeader", BooleanValue(true));
    std::string dataRateBeString = std::to_string(dataRateBe) + "kb/s";
    std::cout << "Data rate " << DataRate(dataRateBeString) << std::endl;
    sidelinkClient.SetConstantRate(DataRate(dataRateBeString), udpPacketSizeBe);

    ApplicationContainer clientApps = sidelinkClient.Install(ueVoiceContainer.Get(0));
    // onoff application will send the first packet at :
    // finalSlBearersActivationTime + ((Pkt size in bits) / (Data rate in bits per sec))
    clientApps.Start(finalSlBearersActivationTime);
    clientApps.Stop(finalSimTime);

    // Output app start, stop and duration
    double realAppStart =
        finalSlBearersActivationTime.GetSeconds() +
        ((double)udpPacketSizeBe * 8.0 / (DataRate(dataRateBeString).GetBitRate()));
    double appStopTime = (finalSimTime).GetSeconds();

    std::cout << "App start time " << realAppStart << " sec" << std::endl;
    std::cout << "App stop time " << appStopTime << " sec" << std::endl;

    ApplicationContainer serverApps;
    PacketSinkHelper sidelinkSink("ns3::UdpSocketFactory", localAddress);
    sidelinkSink.SetAttribute("EnableSeqTsSizeHeader", BooleanValue(true));
    serverApps = sidelinkSink.Install(ueVoiceContainer.Get(ueVoiceContainer.GetN() - 1));
    serverApps.Start(Seconds(2.0));

    /*
     * Hook the traces, to be used to compute average PIR and to data to be
     * stored in a database
     */

    // Trace receptions; use the following to be robust to node ID changes
    std::ostringstream path;
    path << "/NodeList/" << ueVoiceContainer.Get(1)->GetId()
         << "/ApplicationList/0/$ns3::PacketSink/Rx";
    Config::ConnectWithoutContext(path.str(), MakeCallback(&ReceivePacket));
    path.str("");

    path << "/NodeList/" << ueVoiceContainer.Get(1)->GetId()
         << "/ApplicationList/0/$ns3::PacketSink/Rx";
    Config::ConnectWithoutContext(path.str(), MakeCallback(&ComputePir));
    path.str("");

    path << "/NodeList/" << ueVoiceContainer.Get(0)->GetId()
         << "/ApplicationList/0/$ns3::OnOffApplication/Tx";
    Config::ConnectWithoutContext(path.str(), MakeCallback(&TransmitPacket));
    path.str("");

    // Datebase setup
    std::string exampleName = simTag + "-" + "nr-v2x-simple-demo";
    SQLiteOutput db(outputDir + exampleName + ".db");

    UeMacPscchTxOutputStats pscchStats;
    pscchStats.SetDb(&db, "pscchTxUeMac");
    Config::ConnectWithoutContext("/NodeList/*/DeviceList/*/$ns3::NrUeNetDevice/"
                                  "ComponentCarrierMapUe/*/NrUeMac/SlPscchScheduling",
                                  MakeBoundCallback(&NotifySlPscchScheduling, &pscchStats));

    UeMacPsschTxOutputStats psschStats;
    psschStats.SetDb(&db, "psschTxUeMac");
    Config::ConnectWithoutContext("/NodeList/*/DeviceList/*/$ns3::NrUeNetDevice/"
                                  "ComponentCarrierMapUe/*/NrUeMac/SlPsschScheduling",
                                  MakeBoundCallback(&NotifySlPsschScheduling, &psschStats));

    UePhyPscchRxOutputStats pscchPhyStats;
    pscchPhyStats.SetDb(&db, "pscchRxUePhy");
    Config::ConnectWithoutContext(
        "/NodeList/*/DeviceList/*/$ns3::NrUeNetDevice/ComponentCarrierMapUe/*/NrUePhy/"
        "NrSpectrumPhyList/*/RxPscchTraceUe",
        MakeBoundCallback(&NotifySlPscchRx, &pscchPhyStats));

    UePhyPsschRxOutputStats psschPhyStats;
    psschPhyStats.SetDb(&db, "psschRxUePhy");
    Config::ConnectWithoutContext(
        "/NodeList/*/DeviceList/*/$ns3::NrUeNetDevice/ComponentCarrierMapUe/*/NrUePhy/"
        "NrSpectrumPhyList/*/RxPsschTraceUe",
        MakeBoundCallback(&NotifySlPsschRx, &psschPhyStats));

    UeToUePktTxRxOutputStats pktStats;
    pktStats.SetDb(&db, "pktTxRx");

    if (!useIPv6)
    {
        // Set Tx traces
        for (uint16_t ac = 0; ac < clientApps.GetN(); ac++)
        {
            Ipv4Address localAddrs = clientApps.Get(ac)
                                         ->GetNode()
                                         ->GetObject<Ipv4L3Protocol>()
                                         ->GetAddress(1, 0)
                                         .GetLocal();
            std::cout << "Tx address: " << localAddrs << std::endl;
            clientApps.Get(ac)->TraceConnect("TxWithSeqTsSize",
                                             "tx",
                                             MakeBoundCallback(&UePacketTraceDb,
                                                               &pktStats,
                                                               ueVoiceContainer.Get(0),
                                                               localAddrs));
        }

        // Set Rx traces
        for (uint16_t ac = 0; ac < serverApps.GetN(); ac++)
        {
            Ipv4Address localAddrs = serverApps.Get(ac)
                                         ->GetNode()
                                         ->GetObject<Ipv4L3Protocol>()
                                         ->GetAddress(1, 0)
                                         .GetLocal();
            std::cout << "Rx address: " << localAddrs << std::endl;
            serverApps.Get(ac)->TraceConnect("RxWithSeqTsSize",
                                             "rx",
                                             MakeBoundCallback(&UePacketTraceDb,
                                                               &pktStats,
                                                               ueVoiceContainer.Get(1),
                                                               localAddrs));
        }
    }
    else
    {
        // Set Tx traces
        for (uint16_t ac = 0; ac < clientApps.GetN(); ac++)
        {
            clientApps.Get(ac)->GetNode()->GetObject<Ipv6L3Protocol>()->AddMulticastAddress(
                groupAddress6);
            Ipv6Address localAddrs = clientApps.Get(ac)
                                         ->GetNode()
                                         ->GetObject<Ipv6L3Protocol>()
                                         ->GetAddress(1, 1)
                                         .GetAddress();
            std::cout << "Tx address: " << localAddrs << std::endl;
            clientApps.Get(ac)->TraceConnect("TxWithSeqTsSize",
                                             "tx",
                                             MakeBoundCallback(&UePacketTraceDb,
                                                               &pktStats,
                                                               ueVoiceContainer.Get(0),
                                                               localAddrs));
        }

        // Set Rx traces
        for (uint16_t ac = 0; ac < serverApps.GetN(); ac++)
        {
            serverApps.Get(ac)->GetNode()->GetObject<Ipv6L3Protocol>()->AddMulticastAddress(
                groupAddress6);
            Ipv6Address localAddrs = serverApps.Get(ac)
                                         ->GetNode()
                                         ->GetObject<Ipv6L3Protocol>()
                                         ->GetAddress(1, 1)
                                         .GetAddress();
            std::cout << "Rx address: " << localAddrs << std::endl;
            serverApps.Get(ac)->TraceConnect("RxWithSeqTsSize",
                                             "rx",
                                             MakeBoundCallback(&UePacketTraceDb,
                                                               &pktStats,
                                                               ueVoiceContainer.Get(1),
                                                               localAddrs));
        }
    }

    Simulator::Stop(finalSimTime);
    Simulator::Run();

    std::cout << "Total Tx bits = " << txByteCounter * 8 << std::endl;
    std::cout << "Total Tx packets = " << txPktCounter << std::endl;

    std::cout << "Total Rx bits = " << rxByteCounter * 8 << std::endl;
    std::cout << "Total Rx packets = " << rxPktCounter << std::endl;

    std::cout << "Avrg thput = "
              << (rxByteCounter * 8) / (finalSimTime - Seconds(realAppStart)).GetSeconds() / 1000.0
              << " kbps" << std::endl;

    std::cout << "Average Packet Inter-Reception (PIR) " << pir.GetSeconds() / pirCounter << " sec"
              << std::endl;

    /*
     * VERY IMPORTANT: Do not forget to empty the database cache, which would
     * dump the data store towards the end of the simulation in to a database.
     */
    pktStats.EmptyCache();
    pscchStats.EmptyCache();
    psschStats.EmptyCache();
    pscchPhyStats.EmptyCache();
    psschPhyStats.EmptyCache();

    Simulator::Destroy();

    return 0;
}