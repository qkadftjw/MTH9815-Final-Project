/**
 * pricingservice.hpp
 * Defines the data types and Service for internal prices.
 *
 * @author Breman Thuraisingham, Fangtong Wang
 */

#ifndef PRICING_SERVICE_HPP
#define PRICING_SERVICE_HPP

#include <string>
#include <map>

#include "soa.hpp"

/**
 * A price object consisting of mid and bid/offer spread.
 * Type T is the product type.
 */
template <typename T>
class Price {
   public:
    // ctor for a price
    Price(const T& _product, double _mid, double _bidOfferSpread);

    Price() = default;
    virtual ~Price() = default;

    // Get the product
    const T& GetProduct() const;

    // Get the mid price
    double GetMid() const;

    // Get the bid/offer spread around the mid
    double GetBidOfferSpread() const;

    // Get String
    vector<string> ToStrings() const;

   private:
    T product;
    double mid;
    double bidOfferSpread;
};

template <typename T>
Price<T>::Price(const T& _product, double _mid, double _bidOfferSpread) : product(_product) {
    mid = _mid;
    bidOfferSpread = _bidOfferSpread;
}

template <typename T>
const T& Price<T>::GetProduct() const {
    return product;
}

template <typename T>
double Price<T>::GetMid() const {
    return mid;
}

template <typename T>
double Price<T>::GetBidOfferSpread() const {
    return bidOfferSpread;
}

template <typename T>
vector<string> Price<T>::ToStrings() const {
    vector<string> outputStrings;

    // Format mid price and bid-offer spread as strings
    string midPriceStr = FormatPrice(mid);
    string spreadStr = FormatPrice(bidOfferSpread);

    // Extract product ID
    string productId = product.GetProductId();

    // Collect all fields into a vector of strings
    outputStrings.emplace_back(productId);
    outputStrings.emplace_back(midPriceStr);
    outputStrings.emplace_back(spreadStr);

    return outputStrings;
}

template <typename T>
class PricingConnector;

/**
 * Pricing Service managing mid prices and bid/offers.
 * Keyed on product identifier.
 * Type T is the product type.
 */
template <typename T>
class PricingService : public Service<string, Price<T>> {
public:
    // Default constructor and virtual destructor
    PricingService();
    virtual ~PricingService() = default;

    // Get the data associated with a specific key
    virtual Price<T>& GetData(string key);

    // Handle new or updated data from connectors
    virtual void OnMessage(Price<T>& data);

    // Add a listener to the service
    virtual void AddListener(ServiceListener<Price<T>>* listener);

    // Retrieve all listeners currently registered
    virtual const vector<ServiceListener<Price<T>>*>& GetListeners() const;

protected:
    // Protected members accessible to derived classes
    map<string, Price<T>> priceData;
    vector<ServiceListener<Price<T>>*> serviceListeners;
};

template<typename T>
PricingService<T>::PricingService()
{
	priceData = map<string, Price<T>>();
	serviceListeners = vector<ServiceListener<Price<T>>*>();
}


// Implementation of PricingService
template <typename T>
Price<T>& PricingService<T>::GetData(string key) {
    return priceData[key];
}

template <typename T>
void PricingService<T>::OnMessage(Price<T>& data) {
    priceData[data.GetProduct().GetProductId()] = data;

    for (auto& listener : serviceListeners) {
        listener->ProcessAdd(data);
    }
}

template <typename T>
void PricingService<T>::AddListener(ServiceListener<Price<T>>* listener) {
    serviceListeners.push_back(listener);
}

template <typename T>
const vector<ServiceListener<Price<T>>*>& PricingService<T>::GetListeners() const {
    return serviceListeners;
}

/**
 * Specialized BondPricingService 
 * derived from PricingService
 */
template <typename T>
class BondPricingService : public PricingService<T> {
public:
    // Default constructor and destructor
    BondPricingService();
    virtual ~BondPricingService() = default;

