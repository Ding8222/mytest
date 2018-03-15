#pragma once
#include <functional>
#include <iostream>
#include <google/protobuf/message.h>

using namespace std;

class Callback
{
public:
	virtual ~Callback() {};
	virtual void onMessage(google::protobuf::Message* message) const = 0;
};


template <typename T>
class CallbackT : public Callback
{
public:
	typedef function<void(T* message)> ProtobufMessageCallback;

	CallbackT(const ProtobufMessageCallback& callback)
		: callback_(callback)
	{
	}

	virtual void onMessage(google::protobuf::Message* message) const
	{
		T* t = dynamic_cast<T*>(message);
		assert(t != NULL);
		callback_(t);
	}

private:
	ProtobufMessageCallback callback_;
};

static void discardProtobufMessage(google::protobuf::Message* message)
{
	cout << "Discarding " << message->GetTypeName() << endl;
}

class ProtobufDispatcher
{
public:

	ProtobufDispatcher()
		: defaultCallback_(discardProtobufMessage)
	{
	}

	void onMessage(google::protobuf::Message* message) const
	{
		CallbackMap::const_iterator it = callbacks_.find(message->GetDescriptor());
		if (it != callbacks_.end())
		{
			it->second->onMessage(message);
		}
		else
		{
			defaultCallback_(message);
		}
	}

	template<typename T>
	void registerMessageCallback(const typename CallbackT<T>::ProtobufMessageCallback& callback)
	{
		std::shared_ptr<CallbackT<T> > pd(new CallbackT<T>(callback));
		callbacks_[T::descriptor()] = pd;
	}

	typedef std::map<const google::protobuf::Descriptor*, std::shared_ptr<Callback> > CallbackMap;
	CallbackMap callbacks_;
	function<void(google::protobuf::Message* message)> defaultCallback_;
};

// void onQuery1(netData::PlayerList* query)
// {
// 	cout << "onQuery: " << query->GetTypeName() << endl;
// }
// 
// void onAnswer1(netData::PlayerList* answer)
// {
// 	cout << "onAnswer: " << answer->GetTypeName() << endl;
// }
// 
// int test3()
// {
// 	ProtobufDispatcher dispatcher;
// 	dispatcher.registerMessageCallback<netData::PlayerList>(onQuery1);
// 	dispatcher.registerMessageCallback<netData::PlayerList>(onAnswer1);
// 
// 	netData::PlayerList q;
// 	q.set_account("111");
// 	cout << "onQuery: " << q.account() << endl;
// 	cout << "onQuery: " << q.GetTypeName() << endl;
// 	netData::PlayerList a;
// 	a.set_account("222");
// 	netData::PlayerList e;
// 	e.set_account("333");
// 	dispatcher.onMessage(&q);
// 	dispatcher.onMessage(&a);
// 	dispatcher.onMessage(&e);
// 
// 	google::protobuf::ShutdownProtobufLibrary();
// 	return 0;
// }
