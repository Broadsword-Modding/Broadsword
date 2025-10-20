#include "../../ModAPI/Broadsword.hpp"
#include "../../Engine/SDK/Generated/SDK.hpp"
#include "../../Engine/SDK/Generated/SDK/Arena_Cutting_Map_classes.hpp"
#include "../../Engine/SDK/Generated/SDK/ModularWeaponBP_classes.hpp"
#include "../../Engine/SDK/Generated/SDK/BP_Armor_Master_classes.hpp"

using namespace Broadsword;

namespace GameConstants {
    constexpr float DEFAULT_HEALTH = 100.0f;
    constexpr float DEFAULT_PAIN = 0.0f;
    constexpr float DEFAULT_PLAYER_SPEED = 100.0f;
    constexpr float DEFAULT_MUSCLE_POWER = 25.0f;
    constexpr float DEFAULT_GRAB_FORCE = 60.0f;
    constexpr float DEFAULT_HANDS_RIGIDITY = 2.0f;
    constexpr float DEFAULT_ALL_BODY_TONUS = 1.0f;
    constexpr float FULL_TONUS = 1.0f;
    constexpr float DEFAULT_TIME_DILATION = 1.0f;
    constexpr float DEFAULT_GRAVITY = -980.0f;
    constexpr float GET_UP_RATE = 100.0f;
    constexpr float MAX_DISTANCE = 100000.0f;
    constexpr float MIN_HEALTH = 0.0f;
}

// Helper function to apply no pain effect
void ApplyNoPainEffect(SDK::AWillie_BP_C* willie) {
    if (!willie) return;

    willie->Health = GameConstants::DEFAULT_HEALTH;
    willie->Neck_Health = GameConstants::DEFAULT_HEALTH;
    willie->Head_Health = GameConstants::DEFAULT_HEALTH;
    willie->Body_Upper_Health = GameConstants::DEFAULT_HEALTH;
    willie->Body_Lower_Health = GameConstants::DEFAULT_HEALTH;
    willie->Arm_R_Health = GameConstants::DEFAULT_HEALTH;
    willie->Arm_L_Health = GameConstants::DEFAULT_HEALTH;
    willie->Leg_R_Health = GameConstants::DEFAULT_HEALTH;
    willie->Leg_L_Health = GameConstants::DEFAULT_HEALTH;
    willie->Head_Health__Crush_ = GameConstants::DEFAULT_HEALTH;

    willie->Pain_Lower_Body = GameConstants::DEFAULT_PAIN;
    willie->Pain_Upper_Body = GameConstants::DEFAULT_PAIN;
    willie->Pain_Neck = GameConstants::DEFAULT_PAIN;
    willie->Pain_Head = GameConstants::DEFAULT_PAIN;
    willie->Pain_Arm_R = GameConstants::DEFAULT_PAIN;
    willie->Pain_Arm_L = GameConstants::DEFAULT_PAIN;
    willie->Pain_Leg_R = GameConstants::DEFAULT_PAIN;
    willie->Pain_Leg_L = GameConstants::DEFAULT_PAIN;
    willie->Pain = GameConstants::DEFAULT_PAIN;
    willie->Pain_L_Arm_Alpha = GameConstants::DEFAULT_PAIN;
    willie->Pain_R_Arm_Alpha = GameConstants::DEFAULT_PAIN;
    willie->Pain_Shock = GameConstants::DEFAULT_PAIN;
    willie->Current_Pain_Threshold = GameConstants::DEFAULT_PAIN;
    willie->Pain_Grab_Rate = GameConstants::DEFAULT_PAIN;
    willie->Pain_Shock_Rate = GameConstants::DEFAULT_PAIN;
    willie->Pain_Shock_Interp = GameConstants::DEFAULT_PAIN;
    willie->Sustained_Damage = GameConstants::DEFAULT_PAIN;
}

