/**
 * algoexecutionservice.hpp
 * Contains the definitions for data types and services used in algorithmic executions.
 *
 * This header file implements execution orders, algorithmic
 * execution objects, and services for managing these entities.
 *
 * @authors Breman Thuraisingham, Fangtong Wang
 */

#ifndef ALGO_EXECUTION_SERVICE_HPP
#define ALGO_EXECUTION_SERVICE_HPP

#include <string>
#include "marketdataservice.hpp"
#include "soa.hpp"

/**
 * Enum to represent different types of orders.
 */
enum OrderType { FOK, IOC, MARKET, LIMIT, STOP };

/**
 * Enum for supported trading markets.
 */
enum Market { BROKERTEC, ESPEED, CME };

/**
 * Represents an execution order to be sent to an exchange.
 * T represents the type of product.
 */
template <typename T>
class ExecutionOrder {
   public:
    // Constructor to initialize an order.
    ExecutionOrder(const T& _product, PricingSide _side, string _orderId, OrderType _orderType, double _price,
                   long _visibleQuantity, long _hiddenQuantity, string _parentOrderId, bool _isChildOrder);

    ExecutionOrder() = default;

    virtual ~ExecutionOrder() = default;

    // Accessor for the product.
    const T& GetProduct() const;

    // Accessor for the order ID.
    const string& GetOrderId() const;

    // Accessor for the order type.
    OrderType GetOrderType() const;

    // Accessor for the price.
    double GetPrice() const;

    // Accessor for the pricing side.
    PricingSide GetPriceSide() const;

    // Accessor for the visible quantity.
    long GetVisibleQuantity() const;

    // Accessor for the hidden quantity.
    long GetHiddenQuantity() const;

    // Accessor for the parent order ID.
    const string& GetParentOrderId() const;

    // Checks if the order is a child order.
    bool IsChildOrder() const;

    // Converts attributes to string representations.
    vector<string> ToStrings() const;

   private:
    T product;                 // Product information.
    PricingSide side;          // Side of the order (BID or OFFER).
    string orderId;            // Unique order ID.
    OrderType orderType;       // Type of the order.
    double price;              // Price of the order.
    double visibleQuantity;    // Visible quantity of the order.
    double hiddenQuantity;     // Hidden quantity of the order.
    string parentOrderId;      // Parent order ID.
    bool isChildOrder;         // Indicates if it's a child order.
};

/**
 * Implementation of ExecutionOrder constructor.
 */
template <typename T>
ExecutionOrder<T>::ExecutionOrder(const T& _product, PricingSide _side, string _orderId, OrderType _orderType,
                                  double _price, long _visibleQuantity, long _hiddenQuantity, string _parentOrderId,
                                  bool _isChildOrder)
    : product(_product),
      side(_side),
      orderId(_orderId),
      orderType(_orderType),
      price(_price),
      visibleQuantity(_visibleQuantity),
      hiddenQuantity(_hiddenQuantity),
      parentOrderId(_parentOrderId),
      isChildOrder(_isChildOrder) {}

/**
 * Accessor implementations for ExecutionOrder class.
 */
template <typename T>
const T& ExecutionOrder<T>::GetProduct() const {
    return product;
}

template <typename T>
OrderType ExecutionOrder<T>::GetOrderType() const {
    return orderType;
}

template <typename T>
double ExecutionOrder<T>::GetPrice() const {
    return price;
}

template <typename T>
PricingSide ExecutionOrder<T>::GetPriceSide() const {
    return side;
}

template <typename T>
long ExecutionOrder<T>::GetVisibleQuantity() const {
    return visibleQuantity;
}

template <typename T>
long ExecutionOrder<T>::GetHiddenQuantity() const {
    return hiddenQuantity;
}

template <typename T>
const string& ExecutionOrder<T>::GetOrderId() const {
    return orderId;
}

template <typename T>
const string& ExecutionOrder<T>::GetParentOrderId() const {
    return parentOrderId;
}

template <typename T>
bool ExecutionOrder<T>::IsChildOrder() const {
    return isChildOrder;
}

