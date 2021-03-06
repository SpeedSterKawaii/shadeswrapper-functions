// all these are from shades wrapper source.
// credit shade or else..
#include "Environment.h"
extern "C"
{
#include "Bit/bit.c"
#include "Bit/bit32.c"
}
#include "XHosted.h"
#include "CheckBypasses.h"
#include "RetCheck.hpp"
#include "Tools.h"
#include "TaskScheduler.h"
#include "Drawing/Drawing.h"


namespace Env
{
	lua_State* GL;
	uintptr_t RobloxState;
	uintptr_t DataModel;

	namespace Primary
	{

		const std::vector<const char*> Globals = {
"printidentity","game","Game","workspace","Workspace","print",
"Axes", "BrickColor", "CFrame", "Color3", "ColorSequence","ColorSequenceKeypoint",
"NumberRange","NumberSequence","NumberSequenceKeypoint","PhysicalProperties","Ray",
"Rect","Region3","Region3int16","TweenInfo","UDim","UDim2","Vector2", "shared",
"Vector2int16","Vector3","Vector3int16", "Enum", "Faces", "tick", "utf8",
"Instance","warn","typeof", "tick", "wait", "Wait", "os",
 "DockWidgetPluginGuiInfo", "DateTime", "Stats", "UserSettings", "PluginManager",
"Version", "CellId", "version", "stats", "settings", "RaycastParams", "elapsedTime",
"PathWaypoint", "Random", "PluginDrag", "ElapsedTime", "Delay", "delay", "spawn", "Spawn"
		};

		const std::vector<const char*> Sub =
		{
			"table", "move",
			"table", "create",
			"table", "unpack",
			"table", "pack",
			"table", "find",
			"string", "split",
			"math", "noise",
			"math", "clamp",
			"math", "sign",
			"math", "round",
			"string", "pack",
			"string", "packsize",
			"string", "unpack"
		};

		bool Ready = false;

		bool CreateEnvironment()
		{
#if FunctionNames
			printf("%s Called\n", __FUNCTION__);
#endif
			//Open
			GL = lua_open();
			if (GL)
			{
				Drawing::SetUp(GL);
				luaL_openlibs(GL);
				luaopen_bit(GL);
				luaopen_bit32(GL);
				Tools::Window::InitializeWindow();
				if (Run(unprotect))
				{
#if CorePrints
					printf("XHosted Ran\n");
#endif
#if MemCheckBypass
					Tools::Memory::Strings::FindStrings();//Need to keep here
#if CorePrints
					auto start1 = std::chrono::high_resolution_clock::now();
#endif
					TaskScheduler::UpdateTaskScheduler2();
#if CorePrints
					auto elapsed1 = std::chrono::high_resolution_clock::now() - start1;
					long long microseconds1 = std::chrono::duration_cast<std::chrono::microseconds>(elapsed1).count();
#endif
					if (TaskScheduler::GetJobs != 0)
					{
#if CorePrints
						printf("Updated TaskScheduler: 0x%X in %dmicro\n", noaslr(TaskScheduler::GetJobs), microseconds1);
#endif
						TaskScheduler::LoadJobs();
						auto MemCheckJob = TaskScheduler::GetJobByName("US14116");
						TaskScheduler::OriginalMemcheckJob = reinterpret_cast<decltype(TaskScheduler::OriginalMemcheckJob)>(MemCheckJob->VFTable->Main);
						TaskScheduler::ReplaceMainFunc(MemCheckJob, reinterpret_cast<uintptr_t>(TaskScheduler::MemcheckHook));
#if CorePrints
						auto start = std::chrono::high_resolution_clock::now();
#endif
						Bypasses::CallCheck::CallCheckAddr = Bypasses::CallCheck::UpdateCallCheckAddress();
#if CorePrints
						auto elapsed = std::chrono::high_resolution_clock::now() - start;
						long long microseconds = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
#endif
						if (Bypasses::CallCheck::CallCheckAddr != 0)
						{
#if CorePrints
							printf("Updated CallCheck: 0x%X in %dmicro\n", noaslr(Bypasses::CallCheck::CallCheckAddr), microseconds);
#endif
							TaskScheduler::WriteMemory(Bypasses::CallCheck::CallCheckAddr, (uint8_t*)"\x90\x90", 2);//CallCheck Bypass
						}
						else 
						{
							printf("Failed to update CallCheckAddr\n");
							system("pause");
							return false;
						}
					}
					else
					{
						printf("Failed to update Task Scheduler!\n");
						system("pause");
						return false;
					}
#elif FullyAutoUpdatingCallCheck
					Bypasses::CallCheck::VehHandlerpush();
#else
					//THE BELOW CODE CAN NOT BE USED TO AUTO UPDATE CALLCHECK. YOU NEED TO USE IT COPY THE ADDRESS. GO TO CALLCHECKBYPASSES.H AND REPLACE THE ADDRESS IN THE HOOKLOCATION VARIABLE
					//Bypasses::CallCheck::HookLocation = Bypasses::CallCheck::UpdateCallCheckAddress() + 4;
					//printf("0x%X\n", noaslr(Bypasses::CallCheck::HookLocation));
					//system("pause");
					if (!Bypasses::CallCheck::StartUpCallCheck()) { printf("Failed to bypass CallCheck\n"); return false; }

#endif
					Bypasses::TrustCheck::UpdateTrustCheck();
					Bypasses::TrustCheck::SetCheckStore();
#if CorePrints
					printf("Checks Bypassed\n");
#endif

#if FPSUnlocker
					TaskScheduler::UnlockFPS();
#endif
#if CorePrints
					printf("FPS Unlocked\n");
#endif

					//Register Environment Functions
					lua_register(GL, "RobloxAttached", RobloxAttached);
					lua_register(GL, "setndm", CustomFunctions::SetNDM);
					lua_register(GL, "settrustcheck", CustomFunctions::SetTrustCheck);

					//Yielding Changes
					lua_pushcclosure(GL, YieldingHandling::PCall, 0);
					lua_pushvalue(GL, -1);
					lua_setglobal(GL, "pcall");
					lua_setglobal(GL, "ypcall");

					lua_getglobal(GL, "coroutine");
					lua_pushcclosure(GL, YieldingHandling::Coroutine::luaB_cowrap, 0);
					lua_setfield(GL, -2, "wrap");
					lua_pushcclosure(GL, YieldingHandling::Coroutine::luaB_coresume, 0);
					lua_setfield(GL, -2, "resume");
					lua_pushcclosure(GL, YieldingHandling::Coroutine::luaB_cocreate, 0);
					lua_setfield(GL, -2, "create");
					lua_pop(GL, 1);

					lua_newtable(GL);
					lua_setglobal(GL, "_G");
					lua_newtable(GL);
					lua_setglobal(GL, "shared");

					//Custom Functions
					lua_register(GL, "getgenv", CustomFunctions::GetGenv);
					lua_register(GL, "getrenv", CustomFunctions::GetRenv);
					lua_register(GL, "getreg", CustomFunctions::GetReg);
					lua_register(GL, "readfile", CustomFunctions::ReadFile);
					lua_register(GL, "writefile", CustomFunctions::WriteFile);
					lua_register(GL, "setreadonly", CustomFunctions::setreadonly);
					lua_register(GL, "isreadonly", CustomFunctions::isreadonly);
					lua_register(GL, "iscclosure", CustomFunctions::IsCClosure);
					lua_register(GL, "getreglist", CustomFunctions::GetRegList);
					lua_register(GL, "setclipboard", CustomFunctions::SetClipboard);
					lua_register(GL, "getclipboard", CustomFunctions::GetClipboard);
					lua_register(GL, "fireclickdetector", CustomFunctions::FireClickDetector);
					lua_register(GL, "movemouserel", CustomFunctions::MoveMouseRel);
					lua_register(GL, "mouse1click", CustomFunctions::Mouse1Click);
					lua_register(GL, "mouse1up", CustomFunctions::Mouse1Up);
					lua_register(GL, "mouse1down", CustomFunctions::Mouse1Down);
					lua_register(GL, "mouse2click", CustomFunctions::Mouse2Click);
					lua_register(GL, "mouse2up", CustomFunctions::Mouse2Up);
					lua_register(GL, "mouse2down", CustomFunctions::Mouse2Down);
					lua_register(GL, "getgc", CustomFunctions::GetGC);
					lua_register(GL, "isrobloxobject", CustomFunctions::IsRobloxObject);
					lua_register(GL, "cprint", CustomFunctions::CPrint);
					lua_register(GL, "getrawmetatable", CustomFunctions::GetRawMetaTable);
					lua_register(GL, "unlockmodule", CustomFunctions::UnLockModule);
					lua_register(GL, "checkcaller", CustomFunctions::CheckCaller);
					//lua_register(GL, "getconnections", CustomFunctions::GetConnections); This does not work
					lua_register(GL, "getnamecallmethod", CustomFunctions::GetNameCallMethod);
					lua_register(GL, "setnamecallmethod", CustomFunctions::SetNameCallMethod);
					lua_register(GL, "hookfunction", CustomFunctions::HookFunction);
					lua_register(GL, "setfflag", CustomFunctions::SetFFlag);
					CustomFunctions::DebugLib::luaopen_rdebug(GL);

#if CorePrints
					printf("Environment SetUp\n");
#endif
					if (ROpenState())
						return true;
					else
						return false;
				}
				return true;
			}
			return false;
		}

		bool ROpenState()
		{
#if FunctionNames
			printf("%s Called\n", __FUNCTION__);
#endif
			DataModel = GetDataModel();
#if CorePrints
			printf("DataModel: 0x%X\n", DataModel);
#endif
			while (*reinterpret_cast<uint8_t*>((DataModel - x_InstanceClass) + x_IsLoaded) != true)//IsLoaded
				Sleep(50);
#if CorePrints
			printf("Game Has Loaded\n");
#endif
			RobloxState = GetRState();
#if CorePrints
			printf("RobloxState: 0x%X\n", RobloxState);
#endif
			SetLevel(RobloxState, 7);
#if CorePrints
			printf("Roblox Level: 7\n");
#endif
			ExtraSpace::GetRoot(GL)->RL = RobloxState;
			Execute("RobloxAttached()", true);
			return true;
		}

		int Decider(lua_State* L)
		{
#if FunctionNames
			printf("%s Called\n", __FUNCTION__);
#endif
			const uint16_t Args = lua_gettop(L);
			if (RobloxFunc::CheckRobloxObject(L, 1) || (lua_toboolean(L, lua_upvalueindex(3)) ? RobloxFunc::CheckRobloxObject(L, 2) : false))
			{
				lua_pushvalue(L, lua_upvalueindex(2));
				lua_insert(L, -(Args)-1);
				if (lua_pcall(L, Args, LUA_MULTRET, 0) == 0)
				{
					return lua_gettop(L);
				}
				else
				{
					if (lua_type(L, -1) == LUA_TSTRING)
						return lua_error(L);
					else
						return luaL_error(L, "Unkown Error");
				}
			}
			lua_pushvalue(L, lua_upvalueindex(1));
			lua_insert(L, -(Args)-1);
			if (lua_pcall(L, Args, LUA_MULTRET, 0) == 0)
			{
				return lua_gettop(L);
			}
			else
			{
				if (lua_type(L, -1) == LUA_TSTRING)
					return lua_error(L);
				else
					return luaL_error(L, "Unkown Error");
			}
		}

		void FuncWrap(uintptr_t Thread, const char* LibName, const char* Name, int CheckSecond)
		{
#if FunctionNames
			printf("%s Called\n", __FUNCTION__);
#endif
			if (LibName == "")
			{
				lua_getglobal(GL, Name);
				r_lua_getglobal(Thread, Name);
				Wrapping::Wrap(Thread, GL, -1);
				r_lua_pop(Thread, 1);
				if (CheckSecond)
					lua_pushboolean(GL, true);
				lua_pushcclosure(GL, Decider, 2 + CheckSecond);
				lua_setglobal(GL, Name);
			}
			else
			{
				lua_getglobal(GL, LibName);
				lua_getfield(GL, -1, Name);
				r_lua_getglobal(Thread, LibName);
				r_lua_getfield(Thread, -1, Name);
				Wrapping::Wrap(Thread, GL, -1);
				r_lua_pop(Thread, 2);
				if (CheckSecond)
					lua_pushboolean(GL, true);
				lua_pushcclosure(GL, Decider, 2 + CheckSecond);
				lua_setfield(GL, -2, Name);
				lua_pop(GL, 1);
			}
		}