// Helper to iterate Willies in radius
template<typename Func>
void ForEachWillieInRadius(SDK::UWorld* world, SDK::AWillie_BP_C* player, float radius, Func&& func) {
    if (!world) return;

    SDK::TArray<SDK::AActor*> actors;
    SDK::UGameplayStatics::GetAllActorsOfClass(world, SDK::AWillie_BP_C::StaticClass(), &actors);

    for (auto* actor : actors) {
        auto* willie = static_cast<SDK::AWillie_BP_C*>(actor);
        if (willie == player || !willie) continue;

        if (radius == GameConstants::MAX_DISTANCE || (player && player->GetDistanceTo(willie) <= radius)) {
            func(willie);
        }
    }
}

// Helper to iterate all Willies
template<typename Func>
void ForEachWillie(SDK::UWorld* world, SDK::AWillie_BP_C* player, Func&& func) {
    ForEachWillieInRadius(world, player, GameConstants::MAX_DISTANCE, std::forward<Func>(func));
}

class EnhancerMod : public Broadsword::Mod {
public:
    // Player features config
    [[configurable]] bool m_InfiniteStamina = false;
    [[configurable]] bool m_InfiniteConsciousness = false;
    [[configurable]] float m_JumpForce = 5000.0f;
    [[configurable]] float m_DashForce = 7000.0f;
    [[configurable]] bool m_SpeedEnabled = false;
    [[configurable]] float m_SpeedRunMultiplier = 1.0f;
    [[configurable]] float m_SpeedWalkMultiplier = 1.0f;
    [[configurable]] bool m_StrengthEnabled = false;
    [[configurable]] float m_StrengthMultiplier = 1.0f;
    [[configurable]] float m_GrabForceMultiplier = 1.0f;
    [[configurable]] float m_HandsRigidityMultiplier = 1.0f;
    [[configurable]] float m_BodyTonusMultiplier = 1.0f;
    [[configurable]] bool m_BodyTonusNoWeakening = false;
    [[configurable]] bool m_NoKickCooldown = false;
    [[configurable]] bool m_Invulnerable = false;
    [[configurable]] bool m_NoPain = false;

    // World features config
    [[configurable]] float m_SlowMotionSpeed = 0.4f;
    [[configurable]] float m_CustomGravity = 0.0f;
    [[configurable]] float m_KillRadius = 1000.0f;
    [[configurable]] bool m_SnapNeck = false;
    [[configurable]] float m_ToggleAIRadius = 1000.0f;
    [[configurable]] bool m_DestroyDeadOnly = true;
    [[configurable]] bool m_DestroyDisintegrate = true;
    [[configurable]] float m_ClearBloodAmount = 0.1f;
    [[configurable]] float m_ClearObjectsRadius = 1000.0f;

    // NPC Spawner config
    [[configurable]] int m_SelectedNPCType = 0;
    [[configurable]] float m_NPCSpawnDistanceForward = 200.0f;
    [[configurable]] float m_NPCSpawnDistanceUp = 0.0f;
    [[configurable]] float m_NPCSpawnScale = 1.0f;
    [[configurable]] bool m_NPCBodyguard = false;
    [[configurable]] bool m_NPCSnapToGround = true;
    [[configurable]] int m_NPCTeam = 0;

    // Item Spawner config
    [[configurable]] int m_SelectedItemCategory = 0;
    [[configurable]] int m_SelectedItemSubcategory = 0;
    [[configurable]] int m_SelectedItemIndex = 0;
    [[configurable]] float m_ItemSpawnDistanceForward = 150.0f;
    [[configurable]] float m_ItemSpawnDistanceUp = 50.0f;
    [[configurable]] float m_ItemSpawnScale = 1.0f;
    [[configurable]] bool m_ItemSnapToGround = true;

    Broadsword::ModInfo GetInfo() const override {
        return {
            .Name = "Half Sword Enhancer",
            .Version = "2.0.0",
            .Author = "Broadsword Team",
            .Description = "Comprehensive gameplay enhancement mod with player modifications, world controls, and entity spawning"
        };
    }