template <typename T>
vector<string> ExecutionOrder<T>::ToStrings() const {
    // Map enums to string representations.
    const unordered_map<PricingSide, string> sideMap = {{BID, "BID"}, {OFFER, "OFFER"}};
    const unordered_map<OrderType, string> orderTypeMap = {
        {FOK, "FOK"}, {IOC, "IOC"}, {MARKET, "MARKET"}, {LIMIT, "LIMIT"}, {STOP, "STOP"}};

    // Convert attributes to string format.
    string productStr = product.GetProductId();
    string sideStr = sideMap.at(side);
    string orderIdStr = orderId;
    string orderTypeStr = orderTypeMap.at(orderType);
    string priceStr = FormatPrice(price);
    string visibleQuantityStr = to_string(visibleQuantity);
    string hiddenQuantityStr = to_string(hiddenQuantity);
    string parentOrderIdStr = parentOrderId;
    string isChildOrderStr = isChildOrder ? "YES" : "NO";

    // Return attributes as a vector of strings.
    return {productStr, sideStr, orderIdStr, orderTypeStr, priceStr,
            visibleQuantityStr, hiddenQuantityStr, parentOrderIdStr, isChildOrderStr};
}

/**
 * Represents an algorithmic execution object.
 */
template <typename T>
class AlgoExecution {
   public:
    // Default constructor.
    AlgoExecution() = default;

    // Constructor to initialize with attributes.
    AlgoExecution(const T& product, PricingSide pricingSide, string orderIdentifier, OrderType orderKind,
                  double orderPrice, long visibleQty, long hiddenQty, string parentOrderIdentifier, bool isChild);

    virtual ~AlgoExecution() = default;

    // Retrieves the execution order.
    ExecutionOrder<T>* RetrieveExecutionOrder() const;

   private:
    ExecutionOrder<T>* execOrder;  // Pointer to the execution order.
};

template <typename T>
AlgoExecution<T>::AlgoExecution(const T& product, PricingSide pricingSide, string orderIdentifier, OrderType orderKind,
                                double orderPrice, long visibleQty, long hiddenQty, string parentOrderIdentifier,
                                bool isChild) {
    execOrder = new ExecutionOrder<T>(product, pricingSide, orderIdentifier, orderKind, orderPrice, visibleQty,
                                      hiddenQty, parentOrderIdentifier, isChild);
}

template <typename T>
ExecutionOrder<T>* AlgoExecution<T>::RetrieveExecutionOrder() const {
    return execOrder;
}

template <typename T>
class ListenerAlgoToMarketData;


/**
 * Service for managing algorithmic execution processes.
 */
template <typename T>
class AlgoExecutionService : public Service<string, AlgoExecution<T>> {
   public:
    // Constructor and destructor.
    AlgoExecutionService();
    virtual ~AlgoExecutionService() = default;

    // Retrieves data associated with a given key.
    AlgoExecution<T>& GetData(string key);

    // Callback for handling new or updated data.
    void OnMessage(AlgoExecution<T>& data);

    // Adds a listener for handling events.
    void AddListener(ServiceListener<AlgoExecution<T>>* newListener);

    // Retrieves all registered listeners.
    const vector<ServiceListener<AlgoExecution<T>>*>& GetListeners() const;

    // Retrieves the associated listener.
    ListenerAlgoToMarketData<T>* GetListener();

    // Executes an order in the market.
    void ExecuteOrder(OrderBook<T>& orderBook);

   private:
    double executionSpread;                                       // Spread for execution.
    long executionCount;                                          // Number of executed orders.
    map<string, AlgoExecution<T>> algoExecutionMap;               // Map of product ID to AlgoExecution.
    vector<ServiceListener<AlgoExecution<T>>*> serviceListeners;  // List of service listeners.
    ListenerAlgoToMarketData<T>* algoListener;                    // Listener for Algo-to-MarketData communication.
};

template <typename T>
AlgoExecutionService<T>::AlgoExecutionService() {
    executionSpread = 1.0 / 128.0;
    executionCount = 0;
    algoExecutionMap = map<string, AlgoExecution<T>>();
    serviceListeners = vector<ServiceListener<AlgoExecution<T>>*>();
    algoListener = new ListenerAlgoToMarketData<T>(this);
}

