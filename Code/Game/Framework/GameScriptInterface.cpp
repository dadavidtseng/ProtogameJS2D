//----------------------------------------------------------------------------------------------------
// GameScriptInterface.cpp
// Optimized V8 JavaScript integration with exception-free type conversion
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#include "Game/Framework/GameScriptInterface.hpp"

#include <chrono>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <stdexcept>

#include "Engine/Core/Clock.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/LogSubsystem.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Scripting/V8Subsystem.hpp"
#include "Game/Game.hpp"
#include "Game/Player.hpp"
#include "Game/Framework/GameCommon.hpp"

//----------------------------------------------------------------------------------------------------
GameScriptInterface::GameScriptInterface(Game* game)
    : m_game(game),
      m_fileWatcher(std::make_unique<FileWatcher>()),
      m_scriptReloader(std::make_unique<ScriptReloader>())
{
    if (!g_game)
    {
        ERROR_AND_DIE("GameScriptInterface: Game pointer cannot be null")
    }
}

//----------------------------------------------------------------------------------------------------
GameScriptInterface::~GameScriptInterface()
{
    ShutdownHotReload();
}

//----------------------------------------------------------------------------------------------------
std::string GameScriptInterface::GetScriptObjectName() const
{
    return "game";
}

//----------------------------------------------------------------------------------------------------
std::vector<ScriptMethodInfo> GameScriptInterface::GetAvailableMethods() const
{
    return {
        ScriptMethodInfo("createCube",
                         "在指定位置創建一個立方體",
                         {"float", "float", "float"},
                         "string"),

        ScriptMethodInfo("moveProp",
                         "移動指定索引的道具到新位置",
                         {"int", "float", "float", "float"},
                         "string"),

        ScriptMethodInfo("getPlayerPosition",
                         "取得玩家目前位置",
                         {},
                         "object"),

        ScriptMethodInfo("movePlayerCamera",
                         "移動玩家相機（用於晃動效果）",
                         {"float", "float", "float"},
                         "string"),

        ScriptMethodInfo("update",
                         "JavaScript GameLoop Update",
                         {"float", "float"},
                         "void"),
        ScriptMethodInfo("render",
                         "JavaScript GameLoop Render",
                         {},
                         "void"),

        ScriptMethodInfo("executeCommand",
                         "執行 JavaScript 指令",
                         {"string"},
                         "string"),

        ScriptMethodInfo("executeFile",
                         "執行 JavaScript 檔案",
                         {"string"},
                         "string"),

        ScriptMethodInfo("isAttractMode",
                         "檢查遊戲是否處於吸引模式",
                         {},
                         "bool"),

        ScriptMethodInfo("getGameState",
                         "取得目前遊戲狀態",
                         {},
                         "string"),

        ScriptMethodInfo("getFileTimestamp",
                         "取得檔案的最後修改時間戳記",
                         {"string"},
                         "number")
    };
}

//----------------------------------------------------------------------------------------------------
std::vector<std::string> GameScriptInterface::GetAvailableProperties() const
{
    return {
        "attractMode",
        "gameState"
    };
}

