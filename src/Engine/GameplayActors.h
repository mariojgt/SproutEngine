#pragma once
#include "Actor.h"
#include "CoreComponents.h"

/**
 * Pawn - base class for objects that can be possessed by a controller
 * Similar to Unreal's APawn
 */
class Pawn : public Actor {
public:
    Pawn(World* world, const std::string& name = "Pawn");
    virtual ~Pawn() = default;

    // Pawn interface
    virtual void SetupPlayerInputComponent() {}
    virtual void PossessedBy(class Controller* newController);
    virtual void UnPossessed();

    class Controller* GetController() const { return controller; }

    // Movement
    virtual void AddMovementInput(const glm::vec3& direction, float scaleValue = 1.0f);

    // Static class info (for blueprint system)
    static std::string StaticClass() { return "Pawn"; }

protected:
    class Controller* controller = nullptr;
    glm::vec3 pendingMovementInput{0.0f};

    void TickMovement(float deltaTime);
};

/**
 * Character - pawn with a character movement component
 * Similar to Unreal's ACharacter
 */
class Character : public Pawn {
public:
    Character(World* world, const std::string& name = "Character");
    virtual ~Character() = default;

    void BeginPlay() override;
    void Tick(float deltaTime) override;

    // Character movement
    void MoveForward(float value);
    void MoveRight(float value);
    void Jump();

    // Components
    MeshRendererComponent* GetMeshComponent() const { return meshComponent; }
    CollisionComponent* GetCapsuleComponent() const { return capsuleComponent; }
    CameraComponent* GetCameraComponent() const { return cameraComponent; }

    // Character properties
    float WalkSpeed = 600.0f;
    float JumpVelocity = 420.0f;
    bool bCanJump = true;

    // Static class info
    static std::string StaticClass() { return "Character"; }

protected:
    MeshRendererComponent* meshComponent = nullptr;
    CollisionComponent* capsuleComponent = nullptr;
    CameraComponent* cameraComponent = nullptr;

    void SetupComponents();
};

/**
 * PlayerController - handles input and controls a pawn
 */
class Controller : public Actor {
public:
    Controller(World* world, const std::string& name = "Controller");
    virtual ~Controller() = default;

    virtual void Possess(Pawn* inPawn);
    virtual void UnPossess();

    Pawn* GetPawn() const { return possessedPawn; }

    static std::string StaticClass() { return "Controller"; }

protected:
    Pawn* possessedPawn = nullptr;
};

/**
 * PlayerController - specific controller for player input
 */
class PlayerController : public Controller {
public:
    PlayerController(World* world, const std::string& name = "PlayerController");

    void BeginPlay() override;
    void Tick(float deltaTime) override;

    // Input handling
    virtual void SetupInputComponent();
    void ProcessInput(float deltaTime);

    static std::string StaticClass() { return "PlayerController"; }

private:
    void OnMoveForward(float value);
    void OnMoveRight(float value);
    void OnJump();
    void OnMouseMove(float deltaX, float deltaY);

    // Input state
    float moveForwardValue = 0.0f;
    float moveRightValue = 0.0f;
    bool bJumpPressed = false;

    // Mouse sensitivity
    float mouseSensitivity = 2.0f;
};

/**
 * GameMode - defines the rules and flow of the game
 * Similar to Unreal's AGameMode
 */
class GameMode : public Actor {
public:
    GameMode(World* world, const std::string& name = "GameMode");

    void BeginPlay() override;

    // Game flow
    virtual void StartPlay();
    virtual void EndPlay() override;
    virtual void RestartPlayer(class PlayerController* player);

    // Player management
    virtual Pawn* SpawnDefaultPawnFor(PlayerController* player);
    virtual PlayerController* CreatePlayerController();

    // Game state
    bool HasMatchStarted() const { return bMatchStarted; }
    bool HasMatchEnded() const { return bMatchEnded; }

    static std::string StaticClass() { return "GameMode"; }

protected:
    // Default classes
    std::string DefaultPawnClass = "Character";
    std::string DefaultPlayerControllerClass = "PlayerController";

    // Game state
    bool bMatchStarted = false;
    bool bMatchEnded = false;

    std::vector<PlayerController*> playerControllers;
};

/**
 * Example custom actor - demonstrates the system
 */
class RotatingCube : public Actor {
public:
    RotatingCube(World* world, const std::string& name = "RotatingCube");

    void BeginPlay() override;
    void Tick(float deltaTime) override;

    // Properties that can be set in blueprints
    float RotationSpeed = 90.0f; // degrees per second
    glm::vec3 RotationAxis{0.0f, 1.0f, 0.0f};

    static std::string StaticClass() { return "RotatingCube"; }

private:
    MeshRendererComponent* meshComponent = nullptr;
};
