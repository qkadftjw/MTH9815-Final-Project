/**
* streamingservice.hpp
* Provides functionality for publishing two-way price streams.
*
* @author Breman Thuraisingham, Fangtong Wang
*/

#ifndef STREAMING_SERVICE_HPP
#define STREAMING_SERVICE_HPP

#include "soa.hpp"
#include "algostreamingservice.hpp"

// Forward declarations to resolve dependencies
template<typename T>
class ListenerStreamToAlgoStream;

/**
* Manages and publishes price streams for products.
* @tparam T The product type.
*/
template<typename T>
class StreamingService : public Service<string, PriceStream<T>>
{

private:
    map<string, PriceStream<T>> priceStreams; // Stores price streams keyed by product ID
    vector<ServiceListener<PriceStream<T>>*> listeners; // Listeners for price stream updates
    ServiceListener<AlgoStream<T>>* listener; // Listener for AlgoStream updates

public:
    StreamingService();  // Default constructor
    virtual ~StreamingService() = default;  // Default destructor

    PriceStream<T>& GetData(string _key);  // Retrieve a price stream by key
    void OnMessage(PriceStream<T>& _data);  // Handle new or updated price streams
    void AddListener(ServiceListener<PriceStream<T>>* _listener);  // Register a listener
    const vector<ServiceListener<PriceStream<T>>*>& GetListeners() const;  // Get all listeners
    ServiceListener<AlgoStream<T>>* GetListener();  // Get the listener for AlgoStream
    void PublishPrice(PriceStream<T>& _priceStream);  // Notify listeners of new price streams
};

template<typename T>
StreamingService<T>::StreamingService(): 
    priceStreams(),
    listeners(),
    listener(new ListenerStreamToAlgoStream<T>(this))
{
}

template<typename T>
PriceStream<T>& StreamingService<T>::GetData(string _key)
{
    return priceStreams[_key];
}

template<typename T>
void StreamingService<T>::OnMessage(PriceStream<T>& _data)
{
    priceStreams[_data.GetProduct().GetProductId()] = _data;
}

template<typename T>
void StreamingService<T>::AddListener(ServiceListener<PriceStream<T>>* _listener)
{
    listeners.push_back(_listener);
}

template<typename T>
const vector<ServiceListener<PriceStream<T>>*>& StreamingService<T>::GetListeners() const
{
    return listeners;
}

template<typename T>
ServiceListener<AlgoStream<T>>* StreamingService<T>::GetListener()
{
    return listener;
}

template<typename T>
void StreamingService<T>::PublishPrice(PriceStream<T>& _priceStream)
{
    for (auto& l : listeners)
    {
        l->ProcessAdd(_priceStream);
    }
}

/**
* Handles interactions between AlgoStreamingService and StreamingService.
* @tparam T The product type.
*/
template<typename T>
class ListenerStreamToAlgoStream : public ServiceListener<AlgoStream<T>>
{

private:
    StreamingService<T>* service; // Reference to the parent StreamingService

public:
    ListenerStreamToAlgoStream(StreamingService<T>* _service);  // Constructor
    virtual ~ListenerStreamToAlgoStream() = default;  // Default destructor

    void ProcessAdd(AlgoStream<T>& _data);  // Handle new AlgoStream additions
    void ProcessRemove(AlgoStream<T>& _data);  // Handle AlgoStream removals (not implemented)
    void ProcessUpdate(AlgoStream<T>& _data);  // Handle AlgoStream updates (not implemented)
};

template<typename T>
ListenerStreamToAlgoStream<T>::ListenerStreamToAlgoStream(StreamingService<T>* newService) : service(newService)
{
}

template<typename T>
void ListenerStreamToAlgoStream<T>::ProcessAdd(AlgoStream<T>& _data)
{
    PriceStream<T>* _priceStream = _data.GetPriceStream();
    service->OnMessage(*_priceStream);  // Update the service with the new price stream
    service->PublishPrice(*_priceStream);  // Notify listeners of the new price stream
}

template<typename T>
void ListenerStreamToAlgoStream<T>::ProcessRemove(AlgoStream<T>& _data) {}

template<typename T>
void ListenerStreamToAlgoStream<T>::ProcessUpdate(AlgoStream<T>& _data) {}

#endif
