/**
* historicaldataservice.hpp
* Defines types and services for managing historical data.
*
* @author Breman Thuraisingham, Fangtong Wang
*/

#ifndef HISTORICAL_DATA_SERVICE_HPP
#define HISTORICAL_DATA_SERVICE_HPP

#include "soa.hpp"
#include <map>

enum ServiceType { POSITION, RISK, EXECUTION, STREAMING, INQUIRY };

// Forward declarations
template<typename T>
class HistoricalDataConnector;
template<typename T>
class HistoricalDataListener;

/**
* Service to process and store historical data.
* @tparam T The data type to manage.
*/
template<typename T>
class HistoricalDataService : Service<string, T>
{
public:
    HistoricalDataService();  // Default constructor
    HistoricalDataService(ServiceType _type);  // Constructor with service type
    virtual ~HistoricalDataService() = default;  // Default destructor

    T& GetData(string _key);  // Access data by key
    void OnMessage(T& _data);  // Handle incoming data
    void AddListener(ServiceListener<T>* _listener);  // Register a listener
    const vector<ServiceListener<T>*>& GetListeners() const;  // Get all listeners
    HistoricalDataConnector<T>* GetConnector();  // Access the connector
    ServiceListener<T>* GetListener();  // Access the listener
    ServiceType GetServiceType() const;  // Get the service type
    void PersistData(string persistKey, T& data);  // Save data

private:
    std::map<string, T> historicalDatas;  // Data storage
    vector<ServiceListener<T>*> listeners;  // Listeners for updates
    HistoricalDataConnector<T>* connector;  // Connector for persistence
    ServiceListener<T>* listener;  // Listener for incoming data
    ServiceType type;  // Type of service
};

template<typename T>
HistoricalDataService<T>::HistoricalDataService()
    : historicalDatas(),
      listeners(),
      connector(new HistoricalDataConnector<T>(this)),
      listener(new HistoricalDataListener<T>(this)),
      type(INQUIRY) {}

// Constructor with specified service type
template<typename T>
HistoricalDataService<T>::HistoricalDataService(ServiceType _type)
    : historicalDatas(),
      listeners(),
      connector(new HistoricalDataConnector<T>(this)),
      listener(new HistoricalDataListener<T>(this)),
      type(_type) {}

// Retrieve data by key
template<typename T>
T& HistoricalDataService<T>::GetData(string _key)
{
    return historicalDatas[_key];
}

// Handle incoming data and store it
template<typename T>
void HistoricalDataService<T>::OnMessage(T& _data)
{
    historicalDatas[_data.GetProduct().GetProductId()] = _data;
}

// Add a listener for service events
template<typename T>
void HistoricalDataService<T>::AddListener(ServiceListener<T>* _listener)
{
    listeners.push_back(_listener);
}

// Access the listener
template<typename T>
ServiceListener<T>* HistoricalDataService<T>::GetListener()
{
    return listener;
}

// Get the type of the service
template<typename T>
ServiceType HistoricalDataService<T>::GetServiceType() const
{
    return type;
}

// Persist data using the connector
template<typename T>
void HistoricalDataService<T>::PersistData(string persistKey, T& data)
{
    connector->Publish(data);
}

// Retrieve all registered listeners
template<typename T>
const vector<ServiceListener<T>*>& HistoricalDataService<T>::GetListeners() const
{
    return listeners;
}

// Access the connector
template<typename T>
HistoricalDataConnector<T>* HistoricalDataService<T>::GetConnector()
{
    return connector;
}

/**
* Connector for saving data to external storage.
* @tparam T The data type to handle.
*/
template<typename T>
class HistoricalDataConnector : public Connector<T>
{
public:
    HistoricalDataConnector(HistoricalDataService<T>* _service);  // Constructor
    virtual ~HistoricalDataConnector() = default;  // Default destructor
    void Publish(T& _data);  // Save data
    void Subscribe(ifstream& _data);  // Placeholder for subscription
private:
    HistoricalDataService<T>* service;  // Parent service
};

// Constructor to initialize the connector with the parent service
template<typename T>
HistoricalDataConnector<T>::HistoricalDataConnector(HistoricalDataService<T>* newService) : service(newService) {}

// Save data to a file based on service type
template<typename T>
void HistoricalDataConnector<T>::Publish(T& data) {
    std::unordered_map<ServiceType, std::string> fileMap = {
        {POSITION, "positions.txt"},
        {RISK, "risk.txt"},
        {EXECUTION, "executions.txt"},
        {STREAMING, "streaming.txt"},
        {INQUIRY, "allinquiries.txt"}
    };

    ServiceType serviceType = service->GetServiceType();
    auto it = fileMap.find(serviceType);

    if (it == fileMap.end()) {
        return; // Skip invalid service type
    }

    std::ofstream outputFile(it->second, std::ios::app);
    if (!outputFile.is_open()) {
        return; // Skip if file cannot be opened
    }

    outputFile << CurrentTimeString() << ",";
    for (const auto& element : data.ToStrings()) {
        outputFile << element << ",";
    }
    outputFile << "\n";
}

// Placeholder for subscription implementation
template<typename T>
void HistoricalDataConnector<T>::Subscribe(ifstream& _data) {}

/**
* Listener for processing incoming data events.
* @tparam T The data type to handle.
*/
template<typename T>
class HistoricalDataListener : public ServiceListener<T>
{
public:
    HistoricalDataListener(HistoricalDataService<T>* _service);  // Constructor
    virtual ~HistoricalDataListener() = default;  // Default destructor
    void ProcessAdd(T& _data);  // Handle add events
    void ProcessRemove(T& _data);  // Handle remove events
    void ProcessUpdate(T& _data);  // Handle update events
private:
    HistoricalDataService<T>* service;  // Parent service
};

// Initialize the listener with the parent service
template<typename T>
HistoricalDataListener<T>::HistoricalDataListener(HistoricalDataService<T>* newService) : service(newService) {}

// Handle adding new data and persist it
template<typename T>
void HistoricalDataListener<T>::ProcessAdd(T& _data)
{
    string _persistKey = _data.GetProduct().GetProductId();
    service->PersistData(_persistKey, _data);
}

// Placeholder for removing data
template<typename T>
void HistoricalDataListener<T>::ProcessRemove(T& _data) {}

// Placeholder for updating data
template<typename T>
void HistoricalDataListener<T>::ProcessUpdate(T& _data) {}

#endif