//----------------------------------------------------------------------------------------------------
ScriptMethodResult GameScriptInterface::CallMethod(std::string const&           methodName,
                                                   std::vector<std::any> const& args)
{
    try
    {
        if (methodName == "createCube")
        {
            return ExecuteCreateCube(args);
        }
        else if (methodName == "moveProp")
        {
            return ExecuteMoveProp(args);
        }
        else if (methodName == "getPlayerPosition")
        {
            return ExecuteGetPlayerPosition(args);
        }
        else if (methodName == "movePlayerCamera")
        {
            return ExecuteMovePlayerCamera(args);
        }
        else if (methodName == "update")
        {
            return ExecuteUpdate(args);
        }
        else if (methodName == "render")
        {
            return ExecuteRender(args);
        }
        else if (methodName == "executeCommand")
        {
            return ExecuteJavaScriptCommand(args);
        }
        else if (methodName == "executeFile")
        {
            return ExecuteJavaScriptFile(args);
        }
        else if (methodName == "isAttractMode")
        {
            return ExecuteIsAttractMode(args);
        }
        else if (methodName == "getGameState")
        {
            return ExecuteGetGameState(args);
        }
        else if (methodName == "getFileTimestamp")
        {
            return ExecuteGetFileTimestamp(args);
        }
        else if (methodName == "enableHotReload")
        {
            return ExecuteEnableHotReload(args);
        }
        else if (methodName == "disableHotReload")
        {
            return ExecuteDisableHotReload(args);
        }
        else if (methodName == "isHotReloadEnabled")
        {
            return ExecuteIsHotReloadEnabled(args);
        }
        else if (methodName == "addWatchedFile")
        {
            return ExecuteAddWatchedFile(args);
        }
        else if (methodName == "removeWatchedFile")
        {
            return ExecuteRemoveWatchedFile(args);
        }
        else if (methodName == "getWatchedFiles")
        {
            return ExecuteGetWatchedFiles(args);
        }
        else if (methodName == "reloadScript")
        {
            return ExecuteReloadScript(args);
        }

        return ScriptMethodResult::Error("未知的方法: " + methodName);
    }
    catch (std::exception const& e)
    {
        return ScriptMethodResult::Error("方法執行時發生例外: " + std::string(e.what()));
    }
}

//----------------------------------------------------------------------------------------------------
std::any GameScriptInterface::GetProperty(const std::string& propertyName) const
{
    if (propertyName == "attractMode")
    {
        return m_game->IsAttractMode();
    }
    else if (propertyName == "gameState")
    {
        return m_game->IsAttractMode() ? "attract" : "game";
    }

    return std::any{};
}

//----------------------------------------------------------------------------------------------------
bool GameScriptInterface::SetProperty(const std::string& propertyName, const std::any& value)
{
    // 目前 Game 物件沒有可設定的屬性
    // 如果需要，可以在這裡添加
    UNUSED(propertyName);
    UNUSED(value);
    return false;
}

//----------------------------------------------------------------------------------------------------
ScriptMethodResult GameScriptInterface::ExecuteCreateCube(std::vector<std::any> const& args)
{
    ScriptMethodResult result = ValidateArgCount(args, 3, "createCube");

    if (!result.success) return result;

    try
    {
        Vec3 const position = ExtractVec3(args, 0);

        m_game->CreateCube(position);

        return ScriptMethodResult::Success(std::string("立方體創建成功，位置: (" +
            std::to_string(position.x) + ", " +
            std::to_string(position.y) + ", " +
            std::to_string(position.z) + ")"));
    }
    catch (const std::exception& e)
    {
        return ScriptMethodResult::Error("創建立方體失敗: " + std::string(e.what()));
    }
}

//----------------------------------------------------------------------------------------------------
ScriptMethodResult GameScriptInterface::ExecuteMoveProp(const std::vector<std::any>& args)
{
    auto result = ValidateArgCount(args, 4, "moveProp");
    if (!result.success) return result;

    try
    {
        int  propIndex   = ExtractInt(args[0]);
        Vec3 newPosition = ExtractVec3(args, 1);
        m_game->MoveProp(propIndex, newPosition);
        return ScriptMethodResult::Success(std::string("道具 " + std::to_string(propIndex) +
            " 移動成功，新位置: (" +
            std::to_string(newPosition.x) + ", " +
            std::to_string(newPosition.y) + ", " +
            std::to_string(newPosition.z) + ")"));
    }
    catch (const std::exception& e)
    {
        return ScriptMethodResult::Error("移動道具失敗: " + std::string(e.what()));
    }
}