    void OnRegister(Broadsword::ModContext& ctx) override {
        ctx.log.Info("Half Sword Enhancer initializing...");

        // Subscribe to frame events
        m_FrameSubscriptionId = ctx.events.Subscribe<Broadsword::OnFrameEvent>([this](auto& e) {
            OnFrame(e.frame);
        });

        // Subscribe to player spawn events
        m_PlayerSpawnSubscriptionId = ctx.events.Subscribe<Broadsword::OnPlayerSpawnedEvent>([this](auto& e) {
            m_Player = e.player;
            ctx.log.Info("Player spawned and tracked");
        });

        // Register invulnerability hook
        m_DamageHookId = ctx.hooks.Hook(
            "Function HalfSwordUE5.Willie_BP_C.TakeDamage",
            [this](SDK::UObject*, SDK::UFunction*, void*) -> bool {
                return !m_Invulnerable;
            }
        );

        ctx.log.Info("Half Sword Enhancer registered successfully");
    }

    void OnUnregister() override {
        // Cleanup hooks
        if (m_DamageHookId != 0) {
            // Hook cleanup handled by framework
        }

        m_Player = nullptr;
    }

private:
    void OnFrame(Broadsword::Frame& frame) {
        // Update player reference if needed
        if (!m_Player) {
            auto playerResult = frame.world.GetPlayer();
            if (playerResult) {
                m_Player = playerResult.Value();
            }
        }

        // Apply frame-based player modifications
        ApplyPlayerModifications(frame);

        // Render UI
        RenderUI(frame);
    }

    void ApplyPlayerModifications(Broadsword::Frame& frame) {
        if (!m_Player) return;

        // Infinite Stamina
        if (m_InfiniteStamina) {
            m_Player->Stamina = GameConstants::DEFAULT_HEALTH;
        }

        // Infinite Consciousness
        if (m_InfiniteConsciousness) {
            m_Player->Consciousness = GameConstants::DEFAULT_HEALTH;
            m_Player->Consciousness_Cap = GameConstants::DEFAULT_HEALTH;
            m_Player->Consciousness_2__Legs_ = GameConstants::DEFAULT_HEALTH;
        }

        // Speed Multiplier
        if (m_SpeedEnabled) {
            m_Player->Running_Speed_Rate = GameConstants::DEFAULT_PLAYER_SPEED * m_SpeedRunMultiplier;
            m_Player->Walk_Speed_Rate_Run = GameConstants::DEFAULT_PLAYER_SPEED * m_SpeedWalkMultiplier;
        }

        // Strength Multiplier
        if (m_StrengthEnabled) {
            m_Player->Muscle_Power = GameConstants::DEFAULT_MUSCLE_POWER * m_StrengthMultiplier;
            m_Player->R_Grab_Force_Limit = GameConstants::DEFAULT_GRAB_FORCE * m_GrabForceMultiplier;
            m_Player->L_Grab_Force_Limit = GameConstants::DEFAULT_GRAB_FORCE * m_GrabForceMultiplier;
            m_Player->Hands_Rigidity__Gauntlets_ = GameConstants::DEFAULT_HANDS_RIGIDITY * m_HandsRigidityMultiplier;
        }

        // Body Tonus
        m_Player->All_Body_Tonus = GameConstants::DEFAULT_ALL_BODY_TONUS * m_BodyTonusMultiplier;
        if (m_BodyTonusNoWeakening) {
            m_Player->Head_Tonus = GameConstants::FULL_TONUS;
            m_Player->Arm_L_Tonus = GameConstants::FULL_TONUS;
            m_Player->Arm_R_Tonus = GameConstants::FULL_TONUS;
            m_Player->Leg_L_Tonus = GameConstants::FULL_TONUS;
            m_Player->Leg_R_Tonus = GameConstants::FULL_TONUS;
        }

        // No Kick Cooldown
        if (m_NoKickCooldown) {
            m_Player->Kick_Cooldown = false;
        }

        // No Pain
        if (m_NoPain) {
            ApplyNoPainEffect(m_Player);
        }
    }

    void RenderUI(Broadsword::Frame& frame) {
        if (frame.ui.BeginTabBar("Enhancer")) {
            if (frame.ui.BeginTabItem("Player")) {
                RenderPlayerTab(frame);
                frame.ui.EndTabItem();
            }

            if (frame.ui.BeginTabItem("World")) {
                RenderWorldTab(frame);
                frame.ui.EndTabItem();
            }

            if (frame.ui.BeginTabItem("NPC Spawner")) {
                RenderNPCSpawnerTab(frame);
                frame.ui.EndTabItem();
            }

            if (frame.ui.BeginTabItem("Item Spawner")) {
                RenderItemSpawnerTab(frame);
                frame.ui.EndTabItem();
            }

            frame.ui.EndTabBar();
        }
    }

