/**
 * marketdataservice.hpp
 * Defines the data types and Service for order book market data.
 *
 * @author Breman Thuraisingham, Fangtong Wang
 */

#ifndef MARKET_DATA_SERVICE_HPP
#define MARKET_DATA_SERVICE_HPP

#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <fstream>
#include <tuple>
#include <limits>
#include "soa.hpp"

using namespace std;

/**
 * Enum representing the pricing side of an order.
 * BID: Buy side.
 * OFFER: Sell side.
 */
enum PricingSide { BID, OFFER };

/**
 * Represents a market data order with attributes for price, quantity, and side.
 */
class Order {
   public:
    // Default constructor
    Order() = default;

    // Constructor to initialize an order
    Order(double _price, long _quantity, PricingSide _side);

    // Retrieve the price of the order
    double GetPrice() const;

    // Retrieve the quantity of the order
    long GetQuantity() const;

    // Retrieve the side of the order (BID/OFFER)
    PricingSide GetSide() const;

   private:
    double price;        // Price of the order
    long quantity;       // Quantity of the order
    PricingSide side;    // Side of the order (BID or OFFER)
};

Order::Order(double _price, long _quantity, PricingSide _side) {
    price = _price;
    quantity = _quantity;
    side = _side;
}

double Order::GetPrice() const { return price; }

long Order::GetQuantity() const { return quantity; }

PricingSide Order::GetSide() const { return side; }

/**
 * Represents a bid and offer pair of orders.
 */
class BidOffer {
   public:
    // Default constructor
    BidOffer() = default;

    // Constructor to initialize a bid and offer order pair
    BidOffer(const Order& _bidOrder, const Order& _offerOrder);

    virtual ~BidOffer() = default;

    // Retrieve the bid order
    const Order& GetBidOrder() const;

    // Retrieve the offer order
    const Order& GetOfferOrder() const;

   private:
    Order bidOrder;      // The bid order
    Order offerOrder;    // The offer order
};

BidOffer::BidOffer(const Order& _bidOrder, const Order& _offerOrder) 
    : bidOrder(_bidOrder), offerOrder(_offerOrder) {}

const Order& BidOffer::GetBidOrder() const { return bidOrder; }

const Order& BidOffer::GetOfferOrder() const { return offerOrder; }

/**
 * Represents an order book with a bid and offer stack for a specific product.
 * T: The type of the product associated with the order book.
 */
template <typename T>
class OrderBook {
   public:
    // Default constructor
    OrderBook() = default;

    // Constructor to initialize an order book with product and bid/offer stacks
    OrderBook(const T& _product, const vector<Order>& _bidStack, const vector<Order>& _offerStack);

    virtual ~OrderBook() = default;

    // Retrieve the product associated with the order book
    const T& GetProduct() const;

    // Retrieve the bid stack
    const vector<Order>& GetBidStack() const;

    // Retrieve the offer stack
    const vector<Order>& GetOfferStack() const;

    // Retrieve the best bid and offer orders
    BidOffer GetBestBidOffer() const;

   private:
    T product;               // The product associated with the order book
    vector<Order> bidStack;  // Stack of bid orders
    vector<Order> offerStack; // Stack of offer orders
};

template <typename T>
OrderBook<T>::OrderBook(const T& _product, const vector<Order>& _bidStack, const vector<Order>& _offerStack)
    : product(_product), bidStack(_bidStack), offerStack(_offerStack) {}

template <typename T>
const T& OrderBook<T>::GetProduct() const {
    return product;
}

template <typename T>
const vector<Order>& OrderBook<T>::GetBidStack() const {
    return bidStack;
}

template <typename T>
const vector<Order>& OrderBook<T>::GetOfferStack() const {
    return offerStack;
}

template <typename T>
BidOffer OrderBook<T>::GetBestBidOffer() const {
    double bestBidPrice = numeric_limits<double>::lowest();
    Order bestBid;
    for (const auto& b : bidStack) {
        double px = b.GetPrice();
        if (px > bestBidPrice) {
            bestBidPrice = px;
            bestBid = b;
        }
    }

    double bestOfferPrice = numeric_limits<double>::max();
    Order bestOffer;
    for (const auto& o : offerStack) {
        double px = o.GetPrice();
        if (px < bestOfferPrice) {
            bestOfferPrice = px;
            bestOffer = o;
        }
    }

    return BidOffer(bestBid, bestOffer);
}

// Forward declaration
template <typename T>
class BondMarketDataConnector;

/**
 * Abstract base class for a market data service that distributes market data.
 * Keyed by product identifier. T: The product type.
 */
template <typename T>
class MarketDataService : public Service<string, OrderBook<T>> {
   public:
    // Retrieve the best bid and offer orders for a specific product
    virtual BidOffer GetBestBidOffer(const string& productId) = 0;

    // Aggregate the order book depth
    virtual OrderBook<T> AggregateDepth(const string& productId) = 0;
};

/**
 * Concrete implementation of a market data service for bond products.
 */
template <typename T>
class BondMarketDataService : public MarketDataService<T> {
   public:
    // Constructor and destructor
    BondMarketDataService();
    virtual ~BondMarketDataService() = default;

    // Retrieve data for a specific product
    OrderBook<T>& GetData(string _key);

    // Callback invoked by a connector with new or updated data
    void OnMessage(OrderBook<T>& _data);

    // Add a listener for data updates
    void AddListener(ServiceListener<OrderBook<T>>* _listener);