//----------------------------------------------------------------------------------------------------
ScriptMethodResult GameScriptInterface::ExecuteGetPlayerPosition(const std::vector<std::any>& args)
{
    auto result = ValidateArgCount(args, 0, "getPlayerPosition");
    if (!result.success) return result;

    try
    {
        Player* player = m_game->GetPlayer();
        if (!player)
        {
            return ScriptMethodResult::Error("玩家物件不存在");
        }

        Vec3 position = player->m_position;


        // 回傳一個可以被 JavaScript 使用的物件
        std::string positionStr = "{ x: " + std::to_string(position.x) +
        ", y: " + std::to_string(position.y) +
        ", z: " + std::to_string(position.z) + " }";

        return ScriptMethodResult::Success(positionStr);
    }
    catch (const std::exception& e)
    {
        return ScriptMethodResult::Error("取得玩家位置失敗: " + std::string(e.what()));
    }
}

//----------------------------------------------------------------------------------------------------
ScriptMethodResult GameScriptInterface::ExecuteMovePlayerCamera(const std::vector<std::any>& args)
{
    auto result = ValidateArgCount(args, 3, "movePlayerCamera");
    if (!result.success) return result;

    try
    {
        Vec3 offset = ExtractVec3(args, 0);
        m_game->MovePlayerCamera(offset);
        return ScriptMethodResult::Success(std::string("相機位置已移動: (" +
            std::to_string(offset.x) + ", " +
            std::to_string(offset.y) + ", " +
            std::to_string(offset.z) + ")"));
    }
    catch (const std::exception& e)
    {
        return ScriptMethodResult::Error("移動玩家相機失敗: " + std::string(e.what()));
    }
}

ScriptMethodResult GameScriptInterface::ExecuteRender(std::vector<std::any> const& args)
{
    auto result = ValidateArgCount(args, 0, "Render");
    if (!result.success) return result;

    try
    {
        m_game->Render();
        return ScriptMethodResult::Success(Stringf("Render Success"));
    }
    catch (const std::exception& e)
    {
        return ScriptMethodResult::Error("Render failed: " + std::string(e.what()));
    }
}

ScriptMethodResult GameScriptInterface::ExecuteUpdate(std::vector<std::any> const& args)
{
    auto result = ValidateArgCount(args, 2, "Update");
    if (!result.success) return result;

    try
    {
        float gameDeltaSeconds   = ExtractFloat(args[0]);
        float systemDeltaSeconds = ExtractFloat(args[1]);

        m_game->Update(gameDeltaSeconds, systemDeltaSeconds);
        return ScriptMethodResult::Success(Stringf("Update Success"));
    }
    catch (const std::exception& e)
    {
        return ScriptMethodResult::Error("Update failed: " + std::string(e.what()));
    }
}

//----------------------------------------------------------------------------------------------------
ScriptMethodResult GameScriptInterface::ExecuteJavaScriptCommand(const std::vector<std::any>& args)
{
    auto result = ValidateArgCount(args, 1, "executeCommand");
    if (!result.success) return result;

    try
    {
        std::string command = ExtractString(args[0]);
        m_game->ExecuteJavaScriptCommand(command);
        return ScriptMethodResult::Success(std::string("指令執行: " + command));
    }
    catch (const std::exception& e)
    {
        return ScriptMethodResult::Error("執行 JavaScript 指令失敗: " + std::string(e.what()));
    }
}

//----------------------------------------------------------------------------------------------------
ScriptMethodResult GameScriptInterface::ExecuteJavaScriptFile(const std::vector<std::any>& args)
{
    auto result = ValidateArgCount(args, 1, "executeFile");
    if (!result.success) return result;

    try
    {
        std::string filename = ExtractString(args[0]);
        m_game->ExecuteJavaScriptFile(filename);
        return ScriptMethodResult::Success(std::string("檔案執行: " + filename));
    }
    catch (const std::exception& e)
    {
        return ScriptMethodResult::Error("執行 JavaScript 檔案失敗: " + std::string(e.what()));
    }
}