    void RenderPlayerTab(Broadsword::Frame& frame) {
        frame.ui.Text("Player Modifications");
        frame.ui.Separator();

        // Basic toggles
        frame.ui.Checkbox("Infinite Stamina", &m_InfiniteStamina);
        frame.ui.Checkbox("Infinite Consciousness", &m_InfiniteConsciousness);
        frame.ui.Checkbox("Invulnerability", &m_Invulnerable);
        frame.ui.Checkbox("No Pain", &m_NoPain);
        frame.ui.Checkbox("No Kick Cooldown", &m_NoKickCooldown);

        frame.ui.Separator();

        // Save Loadout button
        if (frame.ui.Button("Save Loadout")) {
            if (m_Player) {
                m_Player->Save_Loadout();
                frame.log.Info("Loadout saved");
            } else {
                frame.log.Warn("Cannot save loadout: player not found");
            }
        }

        // Jump button with force slider
        frame.ui.SliderFloat("Jump Force", &m_JumpForce, 1000.0f, 10000.0f);
        if (frame.ui.Button("Jump")) {
            if (m_Player) {
                m_Player->Mesh->AddImpulse(SDK::FVector(0.0f, 0.0f, m_JumpForce), SDK::FName(), true);
                frame.log.Info("Jumped with force {}", m_JumpForce);
            }
        }

        // Dash button with force slider
        frame.ui.SliderFloat("Dash Force", &m_DashForce, 1000.0f, 10000.0f);
        if (frame.ui.Button("Dash")) {
            if (m_Player) {
                SDK::FVector forwardVector = m_Player->GetActorForwardVector();
                m_Player->Mesh->AddImpulse(forwardVector * m_DashForce, SDK::FName(), true);
                frame.log.Info("Dashed with force {}", m_DashForce);
            }
        }

        // Get Up button
        if (frame.ui.Button("Get Up")) {
            if (m_Player) {
                m_Player->Get_Up_Rate = GameConstants::GET_UP_RATE;
                frame.log.Info("Forcing player to get up");
            }
        }

        // Ragdoll button
        if (frame.ui.Button("Ragdoll")) {
            if (m_Player) {
                m_Player->All_Body_Tonus = GameConstants::DEFAULT_PAIN;
                frame.log.Info("Player ragdolled");
            }
        }

        frame.ui.Separator();

        // Speed Multiplier
        frame.ui.Checkbox("Enable Speed Multiplier", &m_SpeedEnabled);
        if (m_SpeedEnabled) {
            frame.ui.SliderFloat("Run Speed Multiplier", &m_SpeedRunMultiplier, 1.0f, 100.0f);
            frame.ui.SliderFloat("Walk Speed Multiplier", &m_SpeedWalkMultiplier, 1.0f, 100.0f);
        }

        frame.ui.Separator();

        // Strength Multiplier
        frame.ui.Checkbox("Enable Strength Multiplier", &m_StrengthEnabled);
        if (m_StrengthEnabled) {
            frame.ui.SliderFloat("Strength Multiplier", &m_StrengthMultiplier, 1.0f, 10.0f);
            frame.ui.SliderFloat("Grab Force Multiplier", &m_GrabForceMultiplier, 1.0f, 10.0f);
            frame.ui.SliderFloat("Hands Rigidity Multiplier", &m_HandsRigidityMultiplier, 1.0f, 10.0f);
        }

        frame.ui.Separator();

        // Body Tonus
        frame.ui.SliderFloat("Body Tonus Multiplier", &m_BodyTonusMultiplier, 1.0f, 10.0f);
        frame.ui.Checkbox("No Body Weakening", &m_BodyTonusNoWeakening);

        frame.ui.Separator();

        // Possess Nearest Willie
        if (frame.ui.Button("Possess Nearest Willie")) {
            PossessNearestWillie(frame);
        }
    }