		void WrapGlobals(uintptr_t RL)
		{
#if FunctionNames
			printf("%s Called\n", __FUNCTION__);
#endif
			for (int i = 0; i < Globals.size(); i++)
			{
#if CorePrints
				printf("Wrapping: %s\n", Globals[i]);
#endif
				r_lua_getglobal(RL, Globals[i]);
				Wrapping::Wrap(RL, GL, -1);
				if (Globals[i] == "Game" || Globals[i] == "game")//Global index on my side :) So no no no detection. Benefit of wrapper sometimes :)
				{
					lua_getmetatable(GL, -1);
					lua_pushcfunction(GL, MetaMethods::UDIndex);
					lua_setfield(GL, -2, "__index");
					lua_pop(GL, 1);
				}
				lua_setglobal(GL, Globals[i]);
				r_lua_pop(RL, 1);
			}

			for (int i = 0; i < Sub.size(); i++)
			{
#if CorePrints
				printf("Wrapping: %s.%s\n", Sub[i], Sub[i + 1]);
#endif
				r_lua_getglobal(RL, Sub[i]);
				r_lua_getfield(RL, -1, Sub[i + 1]);
				lua_getglobal(GL, Sub[i]);
				Wrapping::Wrap(RL, GL, -1);
				lua_setfield(GL, -2, Sub[i + 1]);
				r_lua_pop(RL, 2);
				i++;
			}
#if CorePrints
			printf("Wrapped All Globals\n");
#endif
		}

		int RobloxAttached(lua_State* L)
		{
#if FunctionNames
			printf("%s Called\n", __FUNCTION__);
#endif
#if CorePrints
			printf("Start Up Thread Loaded\n");
#endif
			uintptr_t Thread = ExtraSpace::GetRoot(L)->RL;//Ignore me
			r_lua_getglobal(Thread, "game");
			r_lua_getfield(Thread, -1, "HttpGetAsync");
			Wrapping::Wrap(Thread, GL, -1);
			r_lua_pop(Thread, 2);
			lua_setglobal(GL, "HttpGetAsync");
#if CorePrints
			printf("HttpGetASync Bypassed\n");
#endif

			WrapGlobals(Thread);

			FuncWrap(Thread, "", "ipairs", true);
			FuncWrap(Thread, "", "pairs", true);
			FuncWrap(Thread, "", "rawget");
			FuncWrap(Thread, "", "rawset");
			FuncWrap(Thread, "table", "foreach");
			FuncWrap(Thread, "table", "foreachi");
			FuncWrap(Thread, "table", "insert");
			FuncWrap(Thread, "table", "concat");
			FuncWrap(Thread, "table", "remove");
			FuncWrap(Thread, "table", "sort");
			FuncWrap(Thread, "table", "getn");
			FuncWrap(Thread, "table", "setn");
			FuncWrap(Thread, "table", "maxn");
			FuncWrap(Thread, "", "next");
			FuncWrap(Thread, "", "tostring");
			FuncWrap(Thread, "", "unpack");
			FuncWrap(Thread, "", "setmetatable");
			FuncWrap(Thread, "", "getmetatable");
			FuncWrap(Thread, "", "getfenv");
			FuncWrap(Thread, "", "setfenv");
			
#if CorePrints
			printf("Function Fixing Done\n");
#endif
			RunSteppedHook::AttachHook(Thread);
			TeleportHandler::AttachTeleportClosure(Thread);
#if CorePrints
			printf("Attached Teleport Hook\n");
#endif
			Ready = true;
#if CorePrints
			printf("Running Init Script\n");
#endif
			Execute(Tools::Files::ReadFile(Tools::Files::GetDLLPath("\\Init.lua")));

			return 0;
		}

		void SandBoxThread(lua_State* Thread)
		{
#if FunctionNames
			printf("%s Called\n", __FUNCTION__);
#endif
			//What's that? I completely copied Roblox's Sandbox Thread function?
			//Oh dear
			//How sad ;(
			lua_newtable(Thread);
			lua_newtable(Thread);
			lua_pushstring(Thread, "__index");
			lua_pushvalue(Thread, LUA_GLOBALSINDEX);
			lua_settable(Thread, -3);
			lua_setmetatable(Thread, -2);
			lua_replace(Thread, LUA_GLOBALSINDEX);
		}

		bool Execute(std::string Script, bool Minimal)
		{
#if FunctionNames
			printf("%s Called\n", __FUNCTION__);
#endif
			if (!Ready && !Minimal)
				return false;
#if CorePrints
			printf("Executing\n");
#endif
			lua_State* Thread = lua_newthread(GL);
			lua_rawseti(GL, LUA_REGISTRYINDEX, reinterpret_cast<uintptr_t>(Thread));
			SandBoxThread(Thread);
#if CorePrints
			printf("Thread Created and SandBoxed\n");
#endif
			const std::string ChunkName = "@" + std::string(ExploitName);
			const uint8_t Load = luaL_loadbuffer(Thread, Script.c_str(), Script.length(), ChunkName.c_str());
#if CorePrints
			printf("Loaded Script\n");
#endif
			switch (Load)
			{
			case LUA_OK://Ok
			{
				if (Minimal)
				{
#if CorePrints
					printf("Minimal Executing\n");
#endif
					uintptr_t OldRL = ExtraSpace::GetRoot(GL)->RL;
					auto Space = ExtraSpace::GetRoot(Thread);
					Space->RL = r_lua_newthread(OldRL);
					Space->RLRef = r_luaL_ref(OldRL, LUA_REGISTRYINDEX);
					r_lua_getglobal(Space->RL, "spawn");
					Wrapping::StartUpWrap(Thread, Space->RL, -1);
					if (r_lua_pcall(Space->RL, 1, 0, 0) == LUA_OK)
					{
						return true;
					}
					else
					{
						r_print(3, r_lua_tostring(Space->RL, -1));
						return false;
					}
				}
				else
				{
#if CorePrints
					printf("Normal Execution\n");
#endif
					Wrapping::StartUpWrap(Thread, RunSteppedHook::ScheduleThread, -1);
					return true;
				}
			}
			case LUA_ERRSYNTAX:
			case LUA_ERRMEM:
			{
				r_print(3, lua_tostring(Thread, -1));
				lua_pop(Thread, 1);
				break;
			}
			default:
			{
				r_print(3, "Unknown Error while loading Script");
			}
			}
			return false;
		}
	}

	namespace RunSteppedHook
	{
		uint32_t ScheduleThread;

		bool AttachHook(uintptr_t RL)
		{
#if FunctionNames
			printf("%s Called\n", __FUNCTION__);
#endif
			ScheduleThread = r_lua_newthread(RL);
			r_luaL_ref(RL, LUA_REGISTRYINDEX);
			r_lua_getglobal(RL, "game");
			r_lua_getfield(RL, -1, "GetService");
			r_lua_insert(RL, -2);
			r_lua_pushstring(RL, "RunService");
			uint8_t Result = r_lua_pcall(RL, 2, 1, 0);
			if (Result == 0)
			{
				r_lua_getfield(RL, -1, "RenderStepped");
				r_lua_getfield(RL, -1, "Connect");
				r_lua_insert(RL, -2);
#if MemCheckBypass
				r_lua_pushcclosure(RL, reinterpret_cast<uint32_t>(Hook), NULL, 0, NULL);
#elif FullyAutoUpdatingCallCheck
				RobloxFunc::r2_lua_pushcclosure(RL, Bypasses::CallCheck::int3breakpoints[4].BreakPoint, 0, true);
#else
				RobloxFunc::r2_lua_pushcclosure(RL, reinterpret_cast<uint32_t>(Hook), 0);
#endif
				Result = r_lua_pcall(RL, 2, 0, 0);
				if (Result == 0)
				{
					return true;
				}
			}
			return false;
		}

		int Hook(uintptr_t RL)
		{
			const uintptr_t Space = *reinterpret_cast<uintptr_t*>(RL + x_Identity2);
			*reinterpret_cast<uintptr_t*>(Space + x_ScriptPtr) = NULL;
			for (int i = 1; i <= r_lua_gettop(ScheduleThread); i++)
			{
				r_lua_getglobal(RL, "spawn");
				r_lua_xmove(ScheduleThread, RL, 1);
				const uint8_t Result = r_lua_pcall(RL, 1, 0, 0);
				if (Result != 0)
				{
					r_luaL_error(RL, r_lua_tostring(RL, -1));
				}
			}
			return 0;
		}
	}

	namespace TeleportHandler
	{
		std::vector<const char*> TeleportStateEnum = { "RequestedFromServer", "Started", "WaitingForServer", "Failed", "InProgress" };

		bool AttachTeleportClosure(uintptr_t RL)
		{
			while (true)
			{
				r_lua_getglobal(RL, "game");
				r_lua_getfield(RL, -1, "GetService");
				r_lua_insert(RL, -2);
				r_lua_pushstring(RL, "Players");
				uint8_t Result = r_lua_pcall(RL, 2, 1, 0);
				if (Result == 0)
				{
					r_lua_getfield(RL, -1, "LocalPlayer");
					r_lua_remove(RL, -2);
					if (r_lua_type(RL, -1) == R_LUA_TUSERDATA)
					{
						r_lua_getfield(RL, -1, "OnTeleport");
						r_lua_remove(RL, -2);
						r_lua_getfield(RL, -1, "Connect");
						r_lua_insert(RL, -2);
#if MemCheckBypass
						r_lua_pushcclosure(RL, reinterpret_cast<uintptr_t>(TeleportHitClosure), NULL, 0, NULL);
#elif FullyAutoUpdatingCallCheck
						RobloxFunc::r2_lua_pushcclosure(RL, Bypasses::CallCheck::int3breakpoints[3].BreakPoint, 0, true);
#else
						RobloxFunc::r2_lua_pushcclosure(RL, reinterpret_cast<uintptr_t>(TeleportHitClosure), 0);
#endif
						Result = r_lua_pcall(RL, 2, 0, 0);
						if (Result == 0)
						{
#if DebugTeleport
							printf("[Teleport Handler] Running\n");
#endif
							return true;
						}
						else
						{
#if DebugTeleport
							printf("[Teleport Handler] Failed\n");
#endif
							return false;
						}
					}
					else
					{
						r_lua_pop(RL, 1);
					}
				}
			}
		}

