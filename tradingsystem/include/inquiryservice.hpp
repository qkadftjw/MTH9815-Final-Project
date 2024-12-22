/**
 * inquiryservice.hpp
 * Defines the data types and Service for customer inquiries.
 *
 * @author Breman Thuraisingham, Fangtong Wang
 */

#ifndef INQUIRY_SERVICE_HPP
#define INQUIRY_SERVICE_HPP

#include "soa.hpp"

// Various inqyury states
enum InquiryState { RECEIVED, QUOTED, DONE, REJECTED, CUSTOMER_REJECTED };
enum Side { BUY, SELL };

using namespace std;

/**
 * Inquiry object modeling a customer inquiry from a client.
 * Type T is the product type.
 */
template <typename T>
class Inquiry {
   public:
    // Default constructor
    Inquiry() = default;

    // Constructor that initializes all fields
    Inquiry(const string &iqId, const T &theProduct, Side inSide, long qty, double inPrice, InquiryState st);

    // Virtual destructor
    virtual ~Inquiry() = default;

    // Get the inquiry ID
    const string &GetInquiryId() const;

    // Get the product
    const T &GetProduct() const;

    // Get the side on the inquiry
    Side GetSide() const;

    // Get the quantity that the client is inquiring for
    long GetQuantity() const;

    // Get the price that we have responded back with
    double GetPrice() const;

    // Set the price that we have responded back with
    void SetPrice(double _price);

    // Get the current state on the inquiry
    InquiryState GetState() const;

    // Set the current state on the inquiry
    void SetState(InquiryState _state);

    // Convert inquiry info into a vector of strings
    vector<string> ToStrings() const;

   private:
    string inquiryId;
    T product;
    Side side;
    long quantity;
    double price;
    InquiryState state;
};

// Implementation of Inquiry constructor
template <typename T>
Inquiry<T>::Inquiry(const string &iqId, const T &theProduct, Side inSide, long qty, double inPrice, InquiryState st)
    : inquiryId(iqId), product(theProduct), side(inSide), quantity(qty), price(inPrice), state(st) {}

// Getter implementations
template <typename T>
const string &Inquiry<T>::GetInquiryId() const {
    return inquiryId;
}

template <typename T>
const T &Inquiry<T>::GetProduct() const {
    return product;
}

template <typename T>
Side Inquiry<T>::GetSide() const {
    return side;
}

template <typename T>
long Inquiry<T>::GetQuantity() const {
    return quantity;
}

template <typename T>
double Inquiry<T>::GetPrice() const {
    return price;
}

template <typename T>
InquiryState Inquiry<T>::GetState() const {
    return state;
}

// Setter implementations
template <typename T>
void Inquiry<T>::SetPrice(double newPrice) {
    price = newPrice;
}

template <typename T>
void Inquiry<T>::SetState(InquiryState newState) {
    state = newState;
}

template <typename T>
vector<string> Inquiry<T>::ToStrings() const {
    // inquiryId
    string strId = inquiryId;

    // productId
    string prodId = product.GetProductId();

    // side
    string strSide;
    switch (side) {
        case BUY:
            strSide = "BUY";
            break;
        case SELL:
            strSide = "SELL";
            break;
    }

    // quantity
    string strQty = to_string(quantity);

    // price
    string strPrice = FormatPrice(price);

    // state
    string strState;
    switch (state) {
        case RECEIVED:
            strState = "RECEIVED";
            break;
        case QUOTED:
            strState = "QUOTED";
            break;
        case DONE:
            strState = "DONE";
            break;
        case REJECTED:
            strState = "REJECTED";
            break;
        case CUSTOMER_REJECTED:
            strState = "CUSTOMER_REJECTED";
            break;
    }

    vector<string> fields;
    fields.push_back(strId);
    fields.push_back(prodId);
    fields.push_back(strSide);
    fields.push_back(strQty);
    fields.push_back(strPrice);
    fields.push_back(strState);
    return fields;
}

/**
 * An abstract base class for an Inquiry Service.
 * Keyed on inquiry identifier. Type T is the product type.
 */
template <typename T>
class InquiryService : public Service<string, Inquiry<T>> {
   public:
    // Default constructor
    InquiryService() = default;

    // Virtual destructor
    virtual ~InquiryService() = default;

    // Send a quote back to the client
    virtual void SendQuote(const string &inquiryId, double price) = 0;

    // Reject an inquiry from the client
    virtual void RejectInquiry(const string &inquiryId) = 0;
};

/**
 * Forward declaration of InquiryConnector for usage within BondInquiryService
 */
template <typename T>
class InquiryConnector;

/**
 * A specialized InquiryService for Bond products.
 */
template <typename T>
class BondInquiryService : public InquiryService<T> {
   public:
    // Constructor
    BondInquiryService();
    // Virtual destructor
    virtual ~BondInquiryService();

    // Return data based on key
    Inquiry<T> &GetData(string key) override;

    // Called by a Connector when new data arrives
    void OnMessage(Inquiry<T> &msg) override;

    // Add a listener
    void AddListener(ServiceListener<Inquiry<T>> *listener) override;

    // Return all listeners
    const vector<ServiceListener<Inquiry<T>> *> &GetListeners() const override;

    // Obtain the connector (for external usage or subscription)
    InquiryConnector<T> *GetConnector();