    void PossessNearestWillie(Broadsword::Frame& frame) {
        auto worldResult = frame.world.GetWorld();
        if (!worldResult) {
            frame.log.Warn("Cannot possess: world not loaded");
            return;
        }
        SDK::UWorld* world = worldResult.Value();

        auto controllerResult = frame.world.GetPlayerController();
        if (!controllerResult) {
            frame.log.Warn("Cannot possess: controller not found");
            return;
        }
        SDK::APlayerController* controller = controllerResult.Value();

        SDK::APawn* currentPawn = controller->K2_GetPawn();

        if (m_PossessedWillie && currentPawn != m_PossessedWillie) {
            m_PrevAIController = nullptr;
            m_OriginalPawn = nullptr;
            m_PossessedWillie = nullptr;
        }

        if (!m_PossessedWillie) {
            m_OriginalPawn = currentPawn;
            SDK::AWillie_BP_C* nearest = nullptr;
            float minDist = GameConstants::MAX_DISTANCE;

            ForEachWillie(world, m_Player, [&](SDK::AWillie_BP_C* willie) {
                if (!m_Player) return;
                const float dist = m_Player->GetDistanceTo(willie);
                if (dist < minDist) {
                    minDist = dist;
                    nearest = willie;
                }
            });

            if (!nearest) {
                frame.log.Warn("No NPCs found to possess");
                return;
            }

            auto* nearestController = nearest->GetController();
            if (nearestController && !nearest->IsA(SDK::AWillie_BP_NoBrain_C::StaticClass())) {
                m_PrevAIController = static_cast<SDK::AAIController*>(nearestController);
                m_PrevAIController->SetActorTickEnabled(false);
            }

            controller->Possess(nearest);
            nearest->Player = true;
            m_PossessedWillie = nearest;
            m_Player = nearest;
            frame.log.Info("Possessed nearest NPC");
        } else {
            auto* williePawn = static_cast<SDK::AWillie_BP_C*>(currentPawn);
            controller->Possess(m_OriginalPawn);
            williePawn->Player = false;

            if (m_PrevAIController) {
                m_PrevAIController->Possess(williePawn);
                m_PrevAIController->SetActorTickEnabled(true);
                m_PrevAIController = nullptr;
            }

            m_PossessedWillie = nullptr;
            m_Player = static_cast<SDK::AWillie_BP_C*>(m_OriginalPawn);
            m_OriginalPawn = nullptr;
            frame.log.Info("Returned to original character");
        }
    }

    void RenderWorldTab(Broadsword::Frame& frame) {
        frame.ui.Text("World Modifications");
        frame.ui.Separator();

        auto worldResult = frame.world.GetWorld();
        if (!worldResult) {
            frame.ui.TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "World not loaded");
            return;
        }
        SDK::UWorld* world = worldResult.Value();

        // Slow Motion
        frame.ui.SliderFloat("Slow Motion Speed", &m_SlowMotionSpeed, 0.01f, 0.99f);
        if (frame.ui.Button("Toggle Slow Motion")) {
            auto* settings = world->K2_GetWorldSettings();
            if (settings) {
                float current = settings->TimeDilation;
                settings->TimeDilation = (current == GameConstants::DEFAULT_TIME_DILATION)
                    ? m_SlowMotionSpeed
                    : GameConstants::DEFAULT_TIME_DILATION;
                frame.log.Info("Time dilation set to {}", settings->TimeDilation);
            }
        }

        frame.ui.Separator();

        // Custom Gravity
        frame.ui.SliderFloat("Custom Gravity", &m_CustomGravity, -3000.0f, 3000.0f);
        if (frame.ui.Button("Toggle Custom Gravity")) {
            auto* settings = world->K2_GetWorldSettings();
            if (settings) {
                settings->bWorldGravitySet = true;
                float current = settings->WorldGravityZ;
                settings->WorldGravityZ = (current == GameConstants::DEFAULT_GRAVITY)
                    ? m_CustomGravity
                    : GameConstants::DEFAULT_GRAVITY;
                frame.log.Info("Gravity set to {}", settings->WorldGravityZ);
            }
        }