		int TeleportHitClosure(uintptr_t RL)
		{
			uintptr_t TeleportStateUD = r_lua_touserdata(RL, 1);
			int8_t TeleportStateD = -1;
			__int64 PlaceID = (__int64)r_lua_tonumber(RL, 2, NULL);
			r_lua_getglobal(RL, "Enum");
			r_lua_getfield(RL, -1, "TeleportState");
			for (int i = 0; i < TeleportStateEnum.size(); i++)
			{
				r_lua_getfield(RL, -1, TeleportStateEnum[i]);
				if (TeleportStateUD == r_lua_touserdata(RL, -1))
				{
					TeleportStateD = i;
					r_lua_pop(RL, 3);
					break;
				}
				else
				{
					r_lua_pop(RL, 1);
				}
			}
			if (TeleportStateD == -1)
				r_lua_pop(RL, 2);
#if DebugTeleport
			printf("[Teleport Handler] State: %s PlaceID: %I64d\n", TeleportStateEnum[TeleportStateD], PlaceID);
#endif
			if (TeleportStateD == 4 && Primary::Ready)
			{
				Primary::Ready = false;
#if MemCheckBypass
				for (int i = 0; i < TaskScheduler::MemoryChanges.size(); i++)
					TaskScheduler::WriteMemory(TaskScheduler::MemoryChanges[i].Address, TaskScheduler::MemoryChanges[i].Reverts, TaskScheduler::MemoryChanges[i].Size, true);
				auto MemCheckJob = TaskScheduler::GetJobByName("US14116");
				TaskScheduler::ReplaceMainFunc(MemCheckJob, reinterpret_cast<uintptr_t>(TaskScheduler::OriginalMemcheckJob));
#endif
				CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)Teleporting, NULL, NULL, NULL);
			}
			return 0;
		}

		void Teleporting()
		{
			uintptr_t TempDM;
			while (true)
			{
				TempDM = GetDataModel();
				if (TempDM != DataModel)
				{
					break;
				}
			}
#if MemCheckBypass
			while (*reinterpret_cast<uint8_t*>((DataModel - x_InstanceClass) + x_IsLoaded) != true)//IsLoaded
				Sleep(50);
			TaskScheduler::Jobs.clear();
			TaskScheduler::LoadJobs();
			auto MemCheckJob = TaskScheduler::GetJobByName("US14116");
			TaskScheduler::ReplaceMainFunc(MemCheckJob, reinterpret_cast<uintptr_t>(TaskScheduler::MemcheckHook));
#endif
			Primary::ROpenState();
		}
	}

	namespace Wrapping
	{

		void StartUpWrap(lua_State* L, uintptr_t RL, const int32_t idx)
		{
			lua_pushvalue(L, idx);
			const uintptr_t Key = reinterpret_cast<uintptr_t>(lua_topointer(L, -1));
			lua_rawseti(L, LUA_REGISTRYINDEX, Key);
			r_lua_pushnumber(RL, Key);
			r_lua_pushlightuserdata(RL, L);
#if MemCheckBypass
			r_lua_pushcclosure(RL, reinterpret_cast<uint32_t>(FunctionHandling::StartUpCall), NULL, 2, NULL);
#elif FullyAutoUpdatingCallCheck
			RobloxFunc::r2_lua_pushcclosure(RL, Bypasses::CallCheck::int3breakpoints[2].BreakPoint, 2, true);
#else
			RobloxFunc::r2_lua_pushcclosure(RL, reinterpret_cast<uintptr_t>(FunctionHandling::StartUpCall), 2);
#endif
		}

		void Wrap(lua_State* L, uintptr_t RL, const int32_t idx)
		{
			const uint8_t Type = lua_type(L, idx);
#if DebugWrapping
			printf("L->RL Type: %s Idx: %d\n", lua_typename(L, Type), idx);
#endif
			switch (Type)
			{
			case LUA_TNIL:
			{
				r_lua_pushnil(RL);
				break;
			}
			case LUA_TBOOLEAN:
			{
				r_lua_pushboolean(RL, lua_toboolean(L, idx));
				break;
			}
			case LUA_TLIGHTUSERDATA:
			{
				r_lua_pushlightuserdata(RL, lua_touserdata(L, idx));
				break;
			}
			case LUA_TNUMBER:
			{
				r_lua_pushnumber(RL, lua_tonumber(L, idx));
				break;
			}
			case LUA_TSTRING:
			{
				r_lua_pushstring(RL, lua_tostring(L, idx));
				break;
			}
			case LUA_TTABLE:
			{
				if (!luaL_getmetafield(L, idx, "__key"))
				{
					const uintptr_t TRef = reinterpret_cast<uint32_t>(lua_topointer(L, idx));
					lua_pushvalue(L, idx);
					r_lua_newtable(RL);
					lua_pushnil(L);
					while (lua_next(L, -2))
					{
						Wrap(L, RL, -2);
						if (TRef == reinterpret_cast<uintptr_t>(lua_topointer(L, -1)))//Done to fix the table inside itself
						{
							r_lua_pushvalue(RL, -2);
						}
						else
							Wrap(L, RL, -1);
						r_lua_settable(RL, -3);
						lua_pop(L, 1);
					}
					if (lua_getmetatable(L, -1))
					{
						Wrap(L, RL, -1);
						lua_pop(L, 1);
						r_lua_setmetatable(RL, -2);
					}

					lua_pop(L, 1);
				}
				else
				{
					const uint32_t Key = lua_tonumber(L, -1);
					lua_pop(L, 1);
					r_lua_rawgeti(RL, LUA_REGISTRYINDEX, Key);
#if DebugWrapping
					printf("Roblox Table Ref: %d\n", Key);
#endif
					if (r_lua_type(RL, -1) != R_LUA_TTABLE)
					{
#if DebugWrapping
						printf("Ref: %d Has Been Dereferenced\n", Key);
#endif
						r_lua_pop(RL, 1);
						r_lua_pushnil(RL);
					}
				}
				break;
			}
			case LUA_TFUNCTION:
			{
				lua_pushvalue(L, idx);
				if (lua_tocfunction(L, -1) == FunctionHandling::RobloxCall)//Prevent double wrapping and memory usage
				{
					if (lua_getupvalue(L, -1, 1))
					{
						if (lua_type(L, -1) == LUA_TNUMBER)
						{
							const uint32_t Ref = lua_tonumber(L, -1);
							lua_pop(L, 1);
							r_lua_rawgeti(RL, LUA_REGISTRYINDEX, Ref);
							if (r_lua_type(RL, -1) == R_LUA_TFUNCTION)
							{
								lua_pop(L, 1);
								return;
							}
							else
							{
								r_lua_pop(RL, 1);
							}
						}
						else
						{
							lua_pop(L, 1);
						}
					}
				}
				const uintptr_t Key = reinterpret_cast<uintptr_t>(lua_topointer(L, -1));
				lua_rawseti(L, LUA_REGISTRYINDEX, Key);
				r_lua_pushnumber(RL, Key);
				r_lua_pushlightuserdata(RL, ExtraSpace::GetRoot(L)->Main);
#if MemCheckBypass
				r_lua_pushcclosure(RL, reinterpret_cast<uint32_t>(FunctionHandling::VanillaCall), NULL, 2, NULL);
#elif FullyAutoUpdatingCallCheck
				RobloxFunc::r2_lua_pushcclosure(RL, Bypasses::CallCheck::int3breakpoints[0].BreakPoint, 2, true);
#else
				RobloxFunc::r2_lua_pushcclosure(RL, reinterpret_cast<uintptr_t>(FunctionHandling::VanillaCall), 2);
#endif
				break;
			}
			case LUA_TUSERDATA:
			{
				void* UD = reinterpret_cast<void*>(lua_touserdata(L, idx));
				r_lua_pushlightuserdata(RL, UD);
				r_lua_rawget(RL, LUA_REGISTRYINDEX);
				if (r_lua_type(RL, -1) != R_LUA_TUSERDATA)
				{
					r_lua_pop(RL, 1);
					void* RUD = reinterpret_cast<void*>(r_lua_newuserdata(RL, 0, NULL));
					if (lua_getmetatable(L, idx))
					{
						Wrap(L, RL, -1);
						lua_pop(L, 1);
						r_lua_setmetatable(RL, -2);
					}
					lua_pushvalue(L, idx);
					lua_pushlightuserdata(L, RUD);
					lua_insert(L, -2);
					lua_rawset(L, LUA_REGISTRYINDEX);
					r_lua_pushlightuserdata(RL, UD);
					r_lua_pushvalue(RL, -2);
					r_lua_rawset(RL, LUA_REGISTRYINDEX, NULL);
				}
				break;
			}
			case LUA_TTHREAD:
			{
				const uintptr_t Ref = reinterpret_cast<uintptr_t>(lua_topointer(L, idx));
				r_lua_rawgeti(RL, LUA_REGISTRYINDEX, Ref);
				if (r_lua_type(RL, -1) != R_LUA_TTHREAD)
				{
					r_lua_pop(RL, 1);
					r_lua_newthread(RL);
				}
				break;
			}
			}

		}

		void Wrap(uintptr_t RL, lua_State* L, const int32_t idx)
		{
			const uint8_t Type = r_lua_type(RL, idx);
#if DebugWrapping
			printf("RL->L Type: %s Idx: %d\n", r_lua_typename(RL, Type), idx);
#endif
			if (Type == R_LUA_TNIL)
			{
				lua_pushnil(L);
			}
			else if (Type == R_LUA_TBOOLEAN)
			{
				lua_pushboolean(L, r_lua_toboolean(RL, idx));
			}
			else if (Type == R_LUA_TLIGHTUSERDATA)
			{
				lua_pushlightuserdata(L, reinterpret_cast<uintptr_t*>(r_lua_touserdata(RL, idx)));
			}
			else if (Type == R_LUA_TNUMBER)
			{
				lua_pushnumber(L, r_lua_tonumber(RL, idx, NULL));
			}
			else if (Type == R_LUA_TSTRING)
			{
				lua_pushstring(L, r_lua_tostring(RL, idx));
			}
			else if (Type == R_LUA_TTABLE)
			{
				r_lua_pushvalue(RL, idx);
				lua_newtable(L);
				index2adr(L, -1)->value.gc->h.readonly = (*reinterpret_cast<uintptr_t*>(r_index2adr(RL, -1)) + x_IsReadOnly);
				const uint32_t Ref = reinterpret_cast<int32_t>(lua_topointer(L, -1));
				r_lua_rawseti(RL, LUA_REGISTRYINDEX, Ref);

				lua_newtable(L);
				lua_pushnumber(L, Ref);
				lua_pushcclosure(L, MetaMethods::Index, 1);
				lua_setfield(L, -2, "__index");
				lua_pushnumber(L, Ref);
				lua_pushcclosure(L, MetaMethods::NewIndex, 1);
				lua_setfield(L, -2, "__newindex");
				lua_pushnumber(L, Ref);
				lua_pushcclosure(L, MetaMethods::Len, 1);
				lua_setfield(L, -2, "__len");
				lua_pushnumber(L, Ref);
				lua_setfield(L, -2, "__key");
				if (r_lua_getmetatable(RL, idx))
				{
					r_lua_pushnil(RL);
					while (r_lua_next(RL, -2))
					{
						Wrap(RL, L, -2);
						Wrap(RL, L, -1);
						lua_settable(L, -3);
						r_lua_pop(RL, 1);
					}
					r_lua_pop(RL, 1);
				}
				lua_setmetatable(L, -2);
			}
			else if (Type == R_LUA_TFUNCTION)
			{
				r_lua_pushvalue(RL, idx);
				if (GetAddress(reinterpret_cast<uintptr_t>(r_lua_topointer(RL, -1))) == reinterpret_cast<uintptr_t>(FunctionHandling::VanillaCall))
				{
					if (RobloxFunc::r2_lua_getupvalue(RL, -1, 1))
					{
						if (r_lua_type(RL, -1) == R_LUA_TNUMBER)
						{
							const uint32_t Key = r_lua_tonumber(RL, -1, NULL);
							lua_rawgeti(L, LUA_REGISTRYINDEX, Key);
							if (lua_type(L, -1) == LUA_TFUNCTION)
							{
								r_lua_pop(RL, 2);
								return;
							}
							else
							{
								lua_pop(L, 1);
							}
						}
						r_lua_pop(RL, 1);
					}
				}
				const uintptr_t Ref = reinterpret_cast<uintptr_t>(r_lua_topointer(RL, -1));
				r_lua_rawseti(RL, LUA_REGISTRYINDEX, Ref);
				lua_pushnumber(L, Ref);
				lua_pushcclosure(L, FunctionHandling::RobloxCall, 1);
			}
			else if (Type == R_LUA_TUSERDATA)
			{
				r_lua_pushvalue(RL, idx);

				uintptr_t* UD = reinterpret_cast<uintptr_t*>(r_lua_touserdata(RL, -1));
				lua_pushlightuserdata(L, UD);
				lua_rawget(L, LUA_REGISTRYINDEX);
				if (lua_type(L, -1) != LUA_TUSERDATA)
				{
					r_luaL_getmetafield(RL, -1, "__type");
					const char* Type = r_lua_tostring(RL, -1);
					r_lua_pop(RL, 1);
					lua_pop(L, 1);
					uintptr_t* NUD = reinterpret_cast<uintptr_t*>(lua_newuserdata(L, 0));
					if (r_lua_getmetatable(RL, -1))
					{
						lua_newtable(L);
						r_lua_pushnil(RL);
						while (r_lua_next(RL, -2))
						{
							Wrap(RL, L, -2);
							Wrap(RL, L, -1);
							lua_settable(L, -3);
							r_lua_pop(RL, 1);
						}
						r_lua_pop(RL, 1);
#if !DisableGC
						lua_pushcfunction(L, MetaMethods::UDGC);
						lua_setfield(L, -2, "__gc");
#endif
						lua_setmetatable(L, -2);
					}
					r_lua_pushlightuserdata(RL, NUD);
					r_lua_insert(RL, -2);
					r_lua_rawset(RL, LUA_REGISTRYINDEX, NULL);
					if (!strcmp(Type, "Instance") || !strcmp(Type, "EnumItem"))
					{
						lua_pushlightuserdata(L, UD);
						lua_pushvalue(L, -2);
						lua_rawset(L, LUA_REGISTRYINDEX);
					}
				}
				else
				{
					r_lua_pop(RL, 1);
				}
			}
			else if (Type == R_LUA_TTHREAD)
			{
				r_lua_pushvalue(RL, idx);
				lua_newthread(L);
				r_lua_rawseti(RL, LUA_REGISTRYINDEX, reinterpret_cast<uintptr_t>(lua_topointer(L, -1)));
			}
		}

	}

	namespace YieldingHandling
	{
		int Resume(uintptr_t RL)
		{
#if FunctionNames
			printf("%s Called\n", __FUNCTION__);
#endif
			lua_State* L = reinterpret_cast<lua_State*>(r_lua_touserdata(RL, lua_upvalueindex(1)));
			auto Space = ExtraSpace::GetRoot(L);
			if (L->status != LUA_YIELD)
			{
				r_luaL_error(RL, "Attempt to resume non-yielded thread");
				return 0;
			}
			const uint32_t YieldedRef = Space->RLRef;
			const uint16_t Args = r_lua_gettop(RL);
			lua_checkstack(L, Args);
			for (int i = 1; i <= Args; ++i)
				Wrapping::Wrap(RL, L, i);
			r_lua_settop(RL, 0);
			Space->RL = r_lua_newthread(RL);
			Space->RLRef = r_luaL_ref(RL, LUA_REGISTRYINDEX);
			Space->RLGiven = RL;
			*reinterpret_cast<uintptr_t*>(Space->RL + x_namecall) = *reinterpret_cast<uintptr_t*>(RL + x_namecall);
			if (VResume(L, Args) != LUA_YIELD)
			{
				if (L != Space->Main && Space->Coroutine != true)
				{
					r_luaL_unref(Space->RL, LUA_REGISTRYINDEX, Space->RLRef);
					lua_pushnumber(Space->Main, reinterpret_cast<uintptr_t>(L));
					lua_rawseti(Space->Main, LUA_REGISTRYINDEX, reinterpret_cast<uintptr_t>(L));
				}
			}
			r_luaL_unref(RL, LUA_REGISTRYINDEX, YieldedRef);
			return 0;
		}

		uint8_t VResume(lua_State* L, const uint16_t args)
		{
#if FunctionNames
			printf("%s Called\n", __FUNCTION__);
#endif
			auto Space = ExtraSpace::GetRoot(L);
			auto Continues = Space->Continues.get();
#if DebugCalls
			printf("[Env: Vanilla] [Action: Resuming] [Thread: 0x%X]\n", L);
#endif
			const uint8_t Result = lua_resume(L, args);
			switch (Result)
			{
			case LUA_OK:
			{
#if DebugCalls
				printf("[Env: Vanilla] [Action: Success] [Thread: 0x%X]\n", L);
#endif
				if (Continues)
				{
					if (Continues->Success)
					{
						try
						{
							(Continues->Success)(L);
						}
						catch (const std::exception& e)
						{
							lua_pushstring(L, e.what());
							if (Continues->Error)
								(Continues->Error)(L);
						}
					}
				}
				break;
			}
			case LUA_YIELD:
			{
#if DebugCalls
				printf("[Env: Vanilla] [Action: Yielded] [Thread: 0x%X]\n", L);
#endif
				break;
			}
			default:
			{
#if DebugCalls
				printf("[Env: Vanilla] [Action: Error] [Thread: 0x%X]\n", L);
#endif
				
				if (Continues)
				{
					if (Continues->Error)
						(Continues->Error)(L);
				}
				else
				{
					if (lua_type(L, -1) == LUA_TSTRING)
					{
						luaL_where(L, 1);
						lua_pushfstring(L, "%s%s", lua_tostring(L, -1), lua_tostring(L, -2));
						r_print(3, lua_tostring(L, -1));
						lua_pop(L, 3);
						RobloxFunc::printCallStack(L);
					}
					else
					{
						luaL_where(L, 1);
						lua_pushfstring(L, "%s%s", lua_tostring(L, -1), "No Error Message");
						r_print(3, lua_tostring(L, -1));
						lua_pop(L, 2);
						RobloxFunc::printCallStack(L);
					}
				}
			}
			}
			return Result;
		}

		bool CheckRobloxYield(lua_State* L)
		{
#if FunctionNames
			printf("%s Called\n", __FUNCTION__);
#endif
			auto Space = ExtraSpace::GetRoot(L);
			if (r_lua_type(Space->RL, -1) == R_LUA_TFUNCTION)
			{
				const uintptr_t Closure = reinterpret_cast<uintptr_t>(r_lua_topointer(Space->RL, -1));
				uintptr_t Address = GetAddress(Closure);
#if MemCheckBypass
				if (Address == reinterpret_cast<uint32_t>(Resume))
					return true;
#elif FullyAutoUpdatingCallCheck
				if (Address == Bypasses::CallCheck::int3breakpoints[1].BreakPoint)
					return true;
#else
				if (Address == Bypasses::CallCheck::HookLocation)
				{
					const uint8_t Nups = *reinterpret_cast<uint8_t*>(Closure + x_closurenups);
					RobloxFunc::r2_lua_getupvalue(Space->RL, -1, Nups);
					Address = r_lua_tonumber(Space->RL, -1, NULL);
					r_lua_pop(Space->RL, 1);
					if (Address == reinterpret_cast<uintptr_t>(Resume))
					{
						return true;
					}
				}
#endif
			}
			return false;
		}

		//Pcall Section

		void PcallOnSuccess(lua_State* L)
		{
#if FunctionNames
			printf("%s Called\n", __FUNCTION__);
#endif
			auto Space = ExtraSpace::GetRoot(L);
			auto Continues = Space->Continues.get();
			lua_State* Dom = Continues->Prior;
			auto Space2 = ExtraSpace::GetRoot(Dom);
			lua_pushboolean(Dom, true);
			int Returns = lua_gettop(L);
			lua_xmove(L, Dom, Returns);
			if (VResume(Dom, Returns + 1) != LUA_YIELD)
			{
				if (Dom != Space->Main && Space2->Coroutine != true)
				{
					lua_pushnumber(Space->Main, reinterpret_cast<uintptr_t>(Dom));
					lua_rawseti(Space->Main, LUA_REGISTRYINDEX, reinterpret_cast<uintptr_t>(Dom));
				}
			}
		}

		void PcallOnError(lua_State* L)
		{
#if FunctionNames
			printf("%s Called\n", __FUNCTION__);
#endif
			auto Space = ExtraSpace::GetRoot(L);
			auto Continues = Space->Continues.get();
			lua_State* Dom = Continues->Prior;
			auto Space2 = ExtraSpace::GetRoot(Dom);
			lua_pushboolean(Dom, false);
			int Returns = lua_gettop(L);
			lua_xmove(L, Dom, Returns);
			if (VResume(Dom, Returns + 1) != LUA_YIELD)
			{
				if (Dom != Space->Main && Space2->Coroutine != true)
				{
					lua_pushnumber(Space->Main, reinterpret_cast<uintptr_t>(Dom));
					lua_rawseti(Space->Main, LUA_REGISTRYINDEX, reinterpret_cast<uintptr_t>(Dom));
				}
			}
		}

		int PCall(lua_State* L)
		{
#if FunctionNames
			printf("%s Called\n", __FUNCTION__);
#endif
			auto Space = ExtraSpace::GetRoot(L);
			const uint16_t Args = lua_gettop(L);
			lua_State* Sub = lua_newthread(L);
			auto NewSpace = ExtraSpace::GetRoot(Sub);
			NewSpace->RL = r_lua_newthread(Space->RL);
			NewSpace->RLRef = r_luaL_ref(Space->RL, LUA_REGISTRYINDEX);
			lua_rawseti(L, LUA_REGISTRYINDEX, reinterpret_cast<uintptr_t>(Sub));
			lua_xmove(L, Sub, Args);
			int result = lua_resume(Sub, Args - 1);
			switch (result)
			{
			case LUA_OK:
			{
				const uint16_t Returns = lua_gettop(Sub);
				lua_pushboolean(L, true);
				lua_xmove(Sub, L, Returns);
				lua_pushnumber(L, reinterpret_cast<uintptr_t>(Sub));
				lua_rawseti(L, LUA_REGISTRYINDEX, reinterpret_cast<uintptr_t>(Sub));
				return Returns + 1;
			}
			case LUA_YIELD:
			{
				Continuations Continues;
				Continues.Success = PcallOnSuccess;
				Continues.Error = PcallOnError;
				Continues.Prior = L;
				ExtraSpace::GetRoot(Sub)->Continues.reset(new Continuations(Continues));
				return lua_yield(L, 0);
			}
			default:
			{
				lua_pushboolean(L, false);
				const uint16_t Returns = lua_gettop(Sub);
				lua_xmove(Sub, L, Returns);
				lua_pushnumber(L, reinterpret_cast<uintptr_t>(Sub));
				lua_rawseti(L, LUA_REGISTRYINDEX, reinterpret_cast<uintptr_t>(Sub));
				return Returns + 1;
			}
			}
		}

		//Coroutines
		namespace Coroutine
		{
			uint8_t costatus(lua_State* L, lua_State* co)
			{
				if (L == co) return CO_RUN;
				switch (lua_status(co)) {
				case LUA_YIELD:
					return CO_SUS;
				case 0: {
					lua_Debug ar;
					if (lua_getstack(co, 0, &ar) > 0)  /* does it have frames? */
						return CO_NOR;  /* it is running */
					else if (lua_gettop(co) == 0)
						return CO_DEAD;
					else
						return CO_SUS;  /* initial state */
				}
				default:  /* some error occured */
					return CO_DEAD;
				}
			}

			int luaB_costatus(lua_State* L) 
			{
				lua_State* co = lua_tothread(L, 1);
				luaL_argcheck(L, co, 1, "coroutine expected");
				lua_pushstring(L, statnames[costatus(L, co)]);
				return 1;
			}

			int32_t auxresume(lua_State* L, lua_State* co, int narg) 
			{
				uint8_t status = costatus(L, co);
				if (!lua_checkstack(co, narg))
					luaL_error(L, "too many arguments to resume");
				if (status != CO_SUS) {
					lua_pushfstring(L, "cannot resume %s coroutine", statnames[status]);
					return -1;  /* error flag */
				}
				lua_xmove(L, co, narg);
				lua_setlevel(L, co);
				status = lua_resume(co, narg);
				if (status == 0 || status == LUA_YIELD) 
				{
					if (CheckRobloxYield(co)) 
						return -2;
					const uint16_t nres = lua_gettop(co);
					if (!lua_checkstack(L, nres + 1))
						luaL_error(L, "too many results to resume");
					lua_xmove(co, L, nres);  /* move yielded values */
					return nres;
				}
				else {
					lua_xmove(co, L, 1);  /* move error message */
					return -1;  /* error flag */
				}
			}

			//Coresume

			int luaB_coresume(lua_State* L)
			{
#if FunctionNames
				printf("%s Called\n", __FUNCTION__);
#endif
				lua_State* co = lua_tothread(L, 1);
				luaL_argcheck(L, co, 1, "coroutine expected");
				const int32_t r = auxresume(L, co, lua_gettop(L) - 1);
				if (r < 0) 
				{
					if (r == -2)
					{
						return 0;
					}
					lua_pushboolean(L, 0);
					lua_insert(L, -2);
					return 2;  /* return false + error message */
				}
				else 
				{
					lua_pushboolean(L, 1);
					lua_insert(L, -(r + 1));
					return r + 1;  /* return true + `resume' returns */
				}
			}

			//AuxWrap

			int luaB_auxwrap(lua_State* L)
			{
#if FunctionNames
				printf("%s Called\n", __FUNCTION__);
#endif
				lua_State* co = lua_tothread(L, lua_upvalueindex(1));
				const int32_t r = auxresume(L, co, lua_gettop(L));
				if (r < 0) 
				{
					if (r == -2) 
					{
						return 0;//Roblox is fucking stupid ugh
					}
					if (lua_isstring(L, -1)) 
					{  /* error object is a string? */
						luaL_where(L, 1);  /* add extra info */
						lua_insert(L, -2);
						lua_concat(L, 2);
					}
					lua_error(L);  /* propagate error */
				}
				return r;
			}

			int luaB_cocreate(lua_State* L)
			{
#if FunctionNames
				printf("%s Called\n", __FUNCTION__);
#endif
				auto Space = ExtraSpace::GetRoot(L);
				lua_State* NL = lua_newthread(L);
				auto NewSpace = ExtraSpace::GetRoot(NL);
				NewSpace->Coroutine = true;
				NewSpace->RL = r_lua_newthread(Space->RL);
				NewSpace->RLRef = r_luaL_ref(Space->RL, LUA_REGISTRYINDEX);
				lua_pushvalue(L, -1);
				lua_rawseti(L, LUA_REGISTRYINDEX, reinterpret_cast<uintptr_t>(NL));
				luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1), 1,
					"Lua function expected");
				lua_pushvalue(L, 1);  /* move function to top */
				lua_xmove(L, NL, 1);  /* move function from L to NL */
				return 1;
			}

			int luaB_cowrap(lua_State* L) 
			{
#if FunctionNames
				printf("%s Called\n", __FUNCTION__);
#endif
				luaB_cocreate(L);
				lua_pushcclosure(L, luaB_auxwrap, 1);
				return 1;
			}

			int luaB_yield(lua_State* L)
			{
#if FunctionNames
				printf("%s Called\n", __FUNCTION__);
#endif
				return lua_yield(L, lua_gettop(L));
			}
		}

	}

	namespace FunctionHandling
	{
		using namespace Wrapping;

		/*
This function is what Roblox calls when it wants to call a Vanilla Closure.
*/
		int StartUpCall(uintptr_t RL)
		{
#if FunctionNames
			printf("%s Called\n", __FUNCTION__);
#endif
			const uint32_t Ref = r_lua_tonumber(RL, lua_upvalueindex(1), NULL);
			lua_State* Thread = reinterpret_cast<lua_State*>(r_lua_touserdata(RL, lua_upvalueindex(2)));
			auto Space = ExtraSpace::GetRoot(Thread);
			Space->Main = Thread;
			Space->RLGiven = RL;
			//Script = "script = Instance.new('LocalScript') script.Disabled = true script.Parent = nil " + Script;
			r_lua_getglobal(RL, "Instance");
			r_lua_getfield(RL, -1, "new");
			r_lua_remove(RL, -2);
			r_lua_pushstring(RL, "LocalScript");
			r_lua_pcall(RL, 1, 1, 0);
			r_lua_pushboolean(RL, true);
			r_lua_setfield(RL, -2, "Disabled");
			r_lua_pushnil(RL);
			r_lua_setfield(RL, -2, "Parent");
			Wrap(RL, Thread, -1);
			r_lua_pop(RL, 1);
			lua_setglobal(Thread, "script");
#if DebugThreading
			printf("[Thread Pre] [VThread: 0x%X] [RThread: 0x%X]\n", Thread, RL);
#endif
			//We create a new thread as if the function yields we need to make sure that the resume function doesn't get cleared
			Space->RL = r_lua_newthread(RL);
			Space->RLRef = r_luaL_ref(RL, LUA_REGISTRYINDEX);
#if DebugCalls
			const char* Name;
			if (lua_iscfunction(Thread, -1))
			{
				Name = "CClosure";
				printf("[Env: Vanilla] [Action: Calling] [Thread: 0x%X] [Name: %s]: 0x%x\n", Thread, Name, Ref);
			}
			else
			{
				Name = "LClosure";
				printf("[Env: Vanilla] [Action: Calling] [Thread: 0x%X] [Name: %s]: 0x%x\n", Thread, Name, Ref);
			}
#endif
			const uint8_t ret = lua_resume(Thread, 0);
			r_lua_settop(RL, 0);
			switch (ret)
			{
			case LUA_OK:
			{
#if DebugCalls
				printf("[Env: Vanilla] [Action: Success] [Thread: 0x%X] [Name: %s]: 0x%x\n", Thread, Name, Ref);
#endif
				return 0;
			}
			case LUA_YIELD:
			{
#if DebugCalls
				printf("[Env: Vanilla] [Action: Yielded] [Thread: 0x%X] [Name: %s]: 0x%x\n", Thread, Name, Ref);
#endif
#if DebugThreading
				printf("[Thread Yielded] [VThread: 0x%X] [RThread: 0x%X]\n", Thread, RL);
#endif
				return 0;
			}
			default:
			{
#if DebugCalls
				printf("[Env: Vanilla] [Action: Error] [Thread: 0x%X] [Name: %s]: 0x%x\n", Thread, Name, Ref);
#endif
				if (lua_type(Thread, -1) == LUA_TSTRING)
				{
					luaL_where(Thread, 1);
					lua_pushfstring(Thread, "%s%s", lua_tostring(Thread, -1), lua_tostring(Thread, -2));
					r_print(3, lua_tostring(Thread, -1));
					lua_pop(Thread, 3);
					RobloxFunc::printCallStack(Thread);
					lua_settop(Thread, 0);
					return 0;
				}
				else
				{
					luaL_where(Thread, 1);
					lua_pushfstring(Thread, "%s%s", lua_tostring(Thread, -1), "No Error Message");
					r_print(3, lua_tostring(Thread, -1));
					lua_pop(Thread, 2);
					RobloxFunc::printCallStack(Thread);
					lua_settop(Thread, 0);
					return 0;
				}
			}
			}

		}

		/*
		This function is what Roblox calls when it wants to call a Vanilla Closure.
		*/
		int VanillaCall(uintptr_t RL)
		{
#if FunctionNames
			printf("%s Called\n", __FUNCTION__);
#endif
			uint32_t Ref;
			lua_State* Prior;
			if (r_lua_type(RL, lua_upvalueindex(1)) != R_LUA_TNUMBER)
			{
				uintptr_t CI = *reinterpret_cast<uintptr_t*>(RL + 28);
				uintptr_t Env = *reinterpret_cast<uintptr_t*>(**reinterpret_cast<uintptr_t**>(CI + 4) + 16);
				RobloxFunc::r_lua_pushobject(RL, Env, R_LUA_TTABLE);
				r_lua_getfield(RL, -1, "__func");
				Ref = r_lua_tonumber(RL, -1, NULL);
				r_lua_pop(RL, 1);
				r_lua_getfield(RL, -1, "__thread");
				Prior = reinterpret_cast<lua_State*>(r_lua_touserdata(RL, -1));
				r_lua_pop(RL, 2);
			}
			else
			{
				Ref = r_lua_tonumber(RL, lua_upvalueindex(1), NULL);
				Prior = reinterpret_cast<lua_State*>(r_lua_touserdata(RL, lua_upvalueindex(2)));
			}
			lua_State* Thread = lua_newthread(Prior);
			auto Space = ExtraSpace::GetRoot(Thread);
#if DebugThreading
			printf("[Thread Created] [VThread: 0x%X] [RThread: 0x%X]\n", Thread, RL);
#endif
			lua_rawseti(Prior, LUA_REGISTRYINDEX, reinterpret_cast<uintptr_t>(Thread));

			lua_rawgeti(Thread, LUA_REGISTRYINDEX, Ref);
			//We create a new thread as if the function yields we need to make sure that the resume function doesn't get cleared
			Space->RL = r_lua_newthread(RL);
			Space->RLRef = r_luaL_ref(RL, LUA_REGISTRYINDEX);
			Space->RLGiven = RL;
			*reinterpret_cast<uintptr_t*>(Space->RL + x_namecall) = *reinterpret_cast<uintptr_t*>(RL + x_namecall);
#if DebugCalls
			const char* Name;
			if (lua_iscfunction(Thread, -1))
			{
				Name = "CClosure";
				printf("[Env: Vanilla] [Action: Calling] [Thread: 0x%X] [Name: %s]: 0x%x\n", Thread, Name, Ref);
			}
			else
			{
				Name = "LClosure";
				printf("[Env: Vanilla] [Action: Calling] [Thread: 0x%X] [Name: %s]: 0x%x\n", Thread, Name, Ref);
		}
#endif
			const uint32_t Args = r_lua_gettop(RL);
			if (Args > 0)
			{
#if DebugCalls
				printf("[Env: Vanilla] [Action: Wrapping Args] [Thread: 0x%X] [Name: %s]: 0x%x\n", Thread, Name, Ref);
#endif
				lua_checkstack(Thread, Args);
				for (int i = 1; i <= Args; ++i)
					Wrap(RL, Thread, i);
			}
			const uint8_t ret = lua_resume(Thread, Args);
			r_lua_settop(RL, 0);
			switch (ret)
			{
			case LUA_OK:
			{
#if DebugCalls
				printf("[Env: Vanilla] [Action: Success] [Thread: 0x%X] [Name: %s]: 0x%x\n", Thread, Name, Ref);
#endif
				const uint16_t Returns = lua_gettop(Thread);
				if (Returns > 0)
				{
#if DebugCalls
					printf("[Env: Vanilla] [Action: Wrapping Returns] [Thread: 0x%X] [Name: %s]: 0x%x\n", Thread, Name, Ref);
#endif
					r_lua_checkstack(RL, Returns);
					for (int i = 1; i <= Returns; ++i)
						Wrap(Thread, RL, i);
				}
				lua_settop(Thread, 0);
				r_luaL_unref(RL, LUA_REGISTRYINDEX, Space->RLRef);
#if DebugThreading
				printf("[Thread Ended] [VThread: 0x%X] [RThread: 0x%X]\n", Thread, RL);
#endif
				lua_pushnumber(Prior, reinterpret_cast<uintptr_t>(Thread));
				lua_rawseti(Prior, LUA_REGISTRYINDEX, reinterpret_cast<uintptr_t>(Thread));
				return Returns;
			}
			case LUA_YIELD:
			{
#if DebugCalls
				printf("[Env: Vanilla] [Action: Yielded] [Thread: 0x%X] [Name: %s]: 0x%x\n", Thread, Name, Ref);
#endif
#if DebugThreading
				printf("[Thread Yielded] [VThread: 0x%X] [RThread: 0x%X]\n", Thread, RL);
#endif
				if (!YieldingHandling::CheckRobloxYield(Thread)) 
				{
					const uint32_t Returns = lua_gettop(Thread);
					r_lua_checkstack(RL, Returns);
					for (int i = 1; i <= Returns; ++i)
						Wrap(Thread, RL, i);
					lua_settop(Thread, 0);
					return Returns;
				}
				return r_lua_yield(RL, 0);
			}
			default:
			{
#if DebugCalls
				printf("[Env: Vanilla] [Action: Error] [Thread: 0x%X] [Name: %s]: 0x%x\n", Thread, Name, Ref);
#endif
				if (lua_type(Thread, -1) == LUA_TSTRING)
				{
					luaL_where(Thread, 1);
					lua_pushfstring(Thread, "%s%s", lua_tostring(Thread, -1), lua_tostring(Thread, -2));
					r_print(3, lua_tostring(Thread, -1));
					lua_pop(Thread, 3);
					RobloxFunc::printCallStack(Thread);
					lua_settop(Thread, 0);
					return 0;
				}
				else
				{
					luaL_where(Thread, 1);
					lua_pushfstring(Thread, "%s%s", lua_tostring(Thread, -1), "No Error Message");
					r_print(3, lua_tostring(Thread, -1));
					lua_pop(Thread, 2);
					RobloxFunc::printCallStack(Thread);
					lua_settop(Thread, 0);
					return 0;
				}
			}
			}

		}

		int RobloxCall(lua_State* L)
		{
#if FunctionNames
			printf("%s Called\n", __FUNCTION__);
#endif
			auto Space = ExtraSpace::GetRoot(L);
			uintptr_t RL = Space->RL;
			r_lua_settop(RL, 0);
			const uint32_t FuncRef = lua_tonumber(L, lua_upvalueindex(1));
			r_lua_rawgeti(RL, LUA_REGISTRYINDEX, FuncRef);
#if DebugCalls
			const char* Name;
			uintptr_t Address1 = 0;
			if (r_lua_iscfunction(RL, -1)) {
				Name = GetName(reinterpret_cast<uintptr_t>(r_lua_topointer(RL, -1)));
				Address1 = GetAddress(reinterpret_cast<uintptr_t>(r_lua_topointer(RL, -1)));
			}
			else
				Name = "LClosure";
			printf("[Env: Roblox] [Action: Calling] [Thread: 0x%X] [Name: %s] 0x%X\n", RL, Name, Address1 ? noaslr(Address1) : 0);
#endif
			const uint32_t Args = lua_gettop(L);
			if (Args > 0) 
			{
#if DebugCalls
				printf("[Env: Roblox] [Action: Wrapping Args] [Thread: 0x%X] [Name: %s]\n", RL, Name);
#endif
				r_lua_checkstack(RL, Args);
				for (int i = 1; i <= Args; ++i)
					Wrapping::Wrap(L, RL, i);
			}
			const uint8_t Result = r_lua_pcall(RL, Args, LUA_MULTRET, 0);
			lua_settop(L, 0);
			if (Result == LUA_OK)
			{
#if DebugCalls
				printf("[Env: Roblox] [Action: Success] [Thread: 0x%X] [Name: %s]\n", RL, Name);
#endif
				const uint32_t Returns = r_lua_gettop(RL);
				if (Returns > 0)
				{
#if DebugCalls
					printf("[Env: Roblox] [Action: Wrapping Returns] [Thread: 0x%X] [Name: %s]\n", RL, Name);
#endif
					lua_checkstack(L, Returns);
					for (int i = 1; i <= Returns; i++)
						Wrap(RL, L, i);
				}
				r_lua_settop(RL, 0);
				return Returns;
			}
			else
			{
				if (r_lua_type(RL, -1) == R_LUA_TSTRING)
				{
					const char* Error = r_lua_tostring(RL, -1);
					if (strcmp(Error, "attempt to yield across metamethod/C-call boundary") == 0)
					{
						r_lua_settop(RL, 0);
#if DebugCalls
						printf("[Env: Roblox] [Action: Yielded] [Thread: 0x%X] [Name: %s]\n", RL, Name);
#endif
						r_lua_pushlightuserdata(RL, L);
#if MemCheckBypass
						r_lua_pushcclosure(RL, reinterpret_cast<uintptr_t>(YieldingHandling::Resume), NULL, 1, NULL);
#elif FullyAutoUpdatingCallCheck
						RobloxFunc::r2_lua_pushcclosure(RL, Bypasses::CallCheck::int3breakpoints[1].BreakPoint, 1, true);
#else
						RobloxFunc::r2_lua_pushcclosure(RL, reinterpret_cast<uintptr_t>(YieldingHandling::Resume), 1);
#endif
						return lua_yield(L, 0);
					}
				}
#if DebugCalls
				printf("[Env: Roblox] [Action: Error] [Thread: 0x%X] [Name: %s]\n", RL, Name);
#endif
				if (r_lua_type(RL, -1) != LUA_TNONE)
				{
					Wrap(RL, L, -1);
					r_lua_settop(RL, 0);
				}
				else
					lua_pushnil(L);
				return lua_error(L);
			}
		}
	}

	namespace MetaMethods
	{
		using namespace Wrapping;

		int UDIndex(lua_State* L)
		{
#if FunctionNames
			printf("%s Called\n", __FUNCTION__);
#endif
			const std::string Looking = lua_tostring(L, 2);
			if (Looking == "HttpGet")
			{
				lua_getglobal(L, "HttpGet");
			}
			else if (Looking == "HttpGetAsync")
			{
				lua_getglobal(L, "HttpGet");
			}
			else if (Looking == "GetObjects")
			{
				lua_getglobal(L, "GetObjects");
			}
			else
			{
				uintptr_t RL = ExtraSpace::GetRoot(L)->RL;
				Wrap(L, RL, 1);
				r_lua_getfield(RL, -1, Looking.c_str());
				Wrap(RL, L, -1);
				r_lua_pop(RL, 2);
			}
			return 1;
		}

		int UDGC(lua_State* L)
		{
			void* UD = lua_touserdata(L, 1);
			uintptr_t RL = ExtraSpace::GetRoot(L)->RL;
			r_lua_pushlightuserdata(RL, UD);
			r_lua_rawget(RL, LUA_REGISTRYINDEX);
			if (r_lua_type(RL, -1) == R_LUA_TUSERDATA)
			{
				uintptr_t* NUD = (uintptr_t*)r_lua_touserdata(RL, -1);
				r_lua_pushlightuserdata(RL, reinterpret_cast<void*>(aslr(x_RobloxInstancePushFunction)));//Roblox Push Function
				r_lua_rawget(RL, LUA_REGISTRYINDEX);
				r_lua_pushlightuserdata(RL, (void*)*NUD);
				r_lua_rawget(RL, -2);
				if (r_lua_type(RL, -1) != R_LUA_TUSERDATA) 
				{
					r_lua_pop(RL, 2);
					lua_pushlightuserdata(L, NUD);
					lua_rawget(L, LUA_REGISTRYINDEX);
					if (lua_type(L, -1) == LUA_TUSERDATA)
					{
						lua_pushlightuserdata(L, NUD);
						lua_pushnumber(L, reinterpret_cast<uintptr_t>(NUD));
						lua_rawset(L, LUA_REGISTRYINDEX);
					}
					lua_pop(L, 1);
					r_lua_pushlightuserdata(RL, UD);
					r_lua_pushnumber(RL, reinterpret_cast<uintptr_t>(UD));
					r_lua_rawset(RL, LUA_REGISTRYINDEX, NULL);
					r_lua_pop(RL, 1);
				}
				else
				{
					r_lua_pop(RL, 3);
				}
			}
			else
			{
				r_lua_pop(RL, 1);
			}
			return 0;
		}

		int Index(lua_State* L)
		{
#if FunctionNames
			printf("%s Called\n", __FUNCTION__);
#endif
			uintptr_t RL = ExtraSpace::GetRoot(L)->RL;
			const uint32_t Ref = lua_tonumber(L, lua_upvalueindex(1));
			r_lua_rawgeti(RL , LUA_REGISTRYINDEX, Ref);
			if (r_lua_type(RL, -1) != R_LUA_TNIL)
			{
				Wrap(L, RL, 2);
				r_lua_gettable(RL, -2);
				r_lua_remove(RL, -2);
				Wrap(RL, L, -1);
				r_lua_pop(RL, 1);
			}
			else
			{
#if DebugMetaMethods
				printf("Table not Reffed for Index MetaMethod Ref: %d\n", Ref);
#endif
				lua_pushnil(L);
			}
			return 1;
		}

		int NewIndex(lua_State* L)
		{
#if FunctionNames
			printf("%s Called\n", __FUNCTION__);
#endif
			uintptr_t RL = ExtraSpace::GetRoot(L)->RL;
			const uint32_t Ref = lua_tonumber(L, lua_upvalueindex(1));
			r_lua_rawgeti(RL, LUA_REGISTRYINDEX, Ref);
			if (r_lua_type(RL, -1) != R_LUA_TNIL)
			{
				Wrap(L, RL, 2);
				Wrap(L, RL, 3);
				r_lua_settable(RL, -3);
				r_lua_pop(RL, 1);
			}
			else
			{
#if DebugMetaMethods
				printf("Table not Reffed for NewIndex MetaMethod Ref: %d\n", Ref);
#endif
			}
			return 0;
		}

		int Len(lua_State* L)
		{
#if FunctionNames
			printf("%s Called\n", __FUNCTION__);
#endif
			uintptr_t RL = ExtraSpace::GetRoot(L)->RL;
			const uint32_t Ref = lua_tonumber(L, lua_upvalueindex(1));
			r_lua_rawgeti(RL, LUA_REGISTRYINDEX, Ref);
			if (r_lua_type(RL, -1) != R_LUA_TNIL)
			{
				lua_pushnumber(L, r_lua_objlen(RL, -1));
				r_lua_pop(RL, 1);
			}
			else
			{
#if DebugMetaMethods
				printf("Table not Reffed for Len MetaMethod Ref: %d\n", Ref);
#endif
				lua_pushnumber(L, 0);
			}
			return 1;
		}
	}

	namespace CustomFunctions
	{
		int SetNDM(lua_State* L)
		{
#if FunctionNames
			printf("%s Called\n", __FUNCTION__);
#endif
			const uint32_t To = lua_tonumber(L, 1);
			uint32_t NDM = (DataModel - x_InstanceClass) + x_Networked;//Offsetsssss so store them somewhere better
			uint32_t Old = *reinterpret_cast<uint32_t*>(NDM);
			*reinterpret_cast<uint32_t*>(NDM) = To;
			lua_pushnumber(L, Old);
			return 1;
		}

		int SetTrustCheck(lua_State* L)
		{
#if FunctionNames
			printf("%s Called\n", __FUNCTION__);
#endif
			const char* URL = lua_tostring(L, 1);
			Bypasses::TrustCheck::SetURLs(URL);
			return 0;
		}

		//Real Globals Metamethods
		int GIndex(lua_State* L)
		{
#if FunctionNames
			printf("%s Called\n", __FUNCTION__);
#endif
			lua_xmove(L, GL, 1);
			lua_gettable(GL, LUA_GLOBALSINDEX);
			lua_xmove(GL, L, 1);
			return 1;
	}

		int GNewIndex(lua_State* L)
		{
#if FunctionNames
			printf("%s Called\n", __FUNCTION__);
#endif
			lua_xmove(L, GL, 2);
			lua_settable(GL, LUA_GLOBALSINDEX);
			return 0;
		}

		int GetGenv(lua_State* L)
		{
#if FunctionNames
			printf("%s Called\n", __FUNCTION__);
#endif
			lua_newtable(L);//Ghost tables for this :) I might need to add Roblox one to this? Combination?
			lua_newtable(L);//Add Len Metamethod
			lua_pushcclosure(L, GIndex, 0);
			lua_setfield(L, -2, "__index");
			lua_pushcclosure(L, GNewIndex, 0);
			lua_setfield(L, -2, "__newindex");
			lua_setmetatable(L, -2);
			return 1;
		}

		int GetRenv(lua_State* L)
		{
			uintptr_t RL = ExtraSpace::GetRoot(L)->RL;
			Wrapping::Wrap(RL, L, LUA_GLOBALSINDEX);
			return 1;
		}

		int GetReg(lua_State* L)
		{
			uintptr_t RL = ExtraSpace::GetRoot(L)->RL;
			Wrapping::Wrap(RL, L, LUA_REGISTRYINDEX);
			return 1;
		}

		int ReadFile(lua_State* L)
		{
			const char* Name = lua_tostring(L, 1);
			std::string Dir = Tools::Files::GetDLLPath("\\workspace").append("\\" + std::string(Name));
			std::ifstream infile(Dir.c_str());
			if (!infile.good())
			{
				return luaL_error(L, "File Does Not Exist!");
			}
			lua_pushstring(L, Tools::Files::ReadFile(Dir).c_str());
			return 1;
		}

		int WriteFile(lua_State* L)
		{
			const char* Name = lua_tostring(L, 1);
			const char* Writing = lua_tostring(L, 2);
			Tools::Files::WriteFile(Tools::Files::GetDLLPath("\\workspace").append("\\" + std::string(Name)), Writing);
			return 0;
		}

		int setreadonly(lua_State* L)//Table, Bool
		{
			luaL_checktype(L, 1, LUA_TTABLE);
			luaL_checktype(L, 2, LUA_TBOOLEAN);
			if (!luaL_getmetafield(L, 1, "__key"))
			{
				index2adr(L, 1)->value.gc->h.readonly = lua_toboolean(L, 2);
			}
			else
			{
				lua_pop(L, 1);
				*reinterpret_cast<uint8_t*>(index2adr(L, 1)->value.gc->h.readonly) = lua_toboolean(L, 2);
			}
			return 0;
		}

		int isreadonly(lua_State* L)//Table
		{
			luaL_checkany(L, 1);
			if (!luaL_getmetafield(L, 1, "__key"))
			{
				lua_pushboolean(L, index2adr(L, 1)->value.gc->h.readonly);
			}
			else
			{
				lua_pop(L, 1);
				lua_pushboolean(L, *reinterpret_cast<uint8_t*>(index2adr(L, 1)->value.gc->h.readonly));
			}
			return 1;
		}

		int IsCClosure(lua_State* L)
		{
			luaL_checktype(L, 1, LUA_TFUNCTION);
			const bool IsR = RobloxFunc::CheckRobloxObject(L, 1);
			if (IsR)
			{
				uintptr_t RL = ExtraSpace::GetRoot(L)->RL;
				Wrapping::Wrap(L, RL, 1);
				lua_pushboolean(L, r_lua_iscfunction(RL, -1));
				r_lua_pop(RL, 1);
			}
			else
			{
				lua_pushboolean(L, lua_iscfunction(L, 1));
			}
			return 1;
		}

		int GetRegList(lua_State* L)
		{
			uintptr_t RL = ExtraSpace::GetRoot(L)->RL;
			r_lua_pushlightuserdata(RL, reinterpret_cast<void*>(aslr(x_RobloxInstancePushFunction)));//Roblox Push Function
			r_lua_rawget(RL, LUA_REGISTRYINDEX);
			Wrapping::Wrap(RL, L, -1);
			r_lua_pop(RL, 1);
			return 1;
		}

		int SetClipboard(lua_State* L)
		{
			std::size_t Length;
			const char* Setting = lua_tolstring(L, 1, &Length);
			if (OpenClipboard(NULL))
			{
				if (EmptyClipboard())
				{
					HGLOBAL hglbCopy = GlobalAlloc(GMEM_FIXED, Length + 1);
					if (hglbCopy == NULL)
					{
						CloseClipboard();
						return luaL_error(L, "Failed to SetClipboard");
					}
					memcpy(hglbCopy, Setting, Length + 1);
					if (!SetClipboardData(CF_TEXT, hglbCopy))
					{
						CloseClipboard();
						GlobalFree(hglbCopy);
						return luaL_error(L, "Failed to SetClipboard");
					}
					CloseClipboard();
				}
				else
				{
					return luaL_error(L, "Failed to SetClipboard");
				}
			}
			else
			{
				return luaL_error(L, "Failed to SetClipboard");
			}
			return 0;
		}

		int GetClipboard(lua_State* L)
		{
			std::size_t Length;
			const char* Setting = lua_tolstring(L, 1, &Length);
			if (OpenClipboard(NULL))
			{
				HANDLE Got = GetClipboardData(CF_TEXT);
				if (Got == nullptr)
				{
					CloseClipboard();
					return luaL_error(L, "Failed to GetClipboard");
				}

				char* pszText = static_cast<char*>(GlobalLock(Got));
				if (pszText == nullptr)
				{
					CloseClipboard();
					return luaL_error(L, "Failed to GetClipboard");
				}
				std::string text(pszText);
				GlobalUnlock(Got);
				CloseClipboard();
				lua_pushstring(L, text.c_str());
				return 1;
			}
			else
			{
				return luaL_error(L, "Failed to GetClipboard");
			}
		}

		//Mouse Functions

		int MoveMouseRel(lua_State* L)
		{
			int32_t X = lua_tonumber(L, 1);
			int32_t Y = lua_tonumber(L, 2);
			mouse_event(MOUSEEVENTF_MOVE, X, Y, 0, 0);
			return 0;
		}

		int Mouse1Click(lua_State* L)
		{
			mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
			return 0;
		}

		int Mouse1Down(lua_State* L)
		{
			mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
			return 0;
		}

		int Mouse1Up(lua_State* L)
		{
			mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
			return 0;
		}

		int Mouse2Click(lua_State* L)
		{
			mouse_event(MOUSEEVENTF_RIGHTDOWN | MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);
			return 0;
		}

		int Mouse2Down(lua_State* L)
		{
			mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, 0);
			return 0;
		}

		int Mouse2Up(lua_State* L)
		{
			mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);
			return 0;
		}

		int GetGC(lua_State* L)
		{
			int Index = 1;
			uintptr_t RL = ExtraSpace::GetRoot(L)->RL;
			uintptr_t GlobalState = GetGlobalState(RL);
			GCObject* GC_Objects = *reinterpret_cast<GCObject**>(GlobalState + x_rootgc);
			r_lua_newtable(RL);
			while (GC_Objects != nullptr)
			{
				const auto GC_Type = *reinterpret_cast<uint8_t*>((reinterpret_cast<uint32_t>(GC_Objects) + x_gchtt));
				if ((GC_Type == R_LUA_TTABLE || GC_Type == R_LUA_TFUNCTION || GC_Type == R_LUA_TUSERDATA))//Do this  && (*(BYTE*)((DWORD)GC_Objects + 5) & 3
				{
					RobloxFunc::r_lua_pushobject(RL, reinterpret_cast<uintptr_t>(GC_Objects), GC_Type);
					r_lua_rawseti(RL, -2, Index++);
				}

				GC_Objects = GC_Objects->gch.next;
			}
			Wrapping::Wrap(RL, L, -1);
			r_lua_pop(RL, 1);
			return 1;
		}

		int IsRobloxObject(lua_State* L)
		{
			lua_pushboolean(L, RobloxFunc::CheckRobloxObject(L, 1));
			return 1;
		}

		int CPrint(lua_State* L)
		{
			printf("%s\n", lua_tostring(L, 1));
			return 0;
		}

		int GetRawMetaTable(lua_State* L)
		{
			const bool RObject = RobloxFunc::CheckRobloxObject(L, 1);
			if (RObject)
			{
				uintptr_t RL = ExtraSpace::GetRoot(L)->RL;
				Wrapping::Wrap(L, RL, 1);
				if (r_lua_getmetatable(RL, -1))
				{
					Wrapping::Wrap(RL, L, -1);
					r_lua_pop(RL, 2);
				}
				else
				{
					r_lua_pop(RL, 1);
					return luaL_error(L, "Attempt to call getrawmetatable failed: Passed value does not have a metatable\n");
				}
			}
			else
			{
				if (!lua_getmetatable(L, 1))
				{
					return luaL_error(L, "Attempt to call getrawmetatable failed: Passed value does not have a metatable\n");
				}
			}
			return 1;
		}

		int UnLockModule(lua_State* L)
		{
			uintptr_t RL = ExtraSpace::GetRoot(L)->RL;
			Wrapping::Wrap(L, RL, 1);
			const uintptr_t Module = *reinterpret_cast<uintptr_t*>(r_lua_touserdata(RL, -1));
			*reinterpret_cast<int8_t*>(Module + x_UnLockModule) = 1;
			r_lua_pop(RL, 1);
			return 0;
		}

		int CheckCaller(lua_State* L)
		{
			uintptr_t Thread = ExtraSpace::GetRoot(L)->RLGiven;
			const uintptr_t Space = *reinterpret_cast<uintptr_t*>(Thread + x_Identity2);
			const uintptr_t Script = *reinterpret_cast<uintptr_t*>(Space + x_ScriptPtr);
			lua_pushboolean(L, !Script);
			return 1;
		}

		int FireClickDetector(lua_State* L)
		{
			uintptr_t RL = ExtraSpace::GetRoot(L)->RL;
			Wrapping::Wrap(L, RL, 1);
			const uintptr_t Click = *reinterpret_cast<uintptr_t*>(r_lua_touserdata(RL, -1));
			float Distance = 0;
			if (lua_gettop(L) > 1)
				Distance = lua_tonumber(L, 2);
			r_lua_getglobal(RL, "game");
			r_lua_getfield(RL, -1, "GetService");
			r_lua_insert(RL, -2);
			r_lua_pushstring(RL, "Players");
			r_lua_pcall(RL, 2, 1, 0);
			r_lua_getfield(RL, -1, "LocalPlayer");
			const uintptr_t LP = *reinterpret_cast<uintptr_t*>(r_lua_touserdata(RL, -1));
			r_fireclickdetector(Click, Distance, LP);
			r_lua_pop(RL, 2);
			return 0;
		}

		int GetNameCallMethod(lua_State* L)
		{
			uintptr_t RL = ExtraSpace::GetRoot(L)->RL;
			RobloxFunc::r_lua_pushobject(RL, *reinterpret_cast<uintptr_t*>(RL + x_namecall), R_LUA_TSTRING);
			Wrapping::Wrap(RL, L, -1);
			r_lua_pop(RL, 1);
			return 1;
		}

		int SetNameCallMethod(lua_State* L)
		{
			uintptr_t RL = ExtraSpace::GetRoot(L)->RL;
			*reinterpret_cast<uintptr_t*>(RL + x_namecall) = reinterpret_cast<uintptr_t>(index2adr(L, 1)->value.p);
			return 0;
		}

		int HookFunction(lua_State* L)
		{
			const bool IsR = RobloxFunc::CheckRobloxObject(L, 1);
			if (IsR)
			{
				auto Space = ExtraSpace::GetRoot(L);
				uintptr_t RL = Space->RL;
				r_lua_settop(RL, 0);
				const uintptr_t Key = reinterpret_cast<uintptr_t>(lua_topointer(L, 2));
				Wrapping::Wrap(L, RL, 1);
				Wrapping::Wrap(L, RL, 2);
				if (r_lua_iscfunction(RL, 1))
				{
					//Calc replacement Address
					const uintptr_t Replacement = GetAddress(reinterpret_cast<uintptr_t>(r_lua_topointer(RL, 2)));

					//Create Return
					uintptr_t Closure2 = reinterpret_cast<uintptr_t>(r_lua_topointer(RL, 1));
					uintptr_t Index2Adr = reinterpret_cast<uintptr_t>(r_index2adr(RL, -2));
					uintptr_t Old = GetAddress(Closure2);
					uint8_t nupvals = *reinterpret_cast<uint8_t*>(Closure2 + x_closurenups);

					for (int i = 1; i <= nupvals; ++i)
						RobloxFunc::r2_lua_getupvalue(RL, -2 + ((i - 1) * -1), i);
					r_lua_pushcclosure(RL, Old, reinterpret_cast<int>(GetName(Closure2)), nupvals, NULL);
					Wrapping::Wrap(RL, L, -1);
					r_lua_pop(RL, 1);

					//Change Primary Function Address
					SetAddress(Closure2, Replacement);

					//Handle UpVal Alternative
					r_lua_newtable(RL);
					r_lua_pushstring(RL, "__func");
					r_lua_pushnumber(RL, Key);
					r_lua_settable(RL, -3);
					r_lua_pushstring(RL, "__thread");
					r_lua_pushlightuserdata(RL, Space->Main);
					r_lua_settable(RL, -3);
					r_lua_setfenv(RL, -3);
					r_lua_pop(RL, 2);
					return 1;
				}
				return luaL_error(L, "LClosure support not added");
			}
			else
			{
				return luaL_error(L, "Vanilla Closures support not added");
			}
		}

		int SetFFlag(lua_State* L)
		{
			const std::string FlagName = lua_tostring(L, 1);
			const std::string SetTo = lua_tostring(L, 2);
			const bool Return = reinterpret_cast<int(__cdecl*)(const std::string&, const std::string&, int, bool)>(aslr(0x15A5DB0))(FlagName, SetTo, 1, true);//SetFFlag
			if (!Return)
			{
				lua_pushfstring(L, "%s FFlag does not exist!", FlagName.c_str());
				return lua_error(L);
			}
			return 0;
		}

		namespace DebugLib
		{

			int dGetUpValues(lua_State* L)
			{
				const bool IsR = RobloxFunc::CheckRobloxObject(L, 1);
				if (IsR)
				{
					uintptr_t RL = ExtraSpace::GetRoot(L)->RL;
					r_lua_newtable(RL);
					Wrapping::Wrap(L, RL, 1);
					TValue* Closure;
					const uintptr_t clo = reinterpret_cast<uintptr_t>(r_lua_topointer(RL, -1));
					uintptr_t Index2Adr = reinterpret_cast<uintptr_t>(r_index2adr(RL, -1));
					const uint8_t nupvals = *reinterpret_cast<uint8_t*>(clo + x_closurenups);//nupvals
					for (int i = 1; i <= nupvals; i++)
					{
						if (!RobloxFunc::r2_lua_getupvalue(RL, -1, i))
							break;
						r_lua_rawseti(RL, -3, i);//Add to table
					}
					r_lua_pop(RL, 1);
					Wrapping::Wrap(RL, L, -1);
					r_lua_pop(RL, 1);
				}
				else
				{
					lua_newtable(L);
					for (int i = 1; i <= 999; ++i)
					{
						if (!lua_getupvalue(L, 1, i))//When the index is incorrect it returns NULL so means we don't need to work out the amount of upvals
							break;
						lua_rawseti(L, -2, i);//Add to table
					}
				}
				return 1;
			}

			int dGetUpValue(lua_State* L)
			{
				const bool IsR = RobloxFunc::CheckRobloxObject(L, 1);
				const uint32_t Index = lua_tonumber(L, 2);
				if (IsR)
				{
					uintptr_t RL = ExtraSpace::GetRoot(L)->RL;
					Wrapping::Wrap(L, RL, 1);
					uintptr_t Index2Adr = reinterpret_cast<uintptr_t>(r_index2adr(RL, -1));
					RobloxFunc::r2_lua_getupvalue(RL, -1 , Index);
					Wrapping::Wrap(RL, L, -1);
					r_lua_pop(RL, 2);
				}
				else
				{
					if (!lua_getupvalue(L, 1, Index))
						lua_pushnil(L);
				}
				return 1;
			}

			int dSetUpValue(lua_State* L)
			{
				const bool IsR = RobloxFunc::CheckRobloxObject(L, 1);
				const uint32_t Index = lua_tonumber(L, 2);
				if (IsR)
				{
					uintptr_t RL = ExtraSpace::GetRoot(L)->RL;
					Wrapping::Wrap(L, RL, 1);
					Wrapping::Wrap(L, RL, 3);
					RobloxFunc::r2_lua_setupvalue(RL, -2, Index);
					r_lua_pop(RL, 1);
				}
				else
				{
					lua_setupvalue(L, 1, Index);
				}
				return 0;
			}

			int dGetConstant(lua_State* L)
			{
				const bool IsR = RobloxFunc::CheckRobloxObject(L, 1);
				const uint32_t Index = lua_tonumber(L, 2);
				if (IsR)
				{
					uintptr_t RL = ExtraSpace::GetRoot(L)->RL;
					Wrapping::Wrap(L, RL, 1);
					if (!r_lua_iscfunction(RL, -1)) 
					{
						if (RobloxFunc::r_lua_getconstant(RL, -1, Index))
						{
							Wrapping::Wrap(RL, L, -1);
							r_lua_pop(RL, 2);
						}
					}
					else
					{
						r_lua_pop(RL, 1);
						lua_pushnil(L);
					}
				}
				else
				{
					if (!lua_iscfunction(L, 1))
					{
						Proto* f = index2adr(L, 1)->value.gc->cl.l.p;
						TValue* Returning = &f->k[Index - 1];
						RobloxFunc::m_settvalue(L->top, Returning, true);
					}
					else
					{
						lua_pushnil(L);//Should really cause an error here
					}
				}
				return 1;
			}

			int dSetConstant(lua_State* L)
			{
				const bool IsR = RobloxFunc::CheckRobloxObject(L, 1);
				const uint32_t Index = lua_tonumber(L, 2);
				TValue* New = index2adr(L, 3);
				if (IsR)
				{
					uintptr_t RL = ExtraSpace::GetRoot(L)->RL;
					Wrapping::Wrap(L, RL, 1);
					if (!r_lua_iscfunction(RL, -1))
					{
						Wrapping::Wrap(L, RL, 3);
						RobloxFunc::r_lua_setconstant(RL, -2, Index);
					}
					r_lua_pop(RL, 1);
				}
				else
				{
					if (!lua_iscfunction(L, 1))
					{
						Proto* f = index2adr(L, 1)->value.gc->cl.l.p;
						TValue* Returning = &f->k[Index - 1];
						RobloxFunc::m_settvalue(Returning, New);
					}
				}
				return 0;
			}

			int dGetConstants(lua_State* L)
			{
				const bool IsR = RobloxFunc::CheckRobloxObject(L, 1);
				if (IsR)
				{
					uintptr_t RL = ExtraSpace::GetRoot(L)->RL;
					r_lua_newtable(RL);
					Wrapping::Wrap(L, RL, 1);
					if (!r_lua_iscfunction(RL, -1))
					{
						int counter = 1;
						while (true)//SizeK
						{
							if (!RobloxFunc::r_lua_getconstant(RL, -1, counter))
								break;
							r_lua_rawseti(RL, -3, counter);
							counter++;
						}
					}
					r_lua_pop(RL, 1);
					Wrapping::Wrap(RL, L, -1);
					r_lua_pop(RL, 1);
				}
				else
				{
					lua_newtable(L);
					if (!lua_iscfunction(L, 1))
					{
						Proto* f = index2adr(L, 1)->value.gc->cl.l.p;
						TValue* Returning = NULL;
						for (int i = 1; i <= f->sizek; i++)
						{
							Returning = &f->k[i - 1];
							RobloxFunc::m_settvalue(L->top, Returning, true);
							lua_rawseti(L, -2, i);
						}
					}
				}
				return 1;
			}

			int dGetProtos(lua_State* L)
			{
				const bool IsR = RobloxFunc::CheckRobloxObject(L, 1);
				if (IsR)
				{
					uintptr_t RL = ExtraSpace::GetRoot(L)->RL;
					Wrapping::Wrap(L, RL, 1);
					const uintptr_t clo = reinterpret_cast<uintptr_t>(r_lua_topointer(RL, -1));
					const uintptr_t f = GetProto(clo);
					r_lua_newtable(RL);
					const uint32_t SizeP = *reinterpret_cast<uint32_t*>(f + x_sizep);
					const uintptr_t* Nested = reinterpret_cast<uintptr_t*>(DeObf(f, x_p));
					for (int i = 0; i < SizeP; i++)
					{
						uintptr_t NewLClo = r_luaF_newlclosure(RL, *reinterpret_cast<uint8_t*>(Nested[i] + x_protonups), *reinterpret_cast<uintptr_t*>(clo + 16), Nested[i]);//I don't think these update but if they ever do I aplogise
						RobloxFunc::r_lua_pushobject(RL, NewLClo, R_LUA_TFUNCTION);
						r_lua_rawseti(RL, -2, i + 1);
					}
					Wrapping::Wrap(RL, L, -1);
					r_lua_pop(RL, 2);
					return 1;
				}
				else
				{
					LClosure Top = index2adr(L, 1)->value.gc->cl.l;
					Proto* f = Top.p;
					lua_newtable(L);
					Proto* Nested = NULL;
					for (int i = 0; i < f->sizep; ++i)
					{
						Nested = f->p[i];
						const lu_byte nup = Nested->nups;
						Closure* ncl = luaF_newLclosure(L, nup, Top.env);
						ncl->l.p = Nested;
						L->top->tt = LUA_TFUNCTION;
						L->top->value.gc = cast(GCObject*, ncl);
						(L->top)++;
						lua_rawseti(L, -2, i + 1);
					}
				}
				return 1;
			}

			int dGetProto(lua_State* L)
			{
				const bool IsR = RobloxFunc::CheckRobloxObject(L, 1);
				const uint32_t Index = lua_tonumber(L, 2);
				if (IsR)
				{
					uintptr_t RL = ExtraSpace::GetRoot(L)->RL;
					Wrapping::Wrap(L, RL, 1);
					uintptr_t clo = reinterpret_cast<uintptr_t>(r_lua_topointer(RL, -1));
					uintptr_t f = GetProto(clo);
					const uint32_t SizeP = *reinterpret_cast<uint32_t*>(f + x_sizep);
					const uintptr_t* Nested = reinterpret_cast<uintptr_t*>(DeObf(f, x_p));
					if (Index <= SizeP)
					{
						uintptr_t NewLClo = r_luaF_newlclosure(RL, *reinterpret_cast<uint8_t*>(Nested[Index - 1] + x_protonups), *reinterpret_cast<uintptr_t*>(clo + 16), Nested[ Index - 1]);//Do it
						RobloxFunc::r_lua_pushobject(RL, NewLClo, R_LUA_TFUNCTION);
						Wrapping::Wrap(RL, L, -1);
						r_lua_pop(RL, 2);
					}
					else
					{
						r_lua_pop(RL, 1);
						lua_pushnil(L);
					}
				}
				else
				{
					LClosure Top = index2adr(L, 1)->value.gc->cl.l;
					Proto* f = Top.p;
					if (Index <= f->sizep)
					{
						Proto* Nested = f->p[Index - 1];
						lu_byte nup = Nested->nups;
						Closure* ncl = luaF_newLclosure(L, nup, Top.env);
						ncl->l.p = Nested;
						L->top->tt = LUA_TFUNCTION;
						L->top->value.gc = cast(GCObject*, ncl);
						(L->top)++;
					}
					else
					{
						lua_pushnil(L);
					}
				}
				return 1;
			}

			int dGetRegistry(lua_State* L)
			{
				uintptr_t RL = ExtraSpace::GetRoot(L)->RL;
				Wrapping::Wrap(RL, L, LUA_REGISTRYINDEX);
				return 1;
			}
			void luaopen_rdebug(lua_State* L)
			{
				lua_getglobal(L, "debug");
				lua_pushcclosure(L, dGetUpValues, 0);
				lua_setfield(L, -2, "getupvalues");
				lua_pushcclosure(L, dGetUpValue, 0);
				lua_setfield(L, -2, "getupvalue");
				lua_pushcclosure(L, dSetUpValue, 0);
				lua_setfield(L, -2, "setupvalue");
				lua_pushcclosure(L, dGetConstant, 0);
				lua_setfield(L, -2, "getconstant");
				lua_pushcclosure(L, dSetConstant, 0);
				lua_setfield(L, -2, "setconstant");
				lua_pushcclosure(L, dGetConstants, 0);
				lua_setfield(L, -2, "getconstants");
				lua_pushcclosure(L, dGetProtos, 0);
				lua_setfield(L, -2, "getprotos");
				lua_pushcclosure(L, dGetProto, 0);
				lua_setfield(L, -2, "getproto");
				lua_pushcclosure(L, dGetRegistry, 0);
				lua_setfield(L, -2, "getregistry");
			}

		}

	}

	namespace RobloxFunc
	{

		void printCallStack(lua_State* thread)
		{
			std::stringstream stream;

			lua_Debug ar;
			if (lua_getstack(thread, 0, &ar))
			{
				bool moreStack = false;
				int level = 0;
				const int maxLevel = 12;
				while (level < maxLevel && lua_getstack(thread, level++, &ar)) 
				{
					if (lua_getinfo(thread, "nSlu", &ar))
					{
						if (ar.currentline >= 0 )
						{
							moreStack = true;
							char source[LUA_IDSIZE];
							luaO_chunkid(source, ar.source, LUA_IDSIZE);
							std::string messageLine;
							if (level == 1)
							{
								r_print(3, "Stack Begin");
							}
							char buff[100];
							if (ar.name)
								snprintf(buff, sizeof(buff), "Script '%s', Line %d - %s %s", source, ar.currentline, ar.namewhat, ar.name);
							else
								snprintf(buff, sizeof(buff), "Script '%s', Line %d", source, ar.currentline);
							std::string buffAsStdStr = buff;
							r_print(3, buffAsStdStr.c_str());
						}
					}
				}
				if (moreStack && level < maxLevel)
				{
					r_print(3, "Stack End");
				}
			}
		}

		void r_lua_pushobject(uintptr_t RL, uintptr_t Obj, uint8_t Type)
		{
			TValue*& Top = *reinterpret_cast<TValue**>(RL + x_Top);
			Top->tt = Type;
			Top->value.p = reinterpret_cast<void*>(Obj);
			(Top)++;
		}
		bool r2_lua_getupvalue(uintptr_t RL, int32_t Clo, int32_t Index)
		{
			TValue* Closure;
			uintptr_t closure = reinterpret_cast<uintptr_t>(r_lua_topointer(RL, Clo));
			if (!r_lua_iscfunction(RL, Clo))//Possible add check for index? Allowing return false
			{
				int UpValue = closure + 16 * (Index + 1);//Need to improve this after source is usable
				if (*(uint32_t*)(UpValue + 12) == R_LUA_TUPVALUE)//If Type of UpValue == Closure(I need to redo this function and make it look better so then this offset is obsolete)
					UpValue = *reinterpret_cast<uintptr_t*>(*reinterpret_cast<uintptr_t*>(UpValue) + 8);
				*&Closure = (TValue*)UpValue;
			}
			else
			{
				r_aux_upvalue(reinterpret_cast<uintptr_t>(r_index2adr(RL, Clo)), Index, reinterpret_cast<uintptr_t*>(&Closure));
			}
			TValue*& Top = *reinterpret_cast<TValue**>(RL + x_Top);
			m_settvalue(Top, Closure, true);
			return true;
		}

		bool r2_lua_setupvalue(uintptr_t RL, int32_t Clo, int32_t Index)
		{
			TValue* Closure;
			uintptr_t clo = reinterpret_cast<uintptr_t>(r_lua_topointer(RL, Clo));
			TValue* Last = reinterpret_cast<TValue*>(r_index2adr(RL, -1));
			if (!r_lua_iscfunction(RL, Clo))//Possible add check for index? Allowing return false
			{
				uintptr_t UpValue = clo + 16 * (Index + 1);//Need to improve this after source is usable
				if (*reinterpret_cast<uint32_t*>(UpValue + 12) == R_LUA_TUPVALUE)//If Type of UpValue == Closure(I need to redo this function and make it look better so then this offset is obsolete)
					UpValue = *reinterpret_cast<uintptr_t*>(*reinterpret_cast<uintptr_t*>(UpValue) + 8);
				*&Closure = (TValue*)UpValue;
				m_settvalue(Closure, Last);
			}
			else
			{
				r_aux_upvalue(reinterpret_cast<uintptr_t>(r_index2adr(RL, Clo)), Index, reinterpret_cast<uintptr_t*>(&Closure));
				m_settvalue(Closure, Last);
			}
			r_lua_pop(RL, 1);
			return true;
		}

		bool r_lua_getconstant(uintptr_t RL, int32_t Clo, int32_t Index)
		{
			uintptr_t clo = reinterpret_cast<uintptr_t>(r_lua_topointer(RL, Clo));
			uintptr_t P = GetProto(clo);
			int Place = Index - 1;
			const uint32_t SizeK = *reinterpret_cast<uint32_t*>(P + x_sizek);
			if (Place < SizeK)//SizeK
			{
				TValue* Constant = reinterpret_cast<TValue*>(DeObf(P, x_k));
				TValue*& Top = *reinterpret_cast<TValue**>((RL + x_Top));
				m_settvalue(Top, &Constant[Place], true);
				return true;
			}
			return false;
		}

		bool r_lua_setconstant(uintptr_t RL, int32_t Clo, int32_t Index)
		{
			TValue* To = reinterpret_cast<TValue*>(r_index2adr(RL, -1));
			uintptr_t clo = reinterpret_cast<uintptr_t>(r_lua_topointer(RL, Clo));
			uintptr_t P = GetProto(clo);
			int Place = Index - 1;
			const uint32_t SizeK = *reinterpret_cast<uint32_t*>(P + x_sizek);
			if (Place < SizeK)//SizeK
			{
				TValue* Constant = reinterpret_cast<TValue*>(DeObf(P, x_k));
				(&Constant[Place])->tt = To->tt;
				(&Constant[Place])->value = To->value;
				return true;
			}
			return false;
		}
#if FullyAutoUpdatingCallCheck
		void r2_lua_pushcclosure(uintptr_t RL, uintptr_t Function, uint32_t UpVals, bool Break)
		{
			uintptr_t Pushing = Function;
			if (!Break)
				Pushing = Bypasses::CallCheck::FindFreeBreakPoint(Function);
			r_lua_pushcclosure(RL, Pushing, (int)"VClosure", UpVals, NULL);
		}
#else
		void r2_lua_pushcclosure(uintptr_t RL, uintptr_t Function, uint32_t UpVals)
		{
			r_lua_pushnumber(RL, Function);
			r_lua_pushcclosure(RL, Bypasses::CallCheck::HookLocation, (int)"VClosure", UpVals + 1, NULL);
		}
#endif
		void m_settvalue(TValue*& First, TValue* New, bool top)
		{
			First->tt = New->tt;
			First->value = New->value;
			if (top)
				(First)++;
		}

		bool CheckRobloxObject(lua_State* L, const int32_t idx)
		{
			const uint8_t Type = lua_type(L, idx);
			switch (Type)
			{
			case LUA_TFUNCTION:
			{
				if (reinterpret_cast<uintptr_t>(lua_tocfunction(L, idx)) == reinterpret_cast<uintptr_t>(FunctionHandling::RobloxCall))
				{
					return true;
				}
				else
				{
					return false;
				}
			}
			case LUA_TTABLE:
			{
				if (luaL_getmetafield(L, idx, "__key"))
				{
					lua_pop(L, 1);
					return true;
				}
				else
				{
					return false;
				}
			}
			case LUA_TUSERDATA:
			{
				uintptr_t RL = ExtraSpace::GetRoot(L)->RL;
				Wrapping::Wrap(L, RL, idx);
				if (r_luaL_getmetafield(RL, -1, "__type"))
				{
					r_lua_pop(RL, 2);
					return true;
				}
				else
				{
					r_lua_pop(RL, 1);
					return false;
				}
			}
			case LUA_TTHREAD:
			{
				if (ExtraSpace::GetRoot(L)->RLRef = NULL)
					return true;
				else
					return false;
			}
			default:
			{
				return false;
			}
			}
		}
		
	}

}

