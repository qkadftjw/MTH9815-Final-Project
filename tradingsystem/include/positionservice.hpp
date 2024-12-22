/**
 * positionservice.hpp
 * Defines the Position class and PositionService for managing financial positions.
 *
 * @authors Breman Thuraisingham, Fangtong Wang
 */

#ifndef POSITION_SERVICE_HPP
#define POSITION_SERVICE_HPP

#include <string>
#include <map>
#include <vector>
#include "soa.hpp"
#include "tradebookingservice.hpp"

using namespace std;

/**
 * @class Position
 * @brief Represents positions for a product within different books.
 * @tparam T The type of the product.
 */
template<typename T>
class Position
{
public:
    // Default constructor
    Position() = default;

    // Constructor initializing with a product
    Position(const T& _product);

    // Retrieve the product associated with the position
    const T& GetProduct() const;

    // Retrieve the position quantity for a specific book
    long GetPosition(string& _book);

    // Retrieve all positions mapped by book
    map<string, long> GetPositions();

    // Update the position quantity for a specific book
    void AddPosition(string& _book, long _position);

    // Calculate and retrieve the aggregate position across all books
    long GetAggregatePosition();

    // Convert attributes to string representations
    vector<string> ToStrings() const;

private:
    T product;                       ///< Product associated with the position
    map<string, long> positions;     ///< Map of book identifiers to position quantities
};

// Implementation of Position class methods
template<typename T>
Position<T>::Position(const T& _product) : product(_product) {}

template<typename T>
const T& Position<T>::GetProduct() const
{
    return product;
}

template<typename T>
long Position<T>::GetPosition(string& _book)
{
    return positions[_book];
}

template<typename T>
map<string, long> Position<T>::GetPositions()
{
    return positions;
}

template<typename T>
void Position<T>::AddPosition(string& _book, long _position)
{
    positions[_book] += _position;
}

template<typename T>
long Position<T>::GetAggregatePosition()
{
    long aggregatePosition = 0;
    for (auto& p : positions)
    {
        aggregatePosition += p.second;
    }
    return aggregatePosition;
}

template<typename T>
vector<string> Position<T>::ToStrings() const
{
    string _product = product.GetProductId();
    vector<string> _positions;
    for (auto& p : positions)
    {
        string _book = p.first;
        string _position = to_string(p.second);
        _positions.push_back(_book);
        _positions.push_back(_position);
    }

    vector<string> _strings;
    _strings.push_back(_product);
    _strings.insert(_strings.end(), _positions.begin(), _positions.end());
    return _strings;
}

template<typename T>
class ListenerPosToTradeBooking;

/**
 * @class PositionService
 * @brief Manages positions across various books and products, keyed by product identifier.
 * @tparam T The type of the product.
 */
template<typename T>
class PositionService : public Service<string, Position<T>>
{
private:
    map<string, Position<T>> positions;                              ///< Map of product identifiers to positions
    vector<ServiceListener<Position<T>>*> listeners;                 ///< List of listeners for service events
    ListenerPosToTradeBooking<T>* listener;                     ///< Listener for trade booking service

public:
    // Constructor and destructor
    PositionService();
    ~PositionService();

    // Retrieve position data using a product identifier
    Position<T>& GetData(string _key);

    // Handle updates or new data via a connector callback
    void OnMessage(Position<T>& _data);

    // Add a listener for service event notifications
    void AddListener(ServiceListener<Position<T>>* _listener);

    // Retrieve all service listeners
    const vector<ServiceListener<Position<T>>*>& GetListeners() const;

    // Retrieve the service's trade booking listener
    ListenerPosToTradeBooking<T>* GetListener();

    // Add a trade to update positions
    virtual void AddTrade(const Trade<T>& _trade);
};

// Implementation of PositionService class methods
template<typename T>
PositionService<T>::PositionService()
{
    positions = map<string, Position<T>>();
    listeners = vector<ServiceListener<Position<T>>*>();
    listener = new ListenerPosToTradeBooking<T>(this);
}

template<typename T>
PositionService<T>::~PositionService() {}

template<typename T>
Position<T>& PositionService<T>::GetData(string _key)
{
    return positions[_key];
}

template<typename T>
void PositionService<T>::OnMessage(Position<T>& _data)
{
    positions[_data.GetProduct().GetProductId()] = _data;
}

template<typename T>
void PositionService<T>::AddListener(ServiceListener<Position<T>>* _listener)
{
    listeners.push_back(_listener);
}

template<typename T>
ListenerPosToTradeBooking<T>* PositionService<T>::GetListener()
{
    return listener;
}

template<typename T>
const vector<ServiceListener<Position<T>>*>& PositionService<T>::GetListeners() const
{
    return listeners;
}

template<typename T>
void PositionService<T>::AddTrade(const Trade<T>& _trade)
{
    T _product = _trade.GetProduct();
    string _productId = _product.GetProductId();
    double _price = _trade.GetPrice();
    string _book = _trade.GetBook();
    long _quantity = _trade.GetQuantity();
    Side _side = _trade.GetSide();
    Position<T> _positionTo(_product);
    switch (_side)
    {
    case BUY:
        _positionTo.AddPosition(_book, _quantity);
        break;
    case SELL:
        _positionTo.AddPosition(_book, -_quantity);
        break;
    }

    Position<T> _positionFrom = positions[_productId];
    map<string, long> _positionMap = _positionFrom.GetPositions();
    for (auto& p : _positionMap)
    {
        _book = p.first;
        _quantity = p.second;
        _positionTo.AddPosition(_book, _quantity);
    }
    positions[_productId] = _positionTo;

    for (auto& l : listeners)
    {
        l->ProcessAdd(_positionTo);
    }
}

/**
 * @class PositionToTradeBookingListener
 * @brief Listener subscribing to trade booking service to update positions.
 * @tparam T The type of the product.
 */
template<typename T>
class ListenerPosToTradeBooking : public ServiceListener<Trade<T>>
{
private:
    PositionService<T>* service; ///< Pointer to the associated position service

public:
    // Constructor and destructor
    ListenerPosToTradeBooking(PositionService<T>* _service);
    virtual ~ListenerPosToTradeBooking() = default;

    // Process add events from the trade booking service
    void ProcessAdd(Trade<T>& _data);

    // Process remove events (unused in this context)
    void ProcessRemove(Trade<T>& _data);

    // Process update events (unused in this context)
    void ProcessUpdate(Trade<T>& _data);
};

// Implementation of PositionToTradeBookingListener methods
template<typename T>
ListenerPosToTradeBooking<T>::ListenerPosToTradeBooking(PositionService<T>* _service)
{
    service = _service;
}

template<typename T>
void ListenerPosToTradeBooking<T>::ProcessAdd(Trade<T>& _data)
{
    service->AddTrade(_data);
}

template<typename T>
void ListenerPosToTradeBooking<T>::ProcessRemove(Trade<T>& _data) {}

template<typename T>
void ListenerPosToTradeBooking<T>::ProcessUpdate(Trade<T>& _data) {}

#endif
