/**
 * algostreamingservice.hpp
 * Defines data structures and the Algo Streaming Service for processing and managing algorithmic price streams.
 *
 * @authors Breman Thuraisingham, Fangtong Wang
 */

#ifndef ALGO_STREAMINGSERVICE_HPP
#define ALGO_STREAMINGSERVICE_HPP

#include <string>
#include "soa.hpp"
#include "marketdataservice.hpp"
#include "pricingservice.hpp"

/**
 * Represents an order in a price stream, containing price, visible quantity, hidden quantity, and side.
 */
class PriceStreamOrder
{

public:

    // Default constructor
    PriceStreamOrder() = default;

    // Constructor initializing price, quantities, and side
    PriceStreamOrder(double _price, long _visibleQuantity, long _hiddenQuantity, PricingSide _side);

    // Default destructor
    virtual ~PriceStreamOrder() = default;

    // Retrieve the price of the order
    double GetPrice() const;

    // Retrieve the visible quantity of the order
    long GetVisibleQuantity() const;

    // Retrieve the hidden quantity of the order
    long GetHiddenQuantity() const;

    // Retrieve the side of the order (BID or OFFER)
    PricingSide GetSide() const;

    // Convert order attributes to a vector of strings for display
    vector<string> ToStrings() const;

private:
    double price;                 // Price of the order
    long visibleQuantity;         // Visible quantity of the order
    long hiddenQuantity;          // Hidden quantity of the order
    PricingSide side;             // Side of the order (BID/OFFER)

};

PriceStreamOrder::PriceStreamOrder(double _price, long _visibleQuantity, long _hiddenQuantity, PricingSide _side)
    : price(_price), visibleQuantity(_visibleQuantity), hiddenQuantity(_hiddenQuantity), side(_side) {}


// Implementation of accessor methods for PriceStreamOrder attributes

double PriceStreamOrder::GetPrice() const
{
    return price;
}

long PriceStreamOrder::GetVisibleQuantity() const
{
    return visibleQuantity;
}

long PriceStreamOrder::GetHiddenQuantity() const
{
    return hiddenQuantity;
}

PricingSide PriceStreamOrder::GetSide() const
{
    return side;
}

vector<string> PriceStreamOrder::ToStrings() const
{
    // Convert price and quantities to string and determine the side
    string priceStr = FormatPrice(price);
    string visibleQtyStr = to_string(visibleQuantity);
    string hiddenQtyStr = to_string(hiddenQuantity);
    string sideStr = (side == BID) ? "BID" : "OFFER";

    // Return all attributes as strings
    return {priceStr, visibleQtyStr, hiddenQtyStr, sideStr};
}

/**
 * Represents a price stream with a two-way market, including bid and offer orders.
 * Template parameter T is the product type.
 */
template<typename T>
class PriceStream
{

public:

    // Default constructor
    PriceStream() = default;

    // Constructor initializing the product and the associated bid/offer orders
    PriceStream(const T& _product, const PriceStreamOrder& _bidOrder, const PriceStreamOrder& _offerOrder);

    // Default destructor
    virtual ~PriceStream() = default;

    // Retrieve the product associated with the price stream
    const T& GetProduct() const;

    // Retrieve the bid order
    const PriceStreamOrder& GetBidOrder() const;

    // Retrieve the offer order
    const PriceStreamOrder& GetOfferOrder() const;

    // Convert price stream attributes to a vector of strings for display
    vector<string> ToStrings() const;

private:
    T product;                       // The product associated with the price stream
    PriceStreamOrder bidOrder;       // Bid order for the price stream
    PriceStreamOrder offerOrder;     // Offer order for the price stream

};

template<typename T>
PriceStream<T>::PriceStream(const T& _product, const PriceStreamOrder& _bidOrder, const PriceStreamOrder& _offerOrder) :
    product(_product), bidOrder(_bidOrder), offerOrder(_offerOrder) {}

template<typename T>
const T& PriceStream<T>::GetProduct() const
{
    return product;
}

template<typename T>
const PriceStreamOrder& PriceStream<T>::GetBidOrder() const
{
    return bidOrder;
}

template<typename T>
const PriceStreamOrder& PriceStream<T>::GetOfferOrder() const
{
    return offerOrder;
}

template<typename T>
vector<string> PriceStream<T>::ToStrings() const
{
    // Retrieve product ID and bid/offer order attributes as strings
    string productStr = product.GetProductId();
    vector<string> bidOrderStrings = bidOrder.ToStrings();
    vector<string> offerOrderStrings = offerOrder.ToStrings();

    // Combine all attributes into a single vector
    vector<string> resultStrings = {productStr};
    resultStrings.insert(resultStrings.end(), bidOrderStrings.begin(), bidOrderStrings.end());
    resultStrings.insert(resultStrings.end(), offerOrderStrings.begin(), offerOrderStrings.end());

    return resultStrings;
}

/**
 * Represents an algorithmically managed price stream, combining a product with its bid and offer orders.
 * Template parameter T is the product type.
 */
template<typename T>
class AlgoStream
{

public:

    // Default constructor
    AlgoStream() = default;

    // Constructor initializing the product and associated bid/offer orders
    AlgoStream(const T& _product, const PriceStreamOrder& _bidOrder, const PriceStreamOrder& _offerOrder);

    // Retrieve the underlying price stream
    PriceStream<T>* GetPriceStream() const;

private:
    PriceStream<T>* priceStream;     // Pointer to the price stream

};

