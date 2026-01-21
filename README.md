# Milestone 2 Game Engine Documentation

## Milestone 1

Task 1: Core graphics setup is handled by the Engine.h/.cpp files

Task 2: Generic entity system is handled by the Entity.h/.cpp files and used by the Engine files

Task 3: Physics system is handled by the Physics.h/.cpp files and used by the Entity files

Task 4: Input handling system is handled by the Input.h/.cpp files

Task 5: Collision detections is handled by the Collision.h/.cpp files and used by the Entity files

Types.h is a header for the OrderedPair and Velocity structs

## Milestone 2

Task 1: Measuring and representing time is handled by the Timeline.h/.cpp files and used by our game's main.cpp files

Task 2: The client server system is handled by Client.h/.cpp, Server.cpp, NetworkTypes.h and used by our game's main.cpp files

Task 3 & 4: Multithreaded loop architecture and asynchronicity is handled by Engine.cpp (worker threads for updating player entities), 
            Server.cpp (client threads and shared moving object thread), and used by our game's main.cpp files
