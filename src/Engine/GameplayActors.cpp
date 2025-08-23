#include "GameplayActors.h"
#include "World.h"
#include <iostream>

// Pawn Implementation
Pawn::Pawn(World* world, const std::string& name) : Actor(world, name) {
}

void Pawn::PossessedBy(Controller* newController) {
    controller = newController;
    SetupPlayerInputComponent();
}

void Pawn::UnPossessed() {
    controller = nullptr;
}

void Pawn::AddMovementInput(const glm::vec3& direction, float scaleValue) {
    pendingMovementInput += direction * scaleValue;
}

void Pawn::TickMovement(float deltaTime) {
    if (glm::length(pendingMovementInput) > 0.0f) {
        glm::vec3 currentLocation = GetActorLocation();
        glm::vec3 movement = glm::normalize(pendingMovementInput) * deltaTime;
        SetActorLocation(currentLocation + movement);
    }
    pendingMovementInput = glm::vec3(0.0f);
}

// Character Implementation
Character::Character(World* world, const std::string& name) : Pawn(world, name) {
    SetupComponents();
}

void Character::BeginPlay() {
    Pawn::BeginPlay();
    std::cout << "Character " << GetName() << " has begun play!" << std::endl;
}

void Character::Tick(float deltaTime) {
    Pawn::Tick(deltaTime);
    TickMovement(deltaTime);
}

void Character::MoveForward(float value) {
    if (value != 0.0f) {
        glm::vec3 forward = GetForwardVector();
        AddMovementInput(forward, value);
    }
}

void Character::MoveRight(float value) {
    if (value != 0.0f) {
        glm::vec3 right = GetRightVector();
        AddMovementInput(right, value);
    }
}

void Character::Jump() {
    if (bCanJump) {
        // Add upward velocity (this would be handled by physics in a real implementation)
        glm::vec3 currentLocation = GetActorLocation();
        SetActorLocation(currentLocation + glm::vec3(0, JumpVelocity * 0.01f, 0));
        bCanJump = false;

        // TODO: Reset bCanJump when landing (needs physics integration)
    }
}

void Character::SetupComponents() {
    // Create mesh component
    meshComponent = CreateComponent<MeshRendererComponent>();
    meshComponent->SetMesh("assets/meshes/character.fbx");
    meshComponent->SetMaterial("assets/materials/character_mat.json");

    // Create collision component
    capsuleComponent = CreateComponent<CollisionComponent>(CollisionComponent::CollisionType::Capsule);
    capsuleComponent->SetCapsuleRadius(0.5f);
    capsuleComponent->SetCapsuleHeight(2.0f);

    // Create camera component
    cameraComponent = CreateComponent<CameraComponent>();
    cameraComponent->SetRelativeLocation(glm::vec3(0, 0, 1.8f)); // Eye level
}

// Controller Implementation
Controller::Controller(World* world, const std::string& name) : Actor(world, name) {
}

void Controller::Possess(Pawn* inPawn) {
    if (possessedPawn) {
        UnPossess();
    }

    possessedPawn = inPawn;
    if (possessedPawn) {
        possessedPawn->PossessedBy(this);
    }
}

void Controller::UnPossess() {
    if (possessedPawn) {
        possessedPawn->UnPossessed();
        possessedPawn = nullptr;
    }
}

// PlayerController Implementation
PlayerController::PlayerController(World* world, const std::string& name) : Controller(world, name) {
}

void PlayerController::BeginPlay() {
    Controller::BeginPlay();
    SetupInputComponent();
}

void PlayerController::Tick(float deltaTime) {
    Controller::Tick(deltaTime);
    ProcessInput(deltaTime);
}

void PlayerController::SetupInputComponent() {
    // TODO: Bind input events to functions
    // This would be connected to the engine's input system
    std::cout << "Setting up input component for " << GetName() << std::endl;
}

void PlayerController::ProcessInput(float deltaTime) {
    if (auto* character = dynamic_cast<Character*>(GetPawn())) {
        character->MoveForward(moveForwardValue);
        character->MoveRight(moveRightValue);

        if (bJumpPressed) {
            character->Jump();
            bJumpPressed = false;
        }
    }
}

void PlayerController::OnMoveForward(float value) {
    moveForwardValue = value;
}

void PlayerController::OnMoveRight(float value) {
    moveRightValue = value;
}

void PlayerController::OnJump() {
    bJumpPressed = true;
}

void PlayerController::OnMouseMove(float deltaX, float deltaY) {
    if (auto* pawn = GetPawn()) {
        glm::vec3 currentRotation = pawn->GetActorRotation();
        currentRotation.y += deltaX * mouseSensitivity;
        currentRotation.x += deltaY * mouseSensitivity;

        // Clamp pitch
        currentRotation.x = glm::clamp(currentRotation.x, -90.0f, 90.0f);

        pawn->SetActorRotation(currentRotation);
    }
}

// GameMode Implementation
GameMode::GameMode(World* world, const std::string& name) : Actor(world, name) {
}

void GameMode::BeginPlay() {
    Actor::BeginPlay();
    StartPlay();
}

void GameMode::StartPlay() {
    if (!bMatchStarted) {
        bMatchStarted = true;
        std::cout << "Game match started!" << std::endl;

        // Create default player controller and pawn
        PlayerController* playerController = CreatePlayerController();
        if (playerController) {
            Pawn* defaultPawn = SpawnDefaultPawnFor(playerController);
            if (defaultPawn) {
                playerController->Possess(defaultPawn);
            }
        }
    }
}

void GameMode::EndPlay() {
    bMatchEnded = true;
    std::cout << "Game match ended!" << std::endl;
    Actor::EndPlay();
}

void GameMode::RestartPlayer(PlayerController* player) {
    if (player) {
        Pawn* newPawn = SpawnDefaultPawnFor(player);
        if (newPawn) {
            player->Possess(newPawn);
        }
    }
}

Pawn* GameMode::SpawnDefaultPawnFor(PlayerController* player) {
    if (GetWorld() && DefaultPawnClass == "Character") {
        return GetWorld()->SpawnActor<Character>("DefaultCharacter");
    }
    return nullptr;
}

PlayerController* GameMode::CreatePlayerController() {
    if (GetWorld()) {
        PlayerController* controller = GetWorld()->SpawnActor<PlayerController>("PlayerController_0");
        playerControllers.push_back(controller);
        return controller;
    }
    return nullptr;
}

// RotatingCube Implementation
RotatingCube::RotatingCube(World* world, const std::string& name) : Actor(world, name) {
    // Create mesh component
    meshComponent = CreateComponent<MeshRendererComponent>();
    meshComponent->SetMesh("assets/meshes/cube.obj");
    meshComponent->SetMaterial("assets/materials/default_mat.json");
}

void RotatingCube::BeginPlay() {
    Actor::BeginPlay();
    std::cout << "RotatingCube " << GetName() << " started rotating!" << std::endl;
}

void RotatingCube::Tick(float deltaTime) {
    Actor::Tick(deltaTime);

    // Rotate the cube
    glm::vec3 currentRotation = GetActorRotation();
    glm::vec3 rotationDelta = RotationAxis * RotationSpeed * deltaTime;
    SetActorRotation(currentRotation + rotationDelta);
}