template<typename T>
AlgoStream<T>::AlgoStream(const T& _product, const PriceStreamOrder& _bidOrder, const PriceStreamOrder& _offerOrder)
{
    priceStream = new PriceStream<T>(_product, _bidOrder, _offerOrder);
}

template<typename T>
PriceStream<T>* AlgoStream<T>::GetPriceStream() const
{
    return priceStream;
}

/**
 * Service for managing and processing algorithmic price streams for products.
 * Template parameter T is the product type.
 */
template<typename T>
class AlgoStreamingService : public Service<string, AlgoStream<T>> {
public:

    // Constructor and destructor
    AlgoStreamingService();
    virtual ~AlgoStreamingService() = default;

    // Retrieve data for a given product ID
    AlgoStream<T>& GetData(string key);

    // Process incoming data updates
    void OnMessage(AlgoStream<T>& data);

    // Add a listener to monitor events
    void AddListener(ServiceListener<AlgoStream<T>>* listener);

    // Retrieve all registered listeners
    const vector<ServiceListener<AlgoStream<T>>*>& GetListeners() const;

    // Retrieve the pricing service listener for algorithmic updates
    ServiceListener<Price<T>>* GetListener();

    // Publish two-way algorithmic prices based on input data
    void PublishAlgorithmicPrice(const Price<T>& price);

private:
    ServiceListener<Price<T>>* algoListener;        // Listener for pricing service updates
    long orderCounter;                              // Counter to manage order sequencing
    map<string, AlgoStream<T>> algoStreamMap;       // Map of product IDs to AlgoStreams
    vector<ServiceListener<AlgoStream<T>>*> listeners; // List of event listeners
};

template<typename T>
class ListenerAlgoStreamToPrc;

template<typename T>
AlgoStreamingService<T>::AlgoStreamingService()
    : algoStreamMap(map<string, AlgoStream<T>>()),
      listeners(vector<ServiceListener<AlgoStream<T>>*>()),
      algoListener(new ListenerAlgoStreamToPrc<T>(this)),
      orderCounter(0) 
{}

template<typename T>
AlgoStream<T>& AlgoStreamingService<T>::GetData(string key)
{
    return algoStreamMap[key];
}

template<typename T>
void AlgoStreamingService<T>::OnMessage(AlgoStream<T>& data)
{
    string productId = data.GetPriceStream()->GetProduct().GetProductId();
    algoStreamMap[productId] = data;
}

template<typename T>
void AlgoStreamingService<T>::AddListener(ServiceListener<AlgoStream<T>>* listener)
{
    listeners.push_back(listener);
}

template<typename T>
const vector<ServiceListener<AlgoStream<T>>*>& AlgoStreamingService<T>::GetListeners() const
{
    return listeners;
}

template<typename T>
ServiceListener<Price<T>>* AlgoStreamingService<T>::GetListener()
{
    return algoListener;
}

template<typename T>
void AlgoStreamingService<T>::PublishAlgorithmicPrice(const Price<T>& price)
{
    const T& product = price.GetProduct();
    string productId = product.GetProductId();

    // Calculate bid and offer prices
    double midPrice = price.GetMid();
    double spread = price.GetBidOfferSpread();
    double bidPrice = midPrice - (spread / 2.0);
    double offerPrice = midPrice + (spread / 2.0);

    // Determine quantities
    long visibleQty = ((orderCounter % 2) + 1) * 10000000;
    long hiddenQty = visibleQty * 2;

    // Increment the order counter
    orderCounter++;

    // Create bid and offer orders and publish as an AlgoStream
    PriceStreamOrder bidOrder(bidPrice, visibleQty, hiddenQty, BID);
    PriceStreamOrder offerOrder(offerPrice, visibleQty, hiddenQty, OFFER);
    AlgoStream<T> algoStream(product, bidOrder, offerOrder);

    algoStreamMap[productId] = algoStream;

    for (auto& listener : listeners)
    {
        listener->ProcessAdd(algoStream);
    }
}

/**
 * Listener for receiving updates from the pricing service and forwarding them to the AlgoStreamingService.
 * Template parameter T is the product type.
 */
template<typename T>
class ListenerAlgoStreamToPrc : public ServiceListener<Price<T>>
{

private:

    AlgoStreamingService<T>* service; // Associated AlgoStreamingService instance

public:

    // Constructor and destructor
    ListenerAlgoStreamToPrc(AlgoStreamingService<T>* _service);
    virtual ~ListenerAlgoStreamToPrc() = default;

    // Process add event from the pricing service
    void ProcessAdd(Price<T>& _data);

    // No implementation needed for remove event
    void ProcessRemove(Price<T>& _data);

    // No implementation needed for update event
    void ProcessUpdate(Price<T>& _data);

};

template<typename T>
ListenerAlgoStreamToPrc<T>::ListenerAlgoStreamToPrc(AlgoStreamingService<T>* _service)
{
    service = _service;
}

template<typename T>
void ListenerAlgoStreamToPrc<T>::ProcessAdd(Price<T>& _data)
{
    service->PublishAlgorithmicPrice(_data);
}

template<typename T>
void ListenerAlgoStreamToPrc<T>::ProcessRemove(Price<T>& _data) {}

template<typename T>
void ListenerAlgoStreamToPrc<T>::ProcessUpdate(Price<T>& _data) {}

#endif
