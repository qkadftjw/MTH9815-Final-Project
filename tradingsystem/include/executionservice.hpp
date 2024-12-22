/**
 * executionservice.hpp
 * Defines the data types and Service for executions.
 *
 * @author Breman Thuraisingham, Fangtong Wang
 */

#ifndef EXECUTION_SERVICE_HPP
#define EXECUTION_SERVICE_HPP

#include <memory>
#include <string>
#include "algoexecutionservice.hpp"
#include "marketdataservice.hpp"
#include "soa.hpp"
#include "utils.hpp"


template <typename T>
class ListenerExeToAlgoExe;


template <typename T>
class ExecutionService : public Service<string, ExecutionOrder<T>> {
   public:
    // Constructor and destructor
    ExecutionService();
    virtual ~ExecutionService() = default;

    // Get data on our service given a key
    ExecutionOrder<T>& GetData(string key);

    // The callback that a Connector should invoke for any new or updated data
    void OnMessage(ExecutionOrder<T>& data);

    // Add a listener to the Service for callbacks on add, remove, and update events for data to the Service
    void AddListener(ServiceListener<ExecutionOrder<T>>* listener);

    // Get all listeners on the Service
    const vector<ServiceListener<ExecutionOrder<T>>*>& GetListeners() const;

    // Get the listener of the service
    ListenerExeToAlgoExe<T>* GetListener();

    // Execute an order on a market
    void ProcessExecution(ExecutionOrder<T>& _executionOrder);

   private:
    map<string, ExecutionOrder<T>> executionOrders;
    vector<ServiceListener<ExecutionOrder<T>>*> listeners;
    ListenerExeToAlgoExe<T>* listener;
};

template <typename T>
ExecutionService<T>::ExecutionService() {
    executionOrders = map<string, ExecutionOrder<T>>();
    listeners = vector<ServiceListener<ExecutionOrder<T>>*>();
    listener = new ListenerExeToAlgoExe<T>(this);
}

template <typename T>
ExecutionOrder<T>& ExecutionService<T>::GetData(string key) {
    return executionOrders[key];
}

template <typename T>
void ExecutionService<T>::OnMessage(ExecutionOrder<T>& data) {
    executionOrders[data.GetProduct().GetProductId()] = data;
}

template <typename T>
void ExecutionService<T>::AddListener(ServiceListener<ExecutionOrder<T>>* listener) {
    listeners.push_back(listener);
}

template <typename T>
const vector<ServiceListener<ExecutionOrder<T>>*>& ExecutionService<T>::GetListeners() const {
    return listeners;
}

template <typename T>
ListenerExeToAlgoExe<T>* ExecutionService<T>::GetListener() {
    return listener;
}

template <typename T>
void ExecutionService<T>::ProcessExecution(ExecutionOrder<T>& _executionOrder) {
    string _productId = _executionOrder.GetProduct().GetProductId();
    executionOrders[_productId] = _executionOrder;

    for (auto& l : listeners) {
        l->ProcessAdd(_executionOrder);
    }
}


template <typename T>
class ListenerExeToAlgoExe : public ServiceListener<AlgoExecution<T>> {
   public:
    // Connector and Destructor
    ListenerExeToAlgoExe(ExecutionService<T>* newService);
    virtual ~ListenerExeToAlgoExe() = default;

    // Listener callback to process an add event to the Service
    void ProcessAdd(AlgoExecution<T>& data);

    // Listener callback to process a remove event to the Service
    void ProcessRemove(AlgoExecution<T>& data);

    // Listener callback to process an update event to the Service
    void ProcessUpdate(AlgoExecution<T>& data);

   private:
    ExecutionService<T>* service;
};

template <typename T>
ListenerExeToAlgoExe<T>::ListenerExeToAlgoExe(ExecutionService<T>* newService) : service(newService)
{
}

template <typename T>
void ListenerExeToAlgoExe<T>::ProcessAdd(AlgoExecution<T>& _data) {
    ExecutionOrder<T>* _executionOrder = _data.RetrieveExecutionOrder();
    service->OnMessage(*_executionOrder);
    service->ProcessExecution(*_executionOrder);
}

template <typename T>
void ListenerExeToAlgoExe<T>::ProcessRemove(AlgoExecution<T>& _data) {}

template <typename T>
void ListenerExeToAlgoExe<T>::ProcessUpdate(AlgoExecution<T>& _data) {}

#endif