//----------------------------------------------------------------------------------------------------
ScriptMethodResult GameScriptInterface::ExecuteIsAttractMode(const std::vector<std::any>& args)
{
    auto result = ValidateArgCount(args, 0, "isAttractMode");
    if (!result.success) return result;

    try
    {
        bool isAttract = m_game->IsAttractMode();
        return ScriptMethodResult::Success(isAttract);
    }
    catch (const std::exception& e)
    {
        return ScriptMethodResult::Error("檢查吸引模式失敗: " + std::string(e.what()));
    }
}

//----------------------------------------------------------------------------------------------------
ScriptMethodResult GameScriptInterface::ExecuteGetGameState(const std::vector<std::any>& args)
{
    auto result = ValidateArgCount(args, 0, "getGameState");
    if (!result.success) return result;

    try
    {
        std::string state = m_game->IsAttractMode() ? "attract" : "game";
        return ScriptMethodResult::Success(state);
    }
    catch (const std::exception& e)
    {
        return ScriptMethodResult::Error("取得遊戲狀態失敗: " + std::string(e.what()));
    }
}

//----------------------------------------------------------------------------------------------------
// 輔助方法實作
//----------------------------------------------------------------------------------------------------

template <typename T>
T GameScriptInterface::ExtractArg(const std::any& arg, const std::string& expectedType) const
{
    // Use type-safe extraction to avoid std::bad_any_cast exceptions
    if (HasType<T>(arg))
    {
        return SafeCast<T>(arg);
    }
    else
    {
        std::string typeInfo = expectedType.empty() ? typeid(T).name() : expectedType;
        throw std::invalid_argument("參數類型錯誤，期望: " + typeInfo);
    }
}

//----------------------------------------------------------------------------------------------------
Vec3 GameScriptInterface::ExtractVec3(const std::vector<std::any>& args, size_t startIndex) const
{
    if (startIndex + 2 >= args.size())
    {
        throw std::invalid_argument("Vec3 需要 3 個參數 (x, y, z)");
    }

    float x = ExtractFloat(args[startIndex]);
    float y = ExtractFloat(args[startIndex + 1]);
    float z = ExtractFloat(args[startIndex + 2]);

    return Vec3(x, y, z);
}

//----------------------------------------------------------------------------------------------------
float GameScriptInterface::ExtractFloat(const std::any& arg) const
{
    try
    {
        // V8 always passes JavaScript numbers as double, so check double first
        if (HasType<double>(arg))
        {
            return static_cast<float>(SafeCast<double>(arg));
        }
        else if (HasType<float>(arg))
        {
            return SafeCast<float>(arg);
        }
        else if (HasType<int>(arg))
        {
            return static_cast<float>(SafeCast<int>(arg));
        }
        else if (HasType<long>(arg))
        {
            return static_cast<float>(SafeCast<long>(arg));
        }
        else if (HasType<unsigned int>(arg))
        {
            return static_cast<float>(SafeCast<unsigned int>(arg));
        }
        else
        {
            // Try direct cast as last resort
            try
            {
                return std::any_cast<float>(arg);
            }
            catch (const std::bad_any_cast&)
            {
                try
                {
                    return static_cast<float>(std::any_cast<double>(arg));
                }
                catch (const std::bad_any_cast&)
                {
                    throw std::invalid_argument("無法轉換為 float 類型");
                }
            }
        }
    }
    catch (const std::bad_any_cast& e)
    {
        throw std::invalid_argument("ExtractFloat: bad_any_cast - " + std::string(e.what()));
    }
}

