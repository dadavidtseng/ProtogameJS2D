# Game Core Module

[Root Directory](../../CLAUDE.md) > [Code](../) > **Game**

## Module Responsibilities

The Game Core module serves as the primary C++ application layer that bridges the DaemonEngine foundation with JavaScript game logic. It implements the main game class, entity system, and provides the core infrastructure for dual-language game development.

## Entry and Startup

### Main Entry Point
- **File**: `Framework/Main_Windows.cpp`
- **Function**: `WinMain()` - Standard Windows application entry
- **Flow**: 
  1. Creates App instance (`g_app = new App()`)
  2. Calls `App::Startup()`
  3. Runs main loop (`App::RunMainLoop()`)
  4. Handles shutdown and cleanup

### Application Framework
- **File**: `Framework/App.cpp/hpp`
- **Responsibilities**: 
  - Application lifecycle management
  - Main loop coordination (BeginFrame → Update → Render → EndFrame)
  - V8 scripting system initialization
  - Input system setup and cursor management

## External Interfaces

### JavaScript Bridge Interface
- **File**: `Framework/GameScriptInterface.cpp/hpp`
- **Pattern**: IScriptableObject implementation
- **Key Methods**:
  - `CallMethod()` - C++ method invocation from JavaScript
  - `GetProperty()` / `SetProperty()` - Property access
  - Hot-reload system integration (`InitializeHotReload()`)
- **Bridge Functions**: CreateCube, MoveProp, MovePlayerCamera, GetPlayerPosition

### Game State Management
- **File**: `Game.cpp/hpp`
- **Public API**:
  - `UpdateJS()` / `RenderJS()` - JavaScript execution entry points
  - `ExecuteJavaScriptCommand()` / `ExecuteJavaScriptFile()` - Script execution
  - Entity management (CreateCube, MoveProp, etc.)
- **Game States**: ATTRACT, GAME (eGameState enum)

## Key Dependencies and Configuration

### Build Configuration
- **Project File**: `Game.vcxproj`
- **Dependencies**: 
  - External Engine project (`../../../Engine/Code/Engine/Engine.vcxproj`)
  - V8 JavaScript engine (`v8-v143-x64.13.0.245.25`)
  - Windows API libraries (winmm.lib, dbghelp.lib, shlwapi.lib)
- **Language Standard**: C++20 with conformance mode
- **Runtime Libraries**: MultiThreadedDebugDLL (Debug), MultiThreadedDLL (Release)

### Engine Integration
- **File**: `EngineBuildPreferences.hpp`
- **Purpose**: Engine feature compilation configuration
- **Include Path**: Engine headers via `$(SolutionDir)../Engine/Code/`

### Hot-Reload System
- **FileWatcher**: Monitors JavaScript file changes
- **ScriptReloader**: Handles V8 context script reloading
- **Integration**: Thread-safe event queue for main thread processing

## Data Models

### Entity System
```cpp
// Base entity class (Entity.cpp/hpp)
class Entity {
    // Common game object functionality
};

// Player entity (Player.cpp/hpp)  
class Player : public Entity {
    // Input handling and movement
};

// Prop entities (Prop.cpp/hpp)
class Prop : public Entity {
    // Static and dynamic world objects
};
```

### Game State Model
```cpp
enum class eGameState : uint8_t {
    ATTRACT,  // Attract screen mode
    GAME      // Active gameplay mode
};

class Game {
    eGameState m_gameState;
    std::vector<Prop*> m_props;  // JavaScript-managed objects
    Player* m_player;
    // Camera and scene objects...
};
```

## Testing and Quality

### Current Testing
- **Integration Testing**: C++/JavaScript bridge validation through test functions
- **Manual Testing**: Runtime validation of hot-reload functionality
- **Debug Support**: Chrome DevTools integration for JavaScript debugging

### Testing Gaps
- No formal unit testing framework
- Limited automated testing coverage
- Manual testing predominant for C++ components

### Quality Tools
- **Compiler**: MSVC Level4 warnings enabled
- **Language Standard**: Full C++20 conformance mode
- **Static Analysis**: Visual Studio built-in analysis

## FAQ

### How does the C++/JavaScript integration work?
The GameScriptInterface class implements IScriptableObject to expose C++ methods to JavaScript. V8Subsystem manages the JavaScript runtime, and the script interface handles type conversion between C++ and JavaScript.

### How does hot-reload work?
FileWatcher monitors script file changes, ScriptReloader handles safe V8 context updates while preserving JavaScript state, and the system supports immediate script reloading without application restart.

### What's the relationship with the external Engine project?
The Game project depends on an external Engine static library that provides core systems (renderer, audio, input, V8 integration). The Engine is referenced via relative path `../../../Engine/Code/Engine/Engine.vcxproj`.

### How are JavaScript errors handled?
JavaScript errors are isolated and don't crash the C++ engine. The V8 integration provides error reporting and the script interface includes robust error handling with fallback behaviors.

## Related File List

### Core Game Files
- `Game.cpp/hpp` - Main game class and state management
- `Entity.cpp/hpp` - Base entity system
- `Player.cpp/hpp` - Player entity implementation
- `Prop.cpp/hpp` - Prop entity implementation

### Framework Infrastructure  
- `Framework/App.cpp/hpp` - Application lifecycle
- `Framework/Main_Windows.cpp` - Windows entry point
- `Framework/GameCommon.cpp/hpp` - Shared definitions
- `Framework/GameScriptInterface.cpp/hpp` - JavaScript bridge
- `Framework/FileWatcher.cpp/hpp` - File monitoring
- `Framework/ScriptReloader.cpp/hpp` - Script hot-reload

### Configuration
- `Game.vcxproj` - Visual Studio project configuration
- `Game.vcxproj.filters` - Project file organization
- `EngineBuildPreferences.hpp` - Engine compilation settings

### Subsystems
- `Subsystem/Light/LightSubsystem.cpp/hpp` - Lighting subsystem example

## Changelog

- **2025-09-21 20:12:07**: Initial module documentation - Comprehensive analysis of C++ game core architecture, JavaScript integration patterns, and hot-reload system implementation.