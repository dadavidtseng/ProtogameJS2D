# JavaScript Game Logic Module

[Root Directory](../../../CLAUDE.md) > [Run](../../) > [Data](../) > **Scripts**

## Module Responsibilities

The JavaScript Game Logic module implements the flexible game logic layer using Google V8 JavaScript engine. It provides a sophisticated system registration framework, hot-reloadable game systems, and seamless integration with the C++ engine layer through the GameScriptInterface bridge.

## Entry and Startup

### Primary Entry Points
- **JSEngine.js**: Core JavaScript engine framework with system registration
- **JSGame.js**: Game-specific logic implementation and system coordination
- **InputSystem.js**: Modular input handling system

### Initialization Flow
```javascript
// Automatic initialization in JSGame.js
const jsEngineInstance = new JSEngine();
const jsGameInstance = new JSGame(jsEngineInstance);
jsEngineInstance.setGame(jsGameInstance);
globalThis.JSEngine = jsEngineInstance;
```

### Hot-Reload Integration
- **C++ FileWatcher**: Monitors script file changes
- **C++ ScriptReloader**: Handles V8 context updates
- **Version Detection**: `InputSystem.version = Date.now()` for reload detection

## External Interfaces

### C++ Engine Bridge
**Available C++ Functions** (via GameScriptInterface):
```javascript
// Game object manipulation
game.createCube(x, y, z)           // Create cube at position
game.moveProp(index, x, y, z)      // Move prop by index
game.movePlayerCamera(x, y, z)     // Camera position offset
game.getPlayerPos()                // Get player position

// Game state
game.update(deltaTime, systemTime) // Update C++ engine
game.render()                      // Render C++ engine
game.isAttractMode()               // Check game state
```

### Input System Interface
```javascript
// F1 key detection (InputSystem.js)
input.wasKeyJustPressed(112)       // F1 key code detection

// Global render toggle
globalThis.shouldRender = true/false  // Controls rendering
```

### Chrome DevTools Integration
- **Port**: 9222 (configurable)
- **Feature**: Full JavaScript debugging support
- **Access**: Professional debugging environment for script development

## Key Dependencies and Configuration

### Runtime Dependencies
- **Google V8 Engine**: v13.0.245.25
- **C++ GameScriptInterface**: Type conversion and method bridging
- **DaemonEngine V8Subsystem**: V8 context management

### Global Variables
```javascript
globalThis.JSEngine        // Engine instance
globalThis.jsGameInstance  // Game instance
globalThis.shouldRender    // Render toggle flag
globalThis.InputSystem     // Input system class
```

### System Registration Framework
```javascript
// Register system with JSEngine
engine.registerSystem('systemId', {
    update: (deltaTime) => { /* update logic */ },
    render: () => { /* render logic */ },
    priority: 0,  // Execution order
    enabled: true,
    data: {}     // System-specific data
});
```

## Data Models

### System Registration Model
```javascript
const system = {
    id: 'uniqueId',
    update: function || null,
    render: function || null,
    priority: number,        // Lower = earlier execution
    enabled: boolean,
    data: object            // System state storage
};
```

### Engine Status Model
```javascript
const status = {
    isInitialized: boolean,
    hasGame: boolean,
    frameCount: number,
    systemCount: number,
    updateSystemCount: number,
    renderSystemCount: number,
    pendingOperations: number,
    hotReloadEnabled: boolean
};
```

### Input State Model
```javascript
// InputSystem state management
{
    lastF1State: boolean,
    logTimer: number,
    version: timestamp      // For hot-reload detection
}
```

## Testing and Quality

### Test Scripts
- **File**: `test_scripts.js`
- **Purpose**: Development and testing validation
- **Integration**: Manual testing support

### Hot-Reload Testing
```javascript
// Version-based reload detection
if (InputSystem.version > this.inputSystemVersion) {
    console.log('InputSystem hot-reloaded, creating new instance');
    this.inputSystem = new InputSystem();
    this.inputSystemVersion = InputSystem.version;
}
```

### Error Handling
- **JavaScript Isolation**: Errors don't crash C++ engine
- **System-Level**: Individual system errors logged but don't stop execution
- **Recovery**: Graceful degradation when C++ functions unavailable

## FAQ

### How does the system registration framework work?
JSEngine maintains a registry of systems with update/render callbacks. Systems are executed in priority order during the engine's update/render cycle. This allows modular game logic development and runtime system management.

### How does hot-reload work for JavaScript?
C++ FileWatcher monitors script files, ScriptReloader manages V8 context updates, and JavaScript uses version timestamps to detect reloaded modules. State is preserved across reloads where possible.

### What's the relationship between JSEngine and JSGame?
JSEngine provides the system registration framework and C++ bridge functionality. JSGame implements game-specific logic as registered systems and delegates to specialized modules like InputSystem.

### How does input handling work?
InputSystem.js contains pure input logic extracted for AI agent modification. JSGame registers an input handler system that delegates to InputSystem, providing clean separation of concerns.

### How does the F1 debug toggle work?
InputSystem detects F1 key presses via `input.wasKeyJustPressed(112)` and toggles the global `shouldRender` flag. The render system in JSGame checks this flag before calling C++ render functions.

## Related File List

### Core Framework Files
- `JSEngine.js` - System registration framework and C++ bridge
- `JSGame.js` - Game logic implementation and system coordination
- `InputSystem.js` - Modular input handling system

### Development Files
- `test_scripts.js` - Testing and validation scripts
- `F1_KeyHandler.js` - Legacy F1 key handling (deprecated)

### System Examples
- Game systems: cppBridge, inputHandler, cubeSpawner, propMover, cameraShaker
- Priority-based execution order
- Runtime enable/disable capabilities

## Changelog

- **2025-09-21 20:12:07**: Initial module documentation - Comprehensive analysis of JavaScript system registration framework, hot-reload architecture, and C++ integration patterns with modular input handling design.