        frame.ui.Separator();

        // Kill All Enemies
        frame.ui.SliderFloat("Kill Radius", &m_KillRadius, 50.0f, 5000.0f);
        frame.ui.Checkbox("Snap Neck", &m_SnapNeck);
        if (frame.ui.Button("Kill All Enemies")) {
            int killCount = 0;
            ForEachWillieInRadius(world, m_Player, m_KillRadius, [&](SDK::AWillie_BP_C* willie) {
                if (m_SnapNeck) {
                    willie->Snap_Neck();
                } else {
                    willie->Death();
                }
                killCount++;
            });
            frame.log.Info("Killed {} enemies within {} units", killCount, m_KillRadius);
        }

        frame.ui.Separator();

        // Toggle Enemy AI
        frame.ui.SliderFloat("AI Toggle Radius", &m_ToggleAIRadius, 50.0f, 5000.0f);
        if (frame.ui.Button("Toggle Enemy AI")) {
            bool newTickEnabled = false;
            bool computed = false;
            int affectedCount = 0;

            ForEachWillieInRadius(world, m_Player, m_ToggleAIRadius, [&](SDK::AWillie_BP_C* willie) {
                auto* controller = static_cast<SDK::AAIController*>(willie->GetController());
                if (controller) {
                    if (!computed) {
                        newTickEnabled = !controller->IsActorTickEnabled();
                        computed = true;
                    }
                    controller->SetActorTickEnabled(newTickEnabled);
                    affectedCount++;
                }
            });
            frame.log.Info("Toggled AI for {} enemies (enabled: {})", affectedCount, newTickEnabled);
        }

        frame.ui.Separator();

        // Destroy All Willies
        frame.ui.Checkbox("Destroy Dead Only", &m_DestroyDeadOnly);
        frame.ui.Checkbox("Disintegrate Effect", &m_DestroyDisintegrate);
        if (frame.ui.Button("Destroy All Willies")) {
            int destroyCount = 0;
            ForEachWillie(world, m_Player, [&](SDK::AWillie_BP_C* willie) {
                if (!m_DestroyDeadOnly || willie->Health <= GameConstants::MIN_HEALTH) {
                    if (m_DestroyDisintegrate) {
                        willie->Disintegrate_and_drop_armor(true);
                    } else {
                        willie->K2_DestroyActor();
                    }
                    destroyCount++;
                }
            });
            frame.log.Info("Destroyed {} NPCs", destroyCount);
        }

        frame.ui.Separator();

        // Clear Blood
        frame.ui.SliderFloat("Blood Clear Amount", &m_ClearBloodAmount, 0.0f, 1.0f);
        if (frame.ui.Button("Clear Blood")) {
            if (world->PersistentLevel && world->PersistentLevel->LevelScriptActor) {
                auto* arenaScript = static_cast<SDK::AArena_Cutting_Map_C*>(world->PersistentLevel->LevelScriptActor);
                if (arenaScript) {
                    arenaScript->Clean_Blood(m_ClearBloodAmount);
                    frame.log.Info("Cleared blood (amount: {})", m_ClearBloodAmount);
                } else {
                    frame.log.Warn("Not in arena - cannot clear blood");
                }
            }
        }

        frame.ui.Separator();

        // Clear Objects
        frame.ui.SliderFloat("Clear Objects Radius", &m_ClearObjectsRadius, 50.0f, 5000.0f);
        if (frame.ui.Button("Clear Objects")) {
            constexpr float MIN_DISTANCE_FROM_WILLIES = 100.0f;
            int clearedCount = 0;

            auto clearObjectsOfType = [&](SDK::UClass* objectClass) {
                SDK::TArray<SDK::AActor*> objects;
                SDK::UGameplayStatics::GetAllActorsOfClass(world, objectClass, &objects);

                for (auto* object : objects) {
                    bool isWithinRadius = false;
                    bool isFarFromAll = true;

                    ForEachWillie(world, nullptr, [&](SDK::AWillie_BP_C* willie) {
                        float distance = willie->GetDistanceTo(object);
                        if (m_Player && m_Player->GetDistanceTo(object) <= m_ClearObjectsRadius) {
                            isWithinRadius = true;
                        }
                        if (distance <= MIN_DISTANCE_FROM_WILLIES) {
                            isFarFromAll = false;
                        }
                    });

                    if (isWithinRadius && isFarFromAll) {
                        object->K2_DestroyActor();
                        clearedCount++;
                    }
                }
            };

            clearObjectsOfType(SDK::AModularWeaponBP_C::StaticClass());
            clearObjectsOfType(SDK::ABP_Armor_Master_C::StaticClass());

            frame.log.Info("Cleared {} objects within {} units", clearedCount, m_ClearObjectsRadius);
        }