//----------------------------------------------------------------------------------------------------
int GameScriptInterface::ExtractInt(const std::any& arg) const
{
    try
    {
        // V8 always passes JavaScript numbers as double, so check double first
        if (HasType<double>(arg))
        {
            return static_cast<int>(SafeCast<double>(arg));
        }
        else if (HasType<int>(arg))
        {
            return SafeCast<int>(arg);
        }
        else if (HasType<float>(arg))
        {
            return static_cast<int>(SafeCast<float>(arg));
        }
        else if (HasType<long>(arg))
        {
            return static_cast<int>(SafeCast<long>(arg));
        }
        else if (HasType<unsigned int>(arg))
        {
            return static_cast<int>(SafeCast<unsigned int>(arg));
        }
        else
        {
            // Try direct cast as last resort
            try
            {
                return std::any_cast<int>(arg);
            }
            catch (const std::bad_any_cast&)
            {
                try
                {
                    return static_cast<int>(std::any_cast<double>(arg));
                }
                catch (const std::bad_any_cast&)
                {
                    throw std::invalid_argument("無法轉換為 int 類型");
                }
            }
        }
    }
    catch (const std::bad_any_cast& e)
    {
        throw std::invalid_argument("ExtractInt: bad_any_cast - " + std::string(e.what()));
    }
}

//----------------------------------------------------------------------------------------------------
std::string GameScriptInterface::ExtractString(const std::any& arg) const
{
    try
    {
        // V8 passes JavaScript strings as std::string
        if (HasType<std::string>(arg))
        {
            return SafeCast<std::string>(arg);
        }
        else if (HasType<const char*>(arg))
        {
            const char* cstr = SafeCast<const char*>(arg);
            return std::string(cstr);
        }
        else if (HasType<char*>(arg))
        {
            char* cstr = SafeCast<char*>(arg);
            return std::string(cstr);
        }
        else
        {
            // Try direct cast as last resort
            try
            {
                return std::any_cast<std::string>(arg);
            }
            catch (const std::bad_any_cast&)
            {
                throw std::invalid_argument("無法轉換為 string 類型");
            }
        }
    }
    catch (const std::bad_any_cast& e)
    {
        throw std::invalid_argument("ExtractString: bad_any_cast - " + std::string(e.what()));
    }
}

//----------------------------------------------------------------------------------------------------
bool GameScriptInterface::ExtractBool(const std::any& arg) const
{
    try
    {
        // V8 passes JavaScript booleans as bool type
        if (HasType<bool>(arg))
        {
            return SafeCast<bool>(arg);
        }
        else if (HasType<int>(arg))
        {
            // Convert numeric values to boolean (0 = false, non-zero = true)
            int val = SafeCast<int>(arg);
            return val != 0;
        }
        else if (HasType<double>(arg))
        {
            // Convert V8 numbers to boolean (0.0 = false, non-zero = true)
            double val = SafeCast<double>(arg);
            return val != 0.0;
        }
        else if (HasType<float>(arg))
        {
            float val = SafeCast<float>(arg);
            return val != 0.0f;
        }
        else
        {
            // Try direct cast as last resort
            try
            {
                return std::any_cast<bool>(arg);
            }
            catch (const std::bad_any_cast&)
            {
                throw std::invalid_argument("無法轉換為 bool 類型");
            }
        }
    }
    catch (const std::bad_any_cast& e)
    {
        throw std::invalid_argument("ExtractBool: bad_any_cast - " + std::string(e.what()));
    }
}

//----------------------------------------------------------------------------------------------------
ScriptMethodResult GameScriptInterface::ValidateArgCount(const std::vector<std::any>& args,
                                                         size_t                       expectedCount,
                                                         const std::string&           methodName) const
{
    if (args.size() != expectedCount)
    {
        std::ostringstream oss;
        oss << methodName << " needs " << expectedCount << " variables, but receives " << args.size();
        return ScriptMethodResult::Error(oss.str());
    }
    return ScriptMethodResult::Success();
}

//----------------------------------------------------------------------------------------------------
ScriptMethodResult GameScriptInterface::ValidateArgCountRange(const std::vector<std::any>& args,
                                                              size_t                       minCount,
                                                              size_t                       maxCount,
                                                              const std::string&           methodName) const
{
    if (args.size() < minCount || args.size() > maxCount)
    {
        std::ostringstream oss;
        oss << methodName << " needs " << minCount << "-" << maxCount << " variables, but receives " << args.size();
        return ScriptMethodResult::Error(oss.str());
    }
    return ScriptMethodResult::Success();
}