    // Retrieve all listeners for the service
    const vector<ServiceListener<OrderBook<T>>*>& GetListeners() const;

    // Retrieve the connector associated with the service
    BondMarketDataConnector<T>* GetConnector();

    // Retrieve the depth of the order book
    int GetBookDepth() const;

    // Retrieve the best bid and offer for a product
    BidOffer GetBestBidOffer(const string& _productId);

    // Aggregate the order book depth
    OrderBook<T> AggregateDepth(const string& _productId);

   private:
    map<string, OrderBook<T>> orderBooks;                    // Map of product ID to order book
    vector<ServiceListener<OrderBook<T>>*> listeners;        // Listeners for data updates
    BondMarketDataConnector<T>* connector;                  // Connector for the service
    int bookDepth;                                           // Depth of the order book
};

template <typename T>
BondMarketDataService<T>::BondMarketDataService() {
    orderBooks = map<string, OrderBook<T>>();
    listeners = vector<ServiceListener<OrderBook<T>>*>();
    connector = new BondMarketDataConnector<T>(this);
    bookDepth = 5;
}

template <typename T>
OrderBook<T>& BondMarketDataService<T>::GetData(string _key) {
    return orderBooks[_key];
}

template <typename T>
void BondMarketDataService<T>::OnMessage(OrderBook<T>& _data) {
    orderBooks[_data.GetProduct().GetProductId()] = _data;

    for (auto& listener : listeners) {
        listener->ProcessAdd(_data);
    }
}

template <typename T>
void BondMarketDataService<T>::AddListener(ServiceListener<OrderBook<T>>* _listener) {
    listeners.push_back(_listener);
}

template <typename T>
const vector<ServiceListener<OrderBook<T>>*>& BondMarketDataService<T>::GetListeners() const {
    return listeners;
}

template <typename T>
BondMarketDataConnector<T>* BondMarketDataService<T>::GetConnector() {
    return connector;
}

template <typename T>
int BondMarketDataService<T>::GetBookDepth() const {
    return bookDepth;
}

template <typename T>
BidOffer BondMarketDataService<T>::GetBestBidOffer(const string& _productId) {
    auto& currOrderBook = orderBooks[_productId];
    return currOrderBook.GetBestBidOffer();
}

template <typename T>
OrderBook<T> BondMarketDataService<T>::AggregateDepth(const string& productId) {
    auto aggregateStack = [](const vector<Order>& stack, PricingSide side) {
        unordered_map<double, long> priceQuantityMap;
        for (const auto& order : stack) {
            priceQuantityMap[order.GetPrice()] += order.GetQuantity();
        }

        vector<Order> aggregatedOrders;
        for (const auto& [price, quantity] : priceQuantityMap) {
            aggregatedOrders.emplace_back(price, quantity, side);
        }

        return aggregatedOrders;
    };

    const auto& bidStack = orderBooks[productId].GetBidStack();
    const auto& offerStack = orderBooks[productId].GetOfferStack();

    vector<Order> aggregatedBidStack = aggregateStack(bidStack, BID);
    vector<Order> aggregatedOfferStack = aggregateStack(offerStack, OFFER);

    return OrderBook<T>(orderBooks[productId].GetProduct(), aggregatedBidStack, aggregatedOfferStack);
}

/**
 * Connector for the BondMarketDataService, used to subscribe and publish data.
 */
template <typename T>
class BondMarketDataConnector : public Connector<OrderBook<T>> {
   private:
    BondMarketDataService<T>* service;  // Associated market data service

   public:
    // Constructor and destructor
    BondMarketDataConnector(BondMarketDataService<T>* _service);
    virtual ~BondMarketDataConnector();

    // Publish data to the connector
    void Publish(OrderBook<T>& _data);

    // Subscribe to data from the connector
    void Subscribe(ifstream& _data);
};

template <typename T>
BondMarketDataConnector<T>::BondMarketDataConnector(BondMarketDataService<T>* _service) : service(_service) {}

template <typename T>
BondMarketDataConnector<T>::~BondMarketDataConnector() {}

template <typename T>
void BondMarketDataConnector<T>::Publish(OrderBook<T>& _data) {}

template <typename T>
void BondMarketDataConnector<T>::Subscribe(ifstream& dataStream) {
    const int bookDepth = service->GetBookDepth();
    const int batchSize = bookDepth * 2;
    vector<Order> bidOrders, offerOrders;

    auto parseOrder = [](const string& line) -> tuple<string, Order> {
        stringstream ss(line);
        vector<string> tokens;
        string token;

        while (getline(ss, token, ',')) {
            tokens.push_back(token);
        }

        string productId = tokens[0];
        double price = ParsePrice(tokens[1]);
        long quantity = stol(tokens[2]);
        PricingSide side = (tokens[3] == "BID") ? BID : OFFER;

        return {productId, Order(price, quantity, side)};
    };

    long orderCount = 0;
    string line;
    while (getline(dataStream, line)) {
        auto [productId, order] = parseOrder(line);

        if (order.GetSide() == BID) {
            bidOrders.push_back(order);
        } else {
            offerOrders.push_back(order);
        }

        if (++orderCount % batchSize == 0) {
            T product = BondInfo(productId);
            OrderBook<T> orderBook(product, bidOrders, offerOrders);
            service->OnMessage(orderBook);

            bidOrders.clear();
            offerOrders.clear();
        }
    }
}

#endif
