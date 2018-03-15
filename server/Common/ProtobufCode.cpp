#include "Login.pb.h"
#include "ProtobufCode.h"

void testQuery()
{
	netData::PlayerList query;
	query.set_account("Ding");

	std::string transport = encode(query);
	std::cout << transport << std::endl;;

	int32_t be32 = 0;
	std::copy(transport.begin(), transport.begin() + sizeof be32, reinterpret_cast<char*>(&be32));
	int32_t len = ::ntohl(be32);
	assert(len == transport.size() - sizeof(be32));

	// network library decodes length header and get the body of message
	std::string buf = transport.substr(sizeof(int32_t));
	assert(len == buf.size());

	netData::PlayerList* newQuery = dynamic_cast<netData::PlayerList*>(decode(buf));
	assert(newQuery != NULL);
	newQuery->PrintDebugString();
	assert(newQuery->DebugString() == query.DebugString());
	delete newQuery;

	buf[buf.size() - 6]++;  // oops, some data is corrupted
	netData::PlayerList* badQuery = dynamic_cast<netData::PlayerList*>(decode(buf));
	assert(badQuery == NULL);
}

void testEmpty()
{
	netData::PlayerList empty;

	std::string transport = encode(empty);
	std::cout << transport << std::endl;;

	std::string buf = transport.substr(sizeof(int32_t));

	netData::PlayerList* newEmpty = dynamic_cast<netData::PlayerList*>(decode(buf));
	assert(newEmpty != NULL);
	newEmpty->PrintDebugString();
	assert(newEmpty->DebugString() == empty.DebugString());
	delete newEmpty;
}

void testAnswer()
{
	netData::PlayerList answer;
	answer.set_account("Ding");

	std::string transport = encode(answer);
	std::cout << transport << std::endl;;

	int32_t be32 = 0;
	std::copy(transport.begin(), transport.begin() + sizeof be32, reinterpret_cast<char*>(&be32));
	int32_t len = ::ntohl(be32);
	assert(len == transport.size() - sizeof(be32));

	// network library decodes length header and get the body of message
	std::string buf = transport.substr(sizeof(int32_t));
	assert(len == buf.size());

	netData::PlayerList* newAnswer = dynamic_cast<netData::PlayerList*>(decode(buf));
	assert(newAnswer != NULL);
	newAnswer->PrintDebugString();
	assert(newAnswer->DebugString() == answer.DebugString());
	delete newAnswer;

	buf[buf.size() - 6]++;  // oops, some data is corrupted
	netData::PlayerList* badAnswer = dynamic_cast<netData::PlayerList*>(decode(buf));
	assert(badAnswer == NULL);
}


int test()
{
	GOOGLE_PROTOBUF_VERIFY_VERSION;

	testQuery();
	puts("");
	testAnswer();
	puts("");
	testEmpty();
	puts("");

	puts("All pass!!!");

	google::protobuf::ShutdownProtobufLibrary();
	return 0;
}