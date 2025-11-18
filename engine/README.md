# Engine Module

This directory contains the reusable game engine library.

## Modules

- **ecs/**: Entity-Component-System implementation
  - `entity.hpp`: Entity type definition
  - `sparse_array.hpp`: Component storage
  - `registry.hpp`: ECS registry and system management
  - `zipper.hpp`: Multi-component iteration utilities

- **net/**: Networking layer
  - `socket.hpp`: UDP/TCP socket abstractions
  - `packet.hpp`: Binary packet serialization
  - `messages/`: Protocol message definitions

- **render/**: Rendering abstractions
  - SFML wrappers and sprite management

- **audio/**: Audio subsystem
  - Sound and music management

- **core/**: Core utilities
  - Time, logging, platform abstractions

- **utils/**: Common utilities
  - Math, geometry, serialization helpers

## Usage

Each module is a CMake target that can be linked independently:

```cmake
target_link_libraries(my_game PRIVATE
    engine_ecs
    engine_net
    engine_render
)
```

## Building

The engine is built as part of the main project. See root README.md for build instructions.
