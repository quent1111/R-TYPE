ARCHITECTURE — R-TYPE ENGINE (overview)

Goal
----
Provide a clean, solid and easy-to-understand architecture for a networked R-Type-like game. The design below favors modularity, testability and incremental implementation.

High-level modules
------------------
1. engine/
   - core/: low-level helpers, platform abstractions (time, logger, small utilities)
   - ecs/: Entity-Component-System implementation (entity id, sparse_array, registry, zipper, systems)
   - net/: network abstraction & protocol implementation (UDP sockets, packet builders, serialization)
   - render/: rendering abstractions and SFML glue (Renderers, Resources)
   - audio/: audio subsystem wrapper (SFML or chosen lib)
   - utils/: common utilities (math, geometry, serialization helpers)

2. client/
   - client application, glue between engine and render/net; UI, input, client-side prediction logic

3. server/
   - authoritative game server: simulation loop, instance manager, networking code, persistence (if any)

4. tools/
   - editors, asset converters, helper scripts

5. tests/
   - unit and integration tests

6. docs/
   - detailed protocol documentation, developer guide and comparison study

Folder layout (example)
------------------------

/ (repo root)
├─ CMakeLists.txt
├─ README.md
├─ ARCHITECTURE.md
├─ engine/
│  ├─ core/
│  ├─ ecs/
│  ├─ net/
│  ├─ render/
│  ├─ audio/
│  └─ utils/
├─ client/
├─ server/
├─ tools/
├─ tests/
├─ docs/
└─ third_party/

Design principles
-----------------
- Single Responsibility per module.
- DI-friendly (pass dependencies explicitly, avoid globals; when necessary provide thin singletons).
- Keep systems decoupled: networking <-> simulation <-> rendering communicate through well-defined messages/events.
- Use composition (ECS) for game objects.
- Keep server authoritative: clients send inputs, server computes world and sends snapshots.

ECS design (short)
------------------
- `entity_t`: strong typedef around an index (size_t). Created by registry only.
- `sparse_array<T>`: vector<std::optional<T>> storing components by entity index.
- `registry`: map<std::type_index, std::any> mapping component type to sparse_array<T>.
  - register_component<T>() creates array and registers an erase callback to remove component for killed entities.
  - add_component / emplace_component / remove_component templates.
- Systems are callables registered with `add_system<Components...>(callable)`; `run_systems` invokes them with typed arrays.
- Provide `zipper` and `indexed_zipper` to iterate multiple sparse arrays in sync and skip missing components.

Networking design (short)
-------------------------
- Binary protocol over UDP for in-game messages (fast updates). Optional TCP for reliable control messages (login, matchmaking).
- Packet layout: header (message id, sequence, timestamp) + payload. Use fixed-size primitives and explicit packing.
- Server sends periodic snapshots; use sequence numbers for client interpolation and reconciliation.
- Protect against malformed packets: bound checks, length checks, validation.

Build & dependencies
--------------------
- Use CMake (recommended minimum: 3.18+).
- Use Conan (recommended) or vcpkg/CPM for dependencies (SFML, asio, gtest).
- Keep third_party folder only for helper config files; do NOT vend full libs.

Coding conventions
------------------
- Use clang-format and a basic clang-tidy profile. Follow Google or LLVM style (team decision).
- Use explicit `override`, `final` and prefer `= default`/`= delete` for special member functions.

Git and workflow
----------------
- Branch model: feature branches, PRs (merge requests); keep `main` protected.
- Use tags for milestone deliveries.
- CI: at least build+unit tests on push/PR. Cache dependencies to avoid long CI downloads.

Next steps (short roadmap)
-------------------------
1. Implement minimal ECS in `engine/ecs` (entity, sparse_array, registry). Add unit tests.
2. Implement a simple server simulation loop (no network yet) using ECS.
3. Implement client rendering skeleton (SFML) to display entities.
4. Add UDP networking with a simple handshake and snapshot messages.
5. Iterate: systems, gameplay, polishing, protocol docs.

Contact & conventions
---------------------
- Always add docs/ note when you change protocol messages.
- Document public APIs in `docs/` and update `ARCHITECTURE.md` when structure changes.