//----------------------------------------------------------------------------------------------------
ScriptMethodResult GameScriptInterface::ExecuteGetFileTimestamp(const std::vector<std::any>& args)
{
    auto result = ValidateArgCount(args, 1, "getFileTimestamp");
    if (!result.success) return result;

    try
    {
        std::string filePath = ExtractString(args[0]);

        // The filePath comes from HotReloader as 'Data/Scripts/filename.js'
        // Build absolute path from the known project structure
        std::string projectRoot = "C:/p4/Personal/SD/ProtogameJS2D/";
        std::string fullPath    = projectRoot + "Run/" + filePath;

        // Debug: Log the paths being used
        DebuggerPrintf("getFileTimestamp: Input path = %s\n", filePath.c_str());
        DebuggerPrintf("getFileTimestamp: Full path = %s\n", fullPath.c_str());

        // Get file timestamp using standard library
        if (std::filesystem::exists(fullPath))
        {
            auto ftime = std::filesystem::last_write_time(fullPath);
            auto sctp  = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                ftime - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now());
            auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(sctp.time_since_epoch()).count();

            return ScriptMethodResult::Success(static_cast<double>(timestamp));
        }
        else
        {
            return ScriptMethodResult::Error("檔案不存在: " + filePath);
        }
    }
    catch (const std::exception& e)
    {
        return ScriptMethodResult::Error("取得檔案時間戳記失敗: " + std::string(e.what()));
    }
}

//----------------------------------------------------------------------------------------------------
// Hot-reload system initialization
//----------------------------------------------------------------------------------------------------
bool GameScriptInterface::InitializeHotReload(V8Subsystem*       v8System,
                                              String const& projectRoot)
{
    try
    {
        DAEMON_LOG(LogScript, eLogVerbosity::Log, StringFormat("GameScriptInterface: Initializing hot-reload system..."));

        // Store project root for path construction
        m_projectRoot = projectRoot;

        // Initialize FileWatcher
        if (!m_fileWatcher->Initialize(projectRoot))
        {
            DAEMON_LOG(LogScript, eLogVerbosity::Error, StringFormat("GameScriptInterface: Failed to initialize FileWatcher"));
            return false;
        }

        // Initialize ScriptReloader
        if (!m_scriptReloader->Initialize(v8System))
        {
            DAEMON_LOG(LogScript, eLogVerbosity::Error, StringFormat("GameScriptInterface: Failed to initialize ScriptReloader"));
            return false;
        }

        // Set up callbacks
        m_fileWatcher->SetChangeCallback([this](const std::string& filePath) {
            OnFileChanged(filePath);
        });

        m_scriptReloader->SetReloadCompleteCallback([this](bool success, const std::string& error) {
            OnReloadComplete(success, error);
        });

        // Add default watched files
        m_fileWatcher->AddWatchedFile("Data/Scripts/JSEngine.js");
        m_fileWatcher->AddWatchedFile("Data/Scripts/JSGame.js");
        m_fileWatcher->AddWatchedFile("Data/Scripts/InputSystem.js");

        // Start watching
        m_fileWatcher->StartWatching();
        m_hotReloadEnabled = true;

        DAEMON_LOG(LogScript, eLogVerbosity::Log, StringFormat("GameScriptInterface: Hot-reload system initialized successfully"));
        return true;
    }
    catch (std::exception const& e)
    {
        DAEMON_LOG(LogScript, eLogVerbosity::Error, StringFormat("GameScriptInterface: Hot-reload initialization failed: {}", e.what()));
        return false;
    }
}