        frame.ui.Separator();

        // Toggle Game Paused
        if (frame.ui.Button("Toggle Game Paused")) {
            bool isPaused = SDK::UGameplayStatics::IsGamePaused(world);
            SDK::UGameplayStatics::SetGamePaused(world, !isPaused);
            frame.log.Info("Game paused: {}", !isPaused);
        }
    }

    void RenderNPCSpawnerTab(Broadsword::Frame& frame) {
        frame.ui.Text("NPC Spawner");
        frame.ui.Separator();

        auto worldResult = frame.world.GetWorld();
        if (!worldResult) {
            frame.ui.TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "World not loaded");
            return;
        }
        SDK::UWorld* world = worldResult.Value();

        // NPC type selector
        static const char* npcTypes[] = {
            "Regular",
            "No Brain",
            "Boss 1",
            "Boss 2",
            "Boss 3",
            "Boss 4",
            "Boss 5",
            "Boss 6",
            "Boss 7",
            "Boss 8",
            "Boss 9 (Baron)"
        };
        static const char* npcClassPaths[] = {
            "/Game/Character/Blueprints/Willie_BP.Willie_BP_C",
            "/Game/Character/Blueprints/Willie_BP_NoBrain.Willie_BP_NoBrain_C",
            "/Game/Character/Blueprints/Unique/Bosses/Willie_BP_Boss_1.Willie_BP_Boss_1_C",
            "/Game/Character/Blueprints/Unique/Bosses/Willie_BP_Boss_2.Willie_BP_Boss_2_C",
            "/Game/Character/Blueprints/Unique/Bosses/Willie_BP_Boss_3.Willie_BP_Boss_3_C",
            "/Game/Character/Blueprints/Unique/Bosses/Willie_BP_Boss_4.Willie_BP_Boss_4_C",
            "/Game/Character/Blueprints/Unique/Bosses/Willie_BP_Boss_5.Willie_BP_Boss_5_C",
            "/Game/Character/Blueprints/Unique/Bosses/Willie_BP_Boss_6.Willie_BP_Boss_6_C",
            "/Game/Character/Blueprints/Unique/Bosses/Willie_BP_Boss_7.Willie_BP_Boss_7_C",
            "/Game/Character/Blueprints/Unique/Bosses/Willie_BP_Boss_8.Willie_BP_Boss_8_C",
            "/Game/Character/Blueprints/Unique/Bosses/Willie_BP_Boss_9_BARON.Willie_BP_Boss_9_BARON_C"
        };

        frame.ui.Combo("NPC Type", &m_SelectedNPCType, npcTypes, 11);

        frame.ui.Separator();

        // Spawn parameters
        frame.ui.SliderFloat("Distance Forward", &m_NPCSpawnDistanceForward, 100.0f, 500.0f);
        frame.ui.SliderFloat("Distance Up", &m_NPCSpawnDistanceUp, 0.0f, 300.0f);
        frame.ui.SliderFloat("Scale", &m_NPCSpawnScale, 0.1f, 4.0f);
        frame.ui.Checkbox("Bodyguard", &m_NPCBodyguard);
        frame.ui.Checkbox("Snap to Ground", &m_NPCSnapToGround);
        frame.ui.SliderInt("Team", &m_NPCTeam, 0, 9);

        frame.ui.Separator();

        // Spawn button
        if (frame.ui.Button("Spawn NPC")) {
            if (!m_Player) {
                frame.log.Warn("Cannot spawn NPC: player not found");
                return;
            }

            SDK::FTransform spawnTransform = m_Player->GetTransform();
            spawnTransform.Translation += m_Player->GetActorForwardVector() * m_NPCSpawnDistanceForward;
            spawnTransform.Translation.Z += m_NPCSpawnDistanceUp;
            spawnTransform.Scale3D = SDK::FVector(m_NPCSpawnScale, m_NPCSpawnScale, m_NPCSpawnScale);

            const char* className = npcClassPaths[m_SelectedNPCType];

            auto spawnResult = frame.world.SpawnActor(className, spawnTransform);
            if (spawnResult) {
                auto* npc = static_cast<SDK::AWillie_BP_C*>(spawnResult.Value());

                if (m_NPCBodyguard) {
                    constexpr int SPECIAL_TEAM_ID = 1337;
                    m_Player->Team_Int = SPECIAL_TEAM_ID;
                    npc->Team_Int = m_Player->Team_Int;
                    frame.log.Info("Spawned {} as bodyguard (team {})", npcTypes[m_SelectedNPCType], SPECIAL_TEAM_ID);
                } else {
                    npc->Team_Int = m_NPCTeam;
                    frame.log.Info("Spawned {} (team {})", npcTypes[m_SelectedNPCType], m_NPCTeam);
                }
            } else {
                frame.log.Error("Failed to spawn {}: {}", npcTypes[m_SelectedNPCType], ToString(spawnResult.Error()));
            }
        }
    }

    void RenderItemSpawnerTab(Broadsword::Frame& frame) {
        frame.ui.Text("Item Spawner");
        frame.ui.Separator();

        auto worldResult = frame.world.GetWorld();
        if (!worldResult) {
            frame.ui.TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "World not loaded");
            return;
        }
        SDK::UWorld* world = worldResult.Value();

        // Define item database (simplified - showing structure, full list would be very long)
        struct ItemInfo {
            const char* name;
            const char* path;
        };

        static const char* categories[] = {
            "Weapons", "Helmets", "Body Armor", "Arms", "Legs",
            "Hands", "Feet", "Neck", "Shoulders", "Waist", "Props"
        };

        static const char* weaponSubcategories[] = {
            "Swords", "Maces", "Axes", "Polearms", "Daggers", "Tools", "Shields", "Improvised"
        };

        // Category selector
        frame.ui.Combo("Category", &m_SelectedItemCategory, categories, 11);

        // Weapon subcategory (only for weapons)
        if (m_SelectedItemCategory == 0) {
            frame.ui.Combo("Subcategory", &m_SelectedItemSubcategory, weaponSubcategories, 8);
        }

        frame.ui.Separator();

        // Spawn parameters
        frame.ui.SliderFloat("Distance Forward", &m_ItemSpawnDistanceForward, 50.0f, 300.0f);
        frame.ui.SliderFloat("Distance Up", &m_ItemSpawnDistanceUp, 0.0f, 200.0f);
        frame.ui.SliderFloat("Scale", &m_ItemSpawnScale, 0.1f, 5.0f);
        frame.ui.Checkbox("Snap to Ground", &m_ItemSnapToGround);

        frame.ui.Separator();

        // For now, show placeholder since we need to implement the full item list
        frame.ui.Text("Item spawner structure ready");
        frame.ui.Text("Full item list implementation in progress...");
        frame.ui.TextWrapped("Note: Will include 249 items total - 96 weapons, 144 armor pieces, 9 props");
    }

    // State
    SDK::AWillie_BP_C* m_Player = nullptr;
    size_t m_FrameSubscriptionId = 0;
    size_t m_PlayerSpawnSubscriptionId = 0;
    size_t m_DamageHookId = 0;

    // Possession state
    SDK::AAIController* m_PrevAIController = nullptr;
    SDK::APawn* m_OriginalPawn = nullptr;
    SDK::AWillie_BP_C* m_PossessedWillie = nullptr;
};

// DLL exports
extern "C" {
    __declspec(dllexport) Broadsword::Mod* CreateMod() {
        return new EnhancerMod();
    }

    __declspec(dllexport) void DestroyMod(Broadsword::Mod* mod) {
        delete mod;
    }
}
