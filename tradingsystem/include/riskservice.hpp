/**
 * riskservice.hpp
 * This header file defines data types and services for managing fixed income risk.
 *
 * Authors: Breman Thuraisingham, Fangtong Wang
 */

#ifndef RISK_SERVICE_HPP
#define RISK_SERVICE_HPP

#include "soa.hpp"
#include "positionservice.hpp"

/**
 * @class PV01
 * @brief Represents PV01 risk for a specific product type.
 * @tparam T The type of the product.
 */
template<typename T>
class PV01
{

public:

    // Default constructor
    PV01() = default;

    // Constructor with parameters
    PV01(const T& _product, double _pv01, long _quantity);

    // Retrieve the associated product
    const T& GetProduct() const;

    // Retrieve the PV01 value
    double GetPV01() const;

    // Retrieve the quantity tied to this risk value
    long GetQuantity() const;

    // Update the quantity associated with this risk value
    void SetQuantity(long _quantity);

    // Convert PV01 attributes into string representations
    vector<string> ToStrings() const;

private:
    T product;     ///< The product associated with this PV01
    double pv01;   ///< The PV01 value
    long quantity; ///< The quantity linked to the PV01

};

// Implementation of PV01 methods

template<typename T>
PV01<T>::PV01(const T& _product, double _pv01, long _quantity) :
    product(_product), pv01(_pv01), quantity(_quantity) {}

template<typename T>
const T& PV01<T>::GetProduct() const
{
    return product;
}

template<typename T>
double PV01<T>::GetPV01() const
{
    return pv01;
}

template<typename T>
long PV01<T>::GetQuantity() const
{
    return quantity;
}

template<typename T>
void PV01<T>::SetQuantity(long _quantity)
{
    quantity = _quantity;
}

template<typename T>
vector<string> PV01<T>::ToStrings() const
{
    string _product = product.GetProductId();
    string _pv01 = to_string(pv01);
    string _quantity = to_string(quantity);

    return {_product, _pv01, _quantity};
}

/**
 * @class BucketedSector
 * @brief Groups multiple securities into a sector for aggregated risk analysis.
 * @tparam T The type of the product.
 */
template<typename T>
class BucketedSector
{

public:

    // Default constructor
    BucketedSector() = default;

    // Constructor with a list of products and a sector name
    BucketedSector(const vector<T>& _products, string _name);

    // Retrieve the products in this sector
    const vector<T>& GetProducts() const;

    // Retrieve the name of the sector
    const string& GetName() const;

private:
    vector<T> products; ///< The products included in this sector
    string name;        ///< The name of the sector

};

// Implementation of BucketedSector methods

template<typename T>
BucketedSector<T>::BucketedSector(const vector<T>& _products, string _name) :
    products(_products), name(_name) {}

template<typename T>
const vector<T>& BucketedSector<T>::GetProducts() const
{
    return products;
}

template<typename T>
const string& BucketedSector<T>::GetName() const
{
    return name;
}

// Forward declarations to avoid compilation errors
template<typename T>
class RiskToPositionListener;

/**
 * @class RiskService
 * @brief Provides risk management services for individual securities and bucketed sectors.
 * Keyed by product identifier.
 * @tparam T The type of the product.
 */
template<typename T>
class RiskService : public Service<string, PV01 <T> >
{

private:

    map<string, PV01<T>> pv01s;                                  ///< Map of product IDs to their PV01 values
    vector<ServiceListener<PV01<T>>*> listeners;                ///< Listeners subscribed to the service
    RiskToPositionListener<T>* listener;                        ///< Listener for position updates

public:

    // Constructor and destructor
    RiskService();
    ~RiskService();

    // Retrieve data for a specific product identifier
    PV01<T>& GetData(string _key);

    // Process incoming data updates
    void OnMessage(PV01<T>& _data);

    // Add a listener to the service
    void AddListener(ServiceListener<PV01<T>>* _listener);

    // Retrieve all listeners associated with the service
    const vector<ServiceListener<PV01<T>>*>& GetListeners() const;

    // Retrieve the service's position listener
    RiskToPositionListener<T>* GetListener();

    // Add a position to calculate and update risk
    void AddPosition(Position<T>& _position);

    // Calculate and retrieve risk for a bucketed sector
    const PV01<BucketedSector<T>>& GetBucketedRisk(const BucketedSector<T>& _sector) const;

};

// Implementation of RiskService methods

template<typename T>
RiskService<T>::RiskService()
{
    listener = new RiskToPositionListener<T>(this);
}

template<typename T>
RiskService<T>::~RiskService() {}

template<typename T>
PV01<T>& RiskService<T>::GetData(string _key)
{
    return pv01s[_key];
}

template<typename T>
void RiskService<T>::OnMessage(PV01<T>& _data)
{
    pv01s[_data.GetProduct().GetProductId()] = _data;
}

template<typename T>
void RiskService<T>::AddListener(ServiceListener<PV01<T>>* _listener)
{
    listeners.push_back(_listener);
}

template<typename T>
const vector<ServiceListener<PV01<T>>*>& RiskService<T>::GetListeners() const
{
    return listeners;
}

template<typename T>
RiskToPositionListener<T>* RiskService<T>::GetListener()
{
    return listener;
}

template<typename T>
void RiskService<T>::AddPosition(Position<T>& _position)
{
    T _product = _position.GetProduct();
    string _productId = _product.GetProductId();
    double _pv01Value = PV01Info(_productId);
    long _quantity = _position.GetAggregatePosition();
    PV01<T> _pv01(_product, _pv01Value, _quantity);
    pv01s[_productId] = _pv01;

    for (auto& l : listeners)
    {
        l->ProcessAdd(_pv01);
    }
}

template<typename T>
const PV01<BucketedSector<T>>& RiskService<T>::GetBucketedRisk(const BucketedSector<T>& _sector) const
{
    BucketedSector<T> _product = _sector;
    double _pv01 = 0;
    long _quantity = 1;

    for (const auto& p : _sector.GetProducts())
    {
        string _pId = p.GetProductId();
        _pv01 += pv01s[_pId].GetPV01() * pv01s[_pId].GetQuantity();
    }

    return PV01<BucketedSector<T>>(_product, _pv01, _quantity);
}

/**
 * @class RiskToPositionListener
 * @brief Listens to position updates and forwards them to the RiskService.
 * @tparam T The type of the product.
 */
template<typename T>
class RiskToPositionListener : public ServiceListener<Position<T>>
{

private:

    RiskService<T>* service; ///< Pointer to the associated RiskService

public:

    // Constructor and Destructor
    RiskToPositionListener(RiskService<T>* _service);
    ~RiskToPositionListener();

    // Process an add event
    void ProcessAdd(Position<T>& _data);

    // Unused: Process a remove event
    void ProcessRemove(Position<T>& _data);

    // Unused: Process an update event
    void ProcessUpdate(Position<T>& _data);

};

// Implementation of RiskToPositionListener methods

template<typename T>
RiskToPositionListener<T>::RiskToPositionListener(RiskService<T>* _service) : service(_service) {}

template<typename T>
RiskToPositionListener<T>::~RiskToPositionListener() {}

template<typename T>
void RiskToPositionListener<T>::ProcessAdd(Position<T>& _data)
{
    service->AddPosition(_data);
}

template<typename T>
void RiskToPositionListener<T>::ProcessRemove(Position<T>& _data) {}

template<typename T>
void RiskToPositionListener<T>::ProcessUpdate(Position<T>& _data) {}

#endif