template <typename T>
AlgoExecution<T>& AlgoExecutionService<T>::GetData(string key) {
    return algoExecutionMap[key];
}

template <typename T>
void AlgoExecutionService<T>::OnMessage(AlgoExecution<T>& data) {
    algoExecutionMap[data.RetrieveExecutionOrder()->GetProduct().GetProductId()] = data;
}

template <typename T>
void AlgoExecutionService<T>::AddListener(ServiceListener<AlgoExecution<T>>* newListener) {
    serviceListeners.push_back(newListener);
}

template <typename T>
const vector<ServiceListener<AlgoExecution<T>>*>& AlgoExecutionService<T>::GetListeners() const {
    return serviceListeners;
}

template <typename T>
ListenerAlgoToMarketData<T>* AlgoExecutionService<T>::GetListener() {
    return algoListener;
}

template <typename T>
void AlgoExecutionService<T>::ExecuteOrder(OrderBook<T>& currentOrderBook) {
    // Retrieve product and product ID.
    T associatedProduct = currentOrderBook.GetProduct();
    string productId = associatedProduct.GetProductId();

    // Variables for the execution order.
    PricingSide selectedSide;
    string uniqueOrderId = GenerateUniqueId();
    double determinedPrice;
    long determinedQuantity;

    // Retrieve the best bid and offer from the order book.
    BidOffer optimalBidOffer = currentOrderBook.GetBestBidOffer();
    Order highestBid = optimalBidOffer.GetBidOrder();
    double highestBidPrice = highestBid.GetPrice();
    long highestBidQuantity = highestBid.GetQuantity();

    Order lowestOffer = optimalBidOffer.GetOfferOrder();
    double lowestOfferPrice = lowestOffer.GetPrice();
    long lowestOfferQuantity = lowestOffer.GetQuantity();

    // Check if the spread meets execution conditions.
    if ((lowestOfferPrice - highestBidPrice) <= executionSpread) {
        // Alternate execution between bid and offer sides.
        if (executionCount % 2 == 0) {
            determinedPrice = highestBidPrice;
            determinedQuantity = highestBidQuantity;
            selectedSide = BID;
        } else {
            determinedPrice = lowestOfferPrice;
            determinedQuantity = lowestOfferQuantity;
            selectedSide = OFFER;
        }

        // Increment the execution counter.
        ++executionCount;

        // Create an AlgoExecution instance.
        AlgoExecution<T> executionInstance(associatedProduct, selectedSide, uniqueOrderId, MARKET, determinedPrice,
                                           determinedQuantity, 0, "", false);

        // Add execution instance to the map.
        algoExecutionMap[productId] = executionInstance;

        // Notify all service listeners.
        for (auto& serviceListener : serviceListeners) {
            serviceListener->ProcessAdd(executionInstance);
        }
    }
}

/**
 * Listener to connect AlgoExecutionService with MarketData.
 */
template <typename T>
class ListenerAlgoToMarketData : public ServiceListener<OrderBook<T>> {
   public:
    ListenerAlgoToMarketData(AlgoExecutionService<T>* _service);
    virtual ~ListenerAlgoToMarketData() = default;

    void ProcessAdd(OrderBook<T>& data);

    void ProcessRemove(OrderBook<T>& _data);

    void ProcessUpdate(OrderBook<T>& data);

   private:
    AlgoExecutionService<T>* service;  // Associated AlgoExecutionService.
};

template <typename T>
ListenerAlgoToMarketData<T>::ListenerAlgoToMarketData(AlgoExecutionService<T>* newService) : service(newService) {}

template <typename T>
void ListenerAlgoToMarketData<T>::ProcessAdd(OrderBook<T>& _data) {
    service->ExecuteOrder(_data);
}

template <typename T>
void ListenerAlgoToMarketData<T>::ProcessRemove(OrderBook<T>& _data) {}

template <typename T>
void ListenerAlgoToMarketData<T>::ProcessUpdate(OrderBook<T>& _data) {}

#endif
