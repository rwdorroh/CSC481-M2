#include <engine/Client.h>
#include <sstream>
#include <iostream>

// static member init
int Client::clientID = 0;

Client::Client()
    : context(1),
    requester(context, zmq::socket_type::req),
    subscriber(context, zmq::socket_type::sub) {
}

Client::~Client() {
    requester.close();
    subscriber.close();
    context.close();
}

void Client::setClientID(int id) {
	clientID = id;
}

bool Client::connect(const std::string& serverAddress) {
	try {
		requester = zmq::socket_t(context, zmq::socket_type::req);
		subscriber = zmq::socket_t(context, zmq::socket_type::sub);

		// avoid hard hangs if server hiccups
		requester.set(zmq::sockopt::rcvtimeo, 1);
		requester.set(zmq::sockopt::sndtimeo, 1000);
		requester.set(zmq::sockopt::linger, 0);

		const std::string reqAddr = serverAddress + ":" + std::to_string(5556 + clientID);
		requester.connect(reqAddr);
		std::cout << "[Client] Connected REQ to " << reqAddr << "\n";

		subscriber.connect(serverAddress + ":5555");
		subscriber.set(zmq::sockopt::subscribe, "");
		std::cout << "[Client] Connected SUB to " << serverAddress << ":5555\n";
		return true;
	}
	catch (const zmq::error_t& e) {
		std::cerr << "[ZMQ Error] " << e.what() << " (code " << e.num() << ")\n";
		return false;
	}
}

void Client::sendCommand(const ClientCommand& cmd) {
	// If previous request hasn't been acked, try to pull it now (non-blocking).
	if (awaitingReply) {
		zmq::message_t pending;
		auto got = requester.recv(pending, zmq::recv_flags::dontwait);
		if (got) {
			awaitingReply = false; // ack drained, we may send again
		}
		else {
			return; // skip sending this frame; REQ must strictly alternate
		}
	}

	std::ostringstream oss;
	oss << "CMD " << cmd.clientId << " " << cmd.tick << " " << cmd.actions
		<< " " << cmd.x << " " << cmd.y;
	std::string msg = oss.str();

	zmq::message_t request(msg.size());
	memcpy(request.data(), msg.data(), msg.size());
	requester.send(request, zmq::send_flags::none);
	awaitingReply = true; // we must receive before next send
}

bool Client::pollUpdate(WorldSnapshot& out) {
	zmq::message_t msg;
	if (!subscriber.recv(msg, zmq::recv_flags::dontwait)) return false;

	std::string data(static_cast<char*>(msg.data()), msg.size());
	std::istringstream iss(data);

	std::string tag; int tick, playerCount, objectCount;
	if (!(iss >> tag) || tag != "SNAP") return false;
	if (!(iss >> tick >> playerCount >> objectCount)) return false;

	out.tick = tick;
	out.playerIds.resize(playerCount);
	out.playerPositions.resize(playerCount);
	out.syncedObjects.resize(objectCount);

	for (int i = 0; i < playerCount; ++i) {
		int id; float x, y;
		if (!(iss >> id >> x >> y)) return false;
		out.playerIds[i] = id;
		out.playerPositions[i] = { x, y };
	}
	
	// Read synchronized objects (id, type, x, y)
	for (int j = 0; j < objectCount; ++j) {
		int id, type; float x, y;
		if (!(iss >> id >> type >> x >> y)) return false;
		out.syncedObjects[j] = { id, type, {x, y} };
	}
	
	return true;
}