void GameScriptInterface::ShutdownHotReload()
{
    try
    {
        if (m_fileWatcher != nullptr)
        {
            m_fileWatcher->Shutdown();
        }
        if (m_scriptReloader)
        {
            m_scriptReloader->Shutdown();
        }
        m_hotReloadEnabled = false;
        DAEMON_LOG(LogScript, eLogVerbosity::Log, StringFormat("GameScriptInterface: Hot-reload system shutdown completed"));
    }
    catch (const std::exception& e)
    {
        DAEMON_LOG(LogScript, eLogVerbosity::Error, StringFormat("GameScriptInterface: Hot-reload shutdown error: {}", e.what()));
    }
}

void GameScriptInterface::OnFileChanged(const std::string& filePath)
{
    try
    {
        DAEMON_LOG(LogScript, eLogVerbosity::Log, StringFormat("GameScriptInterface: File changed (queuing for main thread): {}", filePath));

        // Queue the file change for main thread processing (thread-safe)
        if (m_hotReloadEnabled)
        {
            std::lock_guard<std::mutex> lock(m_fileChangeQueueMutex);
            m_pendingFileChanges.push(filePath);
        }
    }
    catch (const std::exception& e)
    {
        DAEMON_LOG(LogScript, eLogVerbosity::Error, StringFormat("GameScriptInterface: File change handling error: {}", e.what()));
    }
}

void GameScriptInterface::OnReloadComplete(bool success, const std::string& error)
{
    if (success)
    {
        DAEMON_LOG(LogScript, eLogVerbosity::Log, StringFormat("GameScriptInterface: Script reload completed successfully"));
    }
    else
    {
        DAEMON_LOG(LogScript, eLogVerbosity::Error, StringFormat("GameScriptInterface: Script reload failed: {}", error));
    }
}

void GameScriptInterface::ProcessPendingHotReloadEvents()
{
    try
    {
        // Process all pending file changes on the main thread (V8-safe)
        std::queue<std::string> filesToProcess;

        // Get all pending changes under lock
        {
            std::lock_guard<std::mutex> lock(m_fileChangeQueueMutex);
            filesToProcess.swap(m_pendingFileChanges); // Efficiently move all items
        }

        // Process all file changes outside the lock
        while (!filesToProcess.empty())
        {
            const std::string& filePath = filesToProcess.front();

            DAEMON_LOG(LogScript, eLogVerbosity::Log, StringFormat("GameScriptInterface: Processing file change on main thread: {}", filePath));

            // Convert relative path to absolute path for ScriptReloader
            std::string absolutePath = GetAbsoluteScriptPath(filePath);

            // Now safe to call V8 from main thread
            if (m_scriptReloader && m_hotReloadEnabled)
            {
                m_scriptReloader->ReloadScript(absolutePath);
            }

            filesToProcess.pop();
        }
    }
    catch (const std::exception& e)
    {
        DAEMON_LOG(LogScript, eLogVerbosity::Error, StringFormat("GameScriptInterface: Error processing pending hot-reload events: {}", e.what()));
    }
}

std::string GameScriptInterface::GetAbsoluteScriptPath(const std::string& relativePath) const
{
    // Same logic as FileWatcher::GetFullPath()
    std::filesystem::path fullPath = std::filesystem::path(m_projectRoot) / "Run" / relativePath;
    return fullPath.string();
}

//----------------------------------------------------------------------------------------------------
// Hot-reload method implementations
//----------------------------------------------------------------------------------------------------
ScriptMethodResult GameScriptInterface::ExecuteEnableHotReload(const std::vector<std::any>& args)
{
    auto result = ValidateArgCount(args, 0, "enableHotReload");
    if (!result.success) return result;

    try
    {
        if (!m_fileWatcher || !m_scriptReloader)
        {
            return ScriptMethodResult::Error("熱重載系統未初始化");
        }

        if (!m_hotReloadEnabled)
        {
            m_fileWatcher->StartWatching();
            m_hotReloadEnabled = true;
        }

        return ScriptMethodResult::Success(true);
    }
    catch (const std::exception& e)
    {
        return ScriptMethodResult::Error("啟用熱重載失敗: " + std::string(e.what()));
    }
}