int r_luaL_getmetafield(uintptr_t RL, int obj, const char* event)
{
	if (!r_lua_getmetatable(RL, obj))  /* no metatable? */
		return 0;
	r_lua_pushstring(RL, event);
	r_lua_rawget(RL, -2);
	if (r_lua_type(RL, -1) == R_LUA_TNIL) {
		r_lua_pop(RL, 2);  /* remove metatable and metafield */
		return 0;
	}
	else {
		r_lua_remove(RL, -2);  /* remove only metatable */
		return 1;
	}
}

TValue* index2adr(lua_State* L, int idx) 
{
	if (idx > 0) {
		TValue* o = L->base + (idx - 1);
		api_check(L, idx <= L->ci->top - L->base);
		if (o >= L->top) return cast(TValue*, luaO_nilobject);
		else return o;
	}
	else if (idx > LUA_REGISTRYINDEX) {
		api_check(L, idx != 0 && -idx <= L->top - L->base);
		return L->top + idx;
	}
	else switch (idx) {  /* pseudo-indices */
	case LUA_REGISTRYINDEX: return registry(L);
	case LUA_ENVIRONINDEX: {
		Closure* func = curr_func(L);
		sethvalue(L, &L->env, func->c.env);
		return &L->env;
	}
	case LUA_GLOBALSINDEX: return gt(L);
	default: {
		Closure* func = curr_func(L);
		idx = LUA_GLOBALSINDEX - idx;
		return (idx <= func->c.nupvalues)
			? &func->c.upvalue[idx - 1]
			: cast(TValue*, luaO_nilobject);
	}
	}
}
