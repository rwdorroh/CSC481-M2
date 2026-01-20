#include <zmq.hpp>
#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <sstream>
#include <mutex>
#include <atomic>
#include <vector>
#include <cmath>
#include "../include/engine/NetworkTypes.h"
#include "../include/engine/Types.h"

#define THREADS 1

// Player positions tracked by ID
std::unordered_map<int, std::pair<float, float>> players;
std::mutex playersMutex;

// Generic synchronized objects
struct SyncedObject {
    OrderedPair position;
    OrderedPair velocity;
    int type;  // 0 = platform, 1 = enemy, 2 = powerup, etc.
    int id;
};

std::unordered_map<int, SyncedObject> syncedObjects;
std::mutex objectsMutex;

std::atomic<bool> running{ true };

// Initialize synchronized objects (each game can customize this)
void initializeSyncedObjects() {
    std::lock_guard<std::mutex> lock(objectsMutex);
    
    // Harrison's moving platform (ID 0)
    syncedObjects[0] = {{1100.0f, 700.0f}, {150.0f, 0.0f}, 0, 0};

	// Riley's moving orb (ID 1)
	syncedObjects[1] = { {1920.0f - 128.0f, 0.0f}, {-400.0f, 180.0f}, 1, 1 };
    
    // Other team members can add their objects here with different IDs
    // syncedObjects[1] = {{500.0f, 300.0f}, {0.0f, 100.0f}, 1, 1}; // Example enemy
    // syncedObjects[2] = {{200.0f, 400.0f}, {50.0f, 0.0f}, 2, 2}; // Example powerup
}

// Reply handler per client
void reply_handler(zmq::context_t& context, int clientID) {
    zmq::socket_t responder(context, zmq::socket_type::rep);
    int port = 5556 + clientID;
    responder.bind("tcp://*:" + std::to_string(port));
    std::cout << "[Server] Listening for client " << clientID << " on port " << port << "\n";

    while (running) {
        zmq::message_t request;
        if (!responder.recv(request, zmq::recv_flags::none)) continue;

        std::string req(static_cast<char*>(request.data()), request.size());
        std::istringstream iss(req);
        std::string cmd; int id, tick; uint32_t actions; float x, y;
        if (iss >> cmd >> id >> tick >> actions >> x >> y) {
            if (cmd == "CMD") {
                std::lock_guard<std::mutex> lock(playersMutex);
                players[id] = { x, y };
            }
        }

        static const std::string ack = "Acknowledged";
        responder.send(zmq::buffer(ack), zmq::send_flags::none);
    }
}

// Publisher handler sends snapshots to clients
void pub_handler(zmq::context_t& context) {
    zmq::socket_t publisher(context, zmq::socket_type::pub);
    publisher.bind("tcp://*:5555");
    std::cout << "[Server] Publishing updates on tcp://*:5555\n";

    // Initialize synced objects
    initializeSyncedObjects();

    int tick = 0;
    while (running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(33));
        ++tick;

        // Update all synchronized objects
        {
            std::lock_guard<std::mutex> lock(objectsMutex);
            for (auto& [id, obj] : syncedObjects) {
                if (obj.type == 0) { // Platform logic - Harrison's moving platform
                    obj.position.x += obj.velocity.x * 0.033f;
                    
                    // Platform boundaries
                    if (obj.position.x <= 1000.0f) {
                        obj.position.x = 1000.0f;
                        obj.velocity.x = 150.0f;
                    }
                    else if (obj.position.x + 200.0f >= 1500.0f) {
                        obj.position.x = 1500.0f - 200.0f;
                        obj.velocity.x = -150.0f;
                    }
				}
				else if (obj.type == 1) {
					// ORB: integrate velocity
					obj.position.x += obj.velocity.x * 0.033f;
					obj.position.y += obj.velocity.y * 0.033f;

					// Respawn when fully off left or below bottom
					if (obj.position.x + 128.0f < 0.0f || obj.position.y > 1080.0f) {
						obj.position.x = 1920.0 - 128.0f;
						obj.position.y = 0.0f;
					}
				}
                // Add logic for other object types here
                // else if (obj.type == 1) { // Enemy logic }
                // else if (obj.type == 2) { // Powerup logic }
            }
        }

        // Build snapshot: SNAP <tick> <numPlayers> <numObjects> [id x y]... [objId objType objX objY]...
        std::ostringstream oss;
        oss << "SNAP " << tick << " ";

        // Copy players safely
        std::vector<std::tuple<int, float, float>> playersCopy;
        {
            std::lock_guard<std::mutex> lock(playersMutex);
            playersCopy.reserve(players.size());
            for (const auto& kv : players) {
                playersCopy.emplace_back(kv.first, kv.second.first, kv.second.second);
            }
        }

        // Sort players by ID
        std::sort(playersCopy.begin(), playersCopy.end(),
            [](const auto& a, const auto& b) { return std::get<0>(a) < std::get<0>(b); });

        // Output counts
        oss << playersCopy.size() << " " << syncedObjects.size() << " ";

        // Output players
        for (const auto& p : playersCopy) {
            oss << std::get<0>(p) << " " << std::get<1>(p) << " " << std::get<2>(p) << " ";
        }

        // Output synchronized objects
        {
            std::lock_guard<std::mutex> lock(objectsMutex);
            for (const auto& [id, obj] : syncedObjects) {
                oss << obj.id << " " << obj.type << " " << obj.position.x << " " << obj.position.y << " ";
            }
        }

        const std::string snap = oss.str();
        publisher.send(zmq::buffer(snap), zmq::send_flags::none);
    }
}

// Thread for synchronized objects updates
void objects_thread_func() {
    using namespace std::chrono;
    auto last = steady_clock::now();

    while (running) {
        auto now = steady_clock::now();
        duration<float> delta = now - last;
        last = now;

        {
            std::lock_guard<std::mutex> lock(objectsMutex);
            for (auto& [id, obj] : syncedObjects) {
                if (obj.type == 0) { // Platform logic
                    obj.position.x += obj.velocity.x * delta.count();
                    
                    // Platform boundaries
                    if (obj.position.x <= 1000.0f) {
                        obj.position.x = 1000.0f;
                        obj.velocity.x = 150.0f;
                    }
                    else if (obj.position.x + 200.0f >= 1500.0f) {
                        obj.position.x = 1500.0f - 200.0f;
                        obj.velocity.x = -150.0f;
                    }
                }
                // Add logic for other object types here
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
}

int main() {
    zmq::context_t context(THREADS);

    // Start objects update thread
    std::thread objectsThread(objects_thread_func);

    // Publisher thread for world snapshots
    std::thread pubThread(pub_handler, std::ref(context));

    // Reply threads for clients
    std::thread rep0(reply_handler, std::ref(context), 0);
    std::thread rep1(reply_handler, std::ref(context), 1);
    std::thread rep2(reply_handler, std::ref(context), 2);

    // Wait for threads to finish
    pubThread.join();
    rep0.join();
    rep1.join();
    rep2.join();

    // Stop objects thread
    running = false;
    objectsThread.join();

    return 0;
}