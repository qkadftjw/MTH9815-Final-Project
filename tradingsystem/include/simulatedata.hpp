/**
* simulate_data.hpp
* Simulate data by instruction.
* 
* @author Fangtong Wang
*/

#ifndef SIMULATE_DATA_HPP
#define SIMULATE_DATA_HPP

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <random>
#include <algorithm>
#include "utils.hpp"
#include "products.hpp"

using namespace std;

const std::vector<std::string> CUSIPS_VEC = {
    "91282CLY5", "91282CMB4", "91282CMA6", "91282CLZ2", "91282CLW9", "912810UF3", "912810UE6"
};

class DataSimulator {
public:
    // Constants Definition
    static constexpr int PRICES_PER_SECURITY = 1000000; // 1,000,000 prices per security
    static constexpr int TOTAL_SECURITIES = 7;
    static constexpr int TRADES_PER_SECURITY = 10; // 10 trades per security
    static constexpr int ORDER_BOOK_DEPTH = 5;
    static constexpr int INQUIRIES_PER_SECURITY = 10;

private:
    // Data Members
    std::vector<std::string> CUSIPS;
    std::vector<std::string> BOOK_LIST = {"TRSY1", "TRSY2", "TRSY3"};

public:
    // Constructor
    DataSimulator() : CUSIPS(CUSIPS_VEC) {}

    void GenerateMarketData() {
        std::ofstream marketFile("marketdata.txt", std::ios::out | std::ios::trunc);
        if (!marketFile.is_open()) {
            std::cerr << "Error: Unable to open marketdata.txt for writing." << std::endl;
            return;
        }

        const std::vector<double> spreadCycle = {1.0 / 128.0, 1.0 / 64.0, 3.0 / 128.0, 1.0 / 32.0};
        size_t spreadCycleIndex = 0;

        for (const auto& currentCUSIP : CUSIPS) {
            double midPrice = 99.0;
            bool ascending = true;

            for (int updateIndex = 1; updateIndex <= PRICES_PER_SECURITY; ++updateIndex) {
                double topSpread = spreadCycle[spreadCycleIndex];
                spreadCycleIndex = (spreadCycleIndex + 1) % spreadCycle.size();

                std::vector<double> bidSpreads, offerSpreads;
                for (int level = 0; level < ORDER_BOOK_DEPTH; ++level) {
                    bidSpreads.push_back(topSpread + level * (1.0 / 128.0));
                    offerSpreads.push_back(topSpread + level * (1.0 / 128.0));
                }

                for (int level = 0; level < ORDER_BOOK_DEPTH; ++level) {
                    double bidPrice = midPrice - bidSpreads[level];
                    double offerPrice = midPrice + offerSpreads[level];
                    long quantity = (level + 1) * 10000000;

                    marketFile << currentCUSIP << "," 
                               << FormatPrice(bidPrice) << "," 
                               << quantity << ",BID\n";

                    marketFile << currentCUSIP << "," 
                               << FormatPrice(offerPrice) << "," 
                               << quantity << ",OFFER\n";
                }

                midPrice = UpdateMidPrice(midPrice, ascending);
            }
        }

        marketFile.close();
    }

    void GeneratePriceData() {
        std::ofstream priceFile("prices.txt", std::ios::out | std::ios::trunc);
        if (!priceFile.is_open()) {
            std::cerr << "Error: Unable to open prices.txt for writing." << std::endl;
            return;
        }

        for (const auto& currentCUSIP : CUSIPS) {
            double midPrice = 99.0;
            bool ascending = true;
            bool spreadToggle = true;

            for (int priceIndex = 1; priceIndex <= PRICES_PER_SECURITY; ++priceIndex) {
                double spread = (midPrice == 99.0 || midPrice == 101.0) ? 1.0 / 64.0 : (spreadToggle ? 1.0 / 128.0 : 1.0 / 64.0);
                spreadToggle = !spreadToggle;

                double bidPrice = midPrice - spread;
                double offerPrice = midPrice + spread;

                if (bidPrice < 99.0) bidPrice = 99.0;
                if (offerPrice > 101.0) offerPrice = 101.0;

                priceFile << currentCUSIP << "," 
                          << FormatPrice(bidPrice) << "," 
                          << FormatPrice(offerPrice) << "\n";

                midPrice = UpdateMidPrice(midPrice, ascending);
            }
        }

        priceFile.close();
    }

    void GenerateTradeData() {
        std::ofstream tradeFile("trades.txt", std::ios::out | std::ios::trunc);
        if (!tradeFile.is_open()) {
            std::cerr << "Error: Unable to open trades.txt for writing." << std::endl;
            return;
        }

        const std::vector<long> quantitySequence = {1000000, 2000000, 3000000, 4000000, 5000000};
        size_t quantityIndex = 0;

        for (const auto& currentCUSIP : CUSIPS) {
            for (int tradeNum = 0; tradeNum < TRADES_PER_SECURITY; ++tradeNum) {
                std::string tradeID = GenerateUniqueId();
                std::string tradeSide = (tradeNum % 2 == 0) ? "BUY" : "SELL";
                double tradePriceValue = (tradeSide == "BUY") ? 99.0 : 100.0;
                std::string tradePrice = FormatPrice(tradePriceValue);
                std::string tradeBook = BOOK_LIST[tradeNum % BOOK_LIST.size()];
                long tradeQuantity = quantitySequence[quantityIndex];
                quantityIndex = (quantityIndex + 1) % quantitySequence.size();

                tradeFile << currentCUSIP << "," 
                          << tradeID << "," 
                          << tradePrice << "," 
                          << tradeBook << "," 
                          << tradeQuantity << "," 
                          << tradeSide << "\n";
            }
        }

        tradeFile.close();
    }

    void GenerateInquiries() {
        std::ofstream inquiriesFile("inquiries.txt");
        if (!inquiriesFile.is_open()) {
            std::cerr << "Error: Unable to open inquiries.txt for writing." << std::endl;
            return;
        }

        for (const auto& currentCUSIP : CUSIPS) {
            for (int inquiryIndex = 0; inquiryIndex < INQUIRIES_PER_SECURITY; ++inquiryIndex) {
                std::string inquiryID = GenerateUniqueId();
                std::string side = (inquiryIndex % 2) ? "BUY" : "SELL";
                long quantity = ((inquiryIndex % 5) + 1) * 1000000;
                std::string state = "RECEIVED";

                inquiriesFile << inquiryID << "," 
                              << currentCUSIP << "," 
                              << side << "," 
                              << quantity << "," 
                              << state << "\n";
            }
        }

        inquiriesFile.close();
    }

    void GenerateAllData() {
        GenerateTradeData();
        GenerateMarketData();
        GenerateInquiries();
        GeneratePriceData();
    }

private:
    double UpdateMidPrice(double midPrice, bool& ascending) {
        if (ascending) {
            if (midPrice + 1.0 / 256.0 > 101.0) {
                ascending = false;
                return 101.0;
            }
            return midPrice + 1.0 / 256.0;
        } else {
            if (midPrice - 1.0 / 256.0 < 99.0) {
                ascending = true;
                return 99.0;
            }
            return midPrice - 1.0 / 256.0;
        }
    }
};

#endif