ScriptMethodResult GameScriptInterface::ExecuteDisableHotReload(const std::vector<std::any>& args)
{
    auto result = ValidateArgCount(args, 0, "disableHotReload");
    if (!result.success) return result;

    try
    {
        if (m_fileWatcher && m_hotReloadEnabled)
        {
            m_fileWatcher->StopWatching();
            m_hotReloadEnabled = false;
        }

        return ScriptMethodResult::Success(true);
    }
    catch (const std::exception& e)
    {
        return ScriptMethodResult::Error("停用熱重載失敗: " + std::string(e.what()));
    }
}

ScriptMethodResult GameScriptInterface::ExecuteIsHotReloadEnabled(const std::vector<std::any>& args)
{
    auto result = ValidateArgCount(args, 0, "isHotReloadEnabled");
    if (!result.success) return result;

    return ScriptMethodResult::Success(m_hotReloadEnabled);
}

ScriptMethodResult GameScriptInterface::ExecuteAddWatchedFile(const std::vector<std::any>& args)
{
    auto result = ValidateArgCount(args, 1, "addWatchedFile");
    if (!result.success) return result;

    try
    {
        std::string filePath = ExtractString(args[0]);

        if (!m_fileWatcher)
        {
            return ScriptMethodResult::Error("FileWatcher 未初始化");
        }

        m_fileWatcher->AddWatchedFile(filePath);
        return ScriptMethodResult::Success(true);
    }
    catch (const std::exception& e)
    {
        return ScriptMethodResult::Error("新增監控檔案失敗: " + std::string(e.what()));
    }
}

ScriptMethodResult GameScriptInterface::ExecuteRemoveWatchedFile(const std::vector<std::any>& args)
{
    auto result = ValidateArgCount(args, 1, "removeWatchedFile");
    if (!result.success) return result;

    try
    {
        std::string filePath = ExtractString(args[0]);

        if (!m_fileWatcher)
        {
            return ScriptMethodResult::Error("FileWatcher 未初始化");
        }

        m_fileWatcher->RemoveWatchedFile(filePath);
        return ScriptMethodResult::Success(true);
    }
    catch (const std::exception& e)
    {
        return ScriptMethodResult::Error("移除監控檔案失敗: " + std::string(e.what()));
    }
}

ScriptMethodResult GameScriptInterface::ExecuteGetWatchedFiles(const std::vector<std::any>& args)
{
    auto result = ValidateArgCount(args, 0, "getWatchedFiles");
    if (!result.success) return result;

    try
    {
        if (!m_fileWatcher)
        {
            return ScriptMethodResult::Error("FileWatcher 未初始化");
        }

        auto watchedFiles = m_fileWatcher->GetWatchedFiles();

        // Build comma-separated string of watched files
        std::string fileList;
        for (size_t i = 0; i < watchedFiles.size(); ++i)
        {
            if (i > 0) fileList += ", ";
            fileList += watchedFiles[i];
        }

        return ScriptMethodResult::Success(fileList);
    }
    catch (const std::exception& e)
    {
        return ScriptMethodResult::Error("取得監控檔案清單失敗: " + std::string(e.what()));
    }
}

ScriptMethodResult GameScriptInterface::ExecuteReloadScript(const std::vector<std::any>& args)
{
    auto result = ValidateArgCount(args, 1, "reloadScript");
    if (!result.success) return result;

    try
    {
        std::string scriptPath = ExtractString(args[0]);

        if (!m_scriptReloader)
        {
            return ScriptMethodResult::Error("ScriptReloader 未初始化");
        }

        bool success = m_scriptReloader->ReloadScript(scriptPath);
        return ScriptMethodResult::Success(success);
    }
    catch (const std::exception& e)
    {
        return ScriptMethodResult::Error("重載腳本失敗: " + std::string(e.what()));
    }
}
