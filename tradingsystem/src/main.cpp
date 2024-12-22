/**
* main.cpp
* Entry point for the bond trading system.

* @author Fangtong Wang
*/

#include <iostream>
#include <fstream>
#include <string>

#include "soa.hpp"
#include "products.hpp"
#include "algostreamingservice.hpp"
#include "executionservice.hpp"
#include "guiservice.hpp"
#include "historicaldataservice.hpp"
#include "inquiryservice.hpp"
#include "marketdataservice.hpp"
#include "positionservice.hpp"
#include "pricingservice.hpp"
#include "riskservice.hpp"
#include "streamingservice.hpp"
#include "tradebookingservice.hpp"
#include "simulatedata.hpp"

using namespace std;

int main() {
    cout << ">> Bond Trading System Starting <<" << endl;

    // Data generation
    cout << "[INFO] Generating simulation data..." << endl;
    DataSimulator simulator;
    simulator.GenerateAllData();
    cout << "[INFO] Data generation complete." << endl;

    // Service initialization
    cout << "[INFO] Initializing services..." << endl;
    BondPricingService<Bond> pricingService;
    TradeBookingService<Bond> tradeBookingService;
    PositionService<Bond> positionService;
    RiskService<Bond> riskService;
    BondMarketDataService<Bond> marketDataService;
    AlgoExecutionService<Bond> algoExecutionService;
    AlgoStreamingService<Bond> algoStreamingService;
    GUIService<Bond> guiService;
    ExecutionService<Bond> executionService;
    StreamingService<Bond> streamingService;
    BondInquiryService<Bond> inquiryService;

    HistoricalDataService<Position<Bond>> historicalPositionService(POSITION);
    HistoricalDataService<PV01<Bond>> historicalRiskService(RISK);
    HistoricalDataService<ExecutionOrder<Bond>> historicalExecutionService(EXECUTION);
    HistoricalDataService<PriceStream<Bond>> historicalStreamingService(STREAMING);
    HistoricalDataService<Inquiry<Bond>> historicalInquiryService(INQUIRY);
    cout << "[INFO] Services initialized successfully." << endl;

    // Service linkage
    cout << "[INFO] Linking services..." << endl;
    pricingService.AddListener(algoStreamingService.GetListener());
    pricingService.AddListener(guiService.GetListener());
    algoStreamingService.AddListener(streamingService.GetListener());
    streamingService.AddListener(historicalStreamingService.GetListener());
    marketDataService.AddListener(algoExecutionService.GetListener());
    algoExecutionService.AddListener(executionService.GetListener());
    executionService.AddListener(tradeBookingService.GetListener());
    executionService.AddListener(historicalExecutionService.GetListener());
    tradeBookingService.AddListener(positionService.GetListener());
    positionService.AddListener(riskService.GetListener());
    positionService.AddListener(historicalPositionService.GetListener());
    riskService.AddListener(historicalRiskService.GetListener());
    inquiryService.AddListener(historicalInquiryService.GetListener());
    cout << "[INFO] All services linked successfully." << endl;

    // Data processing
    cout << "[INFO] Processing input data..." << endl;

    ifstream priceData("prices.txt");
    pricingService.GetConnector()->Subscribe(priceData);
    cout << "[INFO] Price data processed." << endl;

    ifstream tradeData("trades.txt");
    tradeBookingService.GetConnector()->Subscribe(tradeData);
    cout << "[INFO] Trade data processed." << endl;

    ifstream marketData("marketdata.txt");
    marketDataService.GetConnector()->Subscribe(marketData);
    cout << "[INFO] Market data processed." << endl;

    ifstream inquiryData("inquiries.txt");
    inquiryService.GetConnector()->Subscribe(inquiryData);
    cout << "[INFO] Inquiry data processed." << endl;

    cout << ">> Bond Trading System Completed <<" << endl;

    return 0;
}