	// Override methods to provide BondPricing-specific behavior
    Price<T>& GetData(string key) override;
	void OnMessage(Price<T>& data) override;
    void AddListener(ServiceListener<Price<T>>* listener) override;
    const vector<ServiceListener<Price<T>>*>& GetListeners() const override;

    // Additional method to retrieve the connector for this service
    PricingConnector<T>* GetConnector();

private:
    PricingConnector<T>* bondConnector;
};

template<typename T>
BondPricingService<T>::BondPricingService() : PricingService<T>()
{
    bondConnector = new PricingConnector<T>(this);
}


template <typename T>
Price<T>& BondPricingService<T>::GetData(string key) {
    return this->priceData[key];
}

template <typename T>
void BondPricingService<T>::OnMessage(Price<T>& data) {
    this->priceData[data.GetProduct().GetProductId()] = data;

    for (auto& listener : this->serviceListeners) {
        listener->ProcessAdd(data);
    }
}

template <typename T>
void BondPricingService<T>::AddListener(ServiceListener<Price<T>>* listener) {
    this->serviceListeners.push_back(listener);
}

template <typename T>
const vector<ServiceListener<Price<T>>*>& BondPricingService<T>::GetListeners() const {
    return this->serviceListeners;
}

template <typename T>
PricingConnector<T>* BondPricingService<T>::GetConnector() {
    return bondConnector;
}

// /**
//  * Pricing Connector subscribing data to Pricing Service.
//  * Type T is the product type.
//  */
template <typename T>
class PricingConnector : public Connector<Price<T>> {
   public:
    // Connector and Destructor
    PricingConnector(PricingService<T>* servicePtr);
    virtual ~PricingConnector() = default;

    // Publish data to the Connector
    virtual void Publish(Price<T>& priceData);

    // Subscribe data from the Connector
    virtual void Subscribe(ifstream& inputData);

   private:
    PricingService<T>* service;
};

template <typename T>
PricingConnector<T>::PricingConnector(PricingService<T>* servicePtr) : service(servicePtr)
{
}

template <typename T>
class BondPricingConnector : public PricingConnector<T> 
{
    public:
    // Constructor and destructor
    BondPricingConnector(PricingService<T>* servicePtr);
    virtual ~BondPricingConnector();

    // Implementation of Publish and Subscribe methods
    void Publish(Price<T>& priceData) override;
    void Subscribe(ifstream& inputData) override;
};


// Constructor: initialize with service pointer
template <typename T>
BondPricingConnector<T>::BondPricingConnector(PricingService<T>* servicePtr)
    : PricingConnector<T>(servicePtr) 
{
}

// Destructor: no additional cleanup required
template <typename T>
BondPricingConnector<T>::~BondPricingConnector() 
{
}

template <typename T>
void PricingConnector<T>::Publish(Price<T>& priceData) 
{
}

template <typename T>
void PricingConnector<T>::Subscribe(ifstream& inputData) {
    string lineBuffer;

    while (getline(inputData, lineBuffer)) {
        stringstream lineStream(lineBuffer);
        string cell;
        vector<string> parsedFields;

        // Parse CSV line into individual fields
        while (getline(lineStream, cell, ',')) {
            parsedFields.push_back(cell);
        }

        // Extract individual fields from parsed data
        string productId = parsedFields[0];
        double bidPrice = ParsePrice(parsedFields[1]);
        double offerPrice = ParsePrice(parsedFields[2]);
        double midPrice = (bidPrice + offerPrice) / 2.0;
        double bidOfferSpread = offerPrice - bidPrice;

        // Create a product object (e.g., bond) and price instance
        T productInstance = BondInfo(productId);
        Price<T> priceObject(productInstance, midPrice, bidOfferSpread);

        // Notify the associated service with the new price data
        this->service->OnMessage(priceObject);
    }
}

#endif