    // Send a quote back to the client
    void SendQuote(const string &inquiryId, double price) override;

    // Reject an inquiry
    void RejectInquiry(const string &inquiryId) override;

   private:
    map<string, Inquiry<T>> inquiryRecords;
    vector<ServiceListener<Inquiry<T>> *> listenerCollection;
    InquiryConnector<T> *connectorPtr;
};

// Implementation of BondInquiryService
template <typename T>
BondInquiryService<T>::BondInquiryService() {
    inquiryRecords = map<string, Inquiry<T>>();
    listenerCollection = vector<ServiceListener<Inquiry<T>> *>();
    connectorPtr = new InquiryConnector<T>(this);
}

template <typename T>
BondInquiryService<T>::~BondInquiryService() {}

template <typename T>
Inquiry<T> &BondInquiryService<T>::GetData(string key) {
    return inquiryRecords[key];
}

template <typename T>
void BondInquiryService<T>::OnMessage(Inquiry<T> &msg) {
    InquiryState curState = msg.GetState();
    switch (curState) {
        case RECEIVED: {
            // Example: set a default price to 100 if state is RECEIVED
            msg.SetPrice(100);
            inquiryRecords[msg.GetInquiryId()] = msg;
            // Then publish
            connectorPtr->Publish(msg);
            break;
        }
        case QUOTED: {
            // If it's QUOTED, we mark it as DONE
            msg.SetState(DONE);
            inquiryRecords[msg.GetInquiryId()] = msg;
            // Notify all listeners
            for (auto &ls : listenerCollection) {
                ls->ProcessAdd(msg);
            }
            break;
        }
        default:
            // For states not explicitly handled here
            break;
    }
}

template <typename T>
void BondInquiryService<T>::AddListener(ServiceListener<Inquiry<T>> *listener) {
    listenerCollection.push_back(listener);
}

template <typename T>
const vector<ServiceListener<Inquiry<T>> *> &BondInquiryService<T>::GetListeners() const {
    return listenerCollection;
}

template <typename T>
InquiryConnector<T> *BondInquiryService<T>::GetConnector() {
    return connectorPtr;
}

template <typename T>
void BondInquiryService<T>::SendQuote(const string &inquiryId, double price) {
    Inquiry<T> &inq = inquiryRecords[inquiryId];
    inq.SetPrice(price);
    for (auto &ls : listenerCollection) {
        ls->ProcessAdd(inq);
    }
}

template <typename T>
void BondInquiryService<T>::RejectInquiry(const string &inquiryId) {
    Inquiry<T> &inq = inquiryRecords[inquiryId];
    inq.SetState(REJECTED);
}

/**
 * Connector for subscribing/publishing Inquiry data.
 * This connector is associated with the BondInquiryService.
 **/
template <typename T>
class InquiryConnector : public Connector<Inquiry<T>> {
   public:
    // Constructor
    InquiryConnector(BondInquiryService<T> *svcPtr);
    // Virtual destructor
    virtual ~InquiryConnector();

    // Publish data to the Connector
    void Publish(Inquiry<T> &msg) override;

    // Subscribe data from the Connector using an ifstream
    void Subscribe(ifstream &inputFile) override;

    // An overload for direct Inquiry subscription
    void Subscribe(Inquiry<T> &msg);

   private:
    BondInquiryService<T> *servicePtr;
};
template <typename T>
InquiryConnector<T>::InquiryConnector(BondInquiryService<T> *svcPtr) : servicePtr(svcPtr) {}

template <typename T>
InquiryConnector<T>::~InquiryConnector() {}

template <typename T>
void InquiryConnector<T>::Publish(Inquiry<T> &msg) {
    InquiryState curState = msg.GetState();
    if (curState == RECEIVED) {
        msg.SetState(QUOTED);
        Subscribe(msg);
    }
}

template <typename T>
void InquiryConnector<T>::Subscribe(ifstream &inputFile) {
    string fileLine;
    while (getline(inputFile, fileLine)) {
        stringstream lineStream(fileLine);
        string token;
        vector<string> tokens;
        while (getline(lineStream, token, ',')) {
            tokens.push_back(token);
        }

        // Expecting: inquiryId, productId, side, quantity, state
        string iqId = tokens[0];
        string prodId = tokens[1];

        Side s;
        if (tokens[2] == "BUY")
            s = BUY;
        else if (tokens[2] == "SELL")
            s = SELL;

        long qty = stol(tokens[3]);

        InquiryState st;
        if (tokens[4] == "RECEIVED")
            st = RECEIVED;
        else if (tokens[4] == "QUOTED")
            st = QUOTED;
        else if (tokens[4] == "DONE")
            st = DONE;
        else if (tokens[4] == "REJECTED")
            st = REJECTED;
        else if (tokens[4] == "CUSTOMER_REJECTED")
            st = CUSTOMER_REJECTED;

        // Convert product string to a bond/product T - must be provided by user.
        T theBond = BondInfo(prodId);

        // For new inquiries from file, let's assume price=0 (or any placeholder)
        Inquiry<T> newInquiry(iqId, theBond, s, qty, 0.0, st);
        servicePtr->OnMessage(newInquiry);
    }
}

template <typename T>
void InquiryConnector<T>::Subscribe(Inquiry<T> &msg) {
    servicePtr->OnMessage(msg);
}

#endif