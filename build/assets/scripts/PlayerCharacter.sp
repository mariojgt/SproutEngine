// Example Sprout Script (.sp) file
// This demonstrates the simple, fun C++ wrapper language

actor PlayerCharacter extends Character {
    // Properties (automatically exposed to Blueprint editor)
    var maxHealth: float = 100.0
    var currentHealth: float = 100.0
    var movementSpeed: float = 600.0
    var jumpHeight: float = 420.0
    var weaponDamage: float = 25.0

    // Arrays and collections
    var inventory: array<string> = ["sword", "potion", "key"]
    var position: vector3 = vector3(0, 0, 0)

    // Called when the actor is created and added to the world
    fun beginPlay() {
        print("PlayerCharacter spawned!")
        print("Starting health: " + currentHealth)

        // Set initial position
        setLocation(0, 0, 0)
        setRotation(0, 0, 0)

        // Setup components
        var meshComp = getMeshComponent()
        if (meshComp != null) {
            meshComp.setMesh("assets/characters/hero.fbx")
            meshComp.setMaterial("assets/materials/hero_mat.json")
        }

        // Bind to input events
        bindInput("MoveForward", onMoveForward)
        bindInput("MoveRight", onMoveRight)
        bindInput("Jump", onJump)
        bindInput("Attack", onAttack)
    }

    // Called every frame
    fun tick(deltaTime: float) {
        // Health regeneration
        if (currentHealth < maxHealth) {
            currentHealth += 10.0 * deltaTime
            if (currentHealth > maxHealth) {
                currentHealth = maxHealth
            }
        }

        // Check if player fell off the world
        var currentPos = getLocation()
        if (currentPos.y < -100) {
            respawn()
        }

        // Update UI
        updateHealthBar(currentHealth / maxHealth)
    }

    // Input handling functions
    fun onMoveForward(value: float) {
        var forward = getForwardVector()
        addMovementInput(forward, value * movementSpeed)
    }

    fun onMoveRight(value: float) {
        var right = getRightVector()
        addMovementInput(right, value * movementSpeed)
    }

    fun onJump() {
        if (canJump()) {
            jump(jumpHeight)
            playSound("assets/audio/jump.wav")
        }
    }

    fun onAttack() {
        performAttack()
        playAnimation("attack")
        playSound("assets/audio/sword_swing.wav")
    }

    // Custom functions
    fun takeDamage(amount: float, attacker: Actor) {
        currentHealth -= amount
        print("Took " + amount + " damage! Health: " + currentHealth)

        // Visual feedback
        flashRed(0.2)
        playSound("assets/audio/hurt.wav")

        if (currentHealth <= 0) {
            die()
        }
    }

    fun heal(amount: float) {
        currentHealth += amount
        if (currentHealth > maxHealth) {
            currentHealth = maxHealth
        }
        print("Healed " + amount + " HP! Health: " + currentHealth)
        playSound("assets/audio/heal.wav")
    }

    fun addToInventory(item: string) {
        inventory.add(item)
        print("Added " + item + " to inventory")
        updateInventoryUI()
    }

    fun removeFromInventory(item: string) -> bool {
        if (inventory.contains(item)) {
            inventory.remove(item)
            print("Removed " + item + " from inventory")
            updateInventoryUI()
            return true
        }
        return false
    }

    fun performAttack() {
        // Raycast forward to find enemies
        var hitResult = raycast(getLocation(), getForwardVector(), 100.0)

        if (hitResult.hit) {
            var hitActor = hitResult.actor
            if (hitActor.hasTag("Enemy")) {
                // Deal damage to enemy
                hitActor.takeDamage(weaponDamage, this)

                // Create hit effect
                spawnEffect("hit_effect", hitResult.location)
            }
        }
    }

    fun die() {
        print("Player died!")
        playAnimation("death")
        playSound("assets/audio/death.wav")

        // Disable input and movement
        setCanMove(false)

        // Respawn after delay
        delay(3.0, respawn)
    }

    fun respawn() {
        print("Respawning player...")
        currentHealth = maxHealth
        setLocation(0, 0, 0) // Spawn point
        setCanMove(true)
        playAnimation("idle")

        // Reset any status effects
        clearStatusEffects()
    }

    // Event handlers (called by engine events)
    fun onBeginOverlap(other: Actor) {
        if (other.hasTag("Pickup")) {
            var pickup = other as PickupActor
            pickup.collect(this)
        } else if (other.hasTag("Enemy")) {
            var enemy = other as EnemyActor
            takeDamage(enemy.contactDamage, enemy)
        }
    }

    fun onEndOverlap(other: Actor) {
        // Handle leaving trigger zones
        if (other.hasTag("SafeZone")) {
            print("Left safe zone")
        }
    }

    // Blueprint-callable functions (can be called from visual blueprints)
    blueprint fun setMaxHealth(newMaxHealth: float) {
        maxHealth = newMaxHealth
        if (currentHealth > maxHealth) {
            currentHealth = maxHealth
        }
    }

    blueprint fun getCurrentHealth() -> float {
        return currentHealth
    }

    blueprint fun getInventorySize() -> int {
        return inventory.size()
    }

    // Private helper functions
    private fun updateHealthBar(percentage: float) {
        // Update UI health bar
        var healthBar = findWidget("HealthBar")
        if (healthBar != null) {
            healthBar.setPercentage(percentage)
        }
    }

    private fun updateInventoryUI() {
        // Update inventory display
        var inventoryWidget = findWidget("InventoryPanel")
        if (inventoryWidget != null) {
            inventoryWidget.refresh(inventory)
        }
    }

    private fun flashRed(duration: float) {
        // Visual damage feedback
        var meshComp = getMeshComponent()
        if (meshComp != null) {
            meshComp.setTint(color(1.0, 0.3, 0.3))
            delay(duration, fun() {
                meshComp.setTint(color(1.0, 1.0, 1.0))
            })
        }
    }
}

// Example of inheritance and composition
actor EnemyCharacter extends Character {
    var attackDamage: float = 15.0
    var detectionRange: float = 500.0
    var attackRange: float = 150.0
    var patrolPoints: array<vector3> = []
    var currentPatrolIndex: int = 0

    fun beginPlay() {
        super.beginPlay() // Call parent function

        // Setup AI behavior
        startPatrolling()
        enableCombatAI()
    }

    fun tick(deltaTime: float) {
        super.tick(deltaTime)

        // AI behavior
        var player = findPlayerCharacter()
        if (player != null) {
            var distanceToPlayer = distance(getLocation(), player.getLocation())

            if (distanceToPlayer <= detectionRange) {
                if (distanceToPlayer <= attackRange) {
                    attackPlayer(player)
                } else {
                    moveTowards(player.getLocation())
                }
            } else {
                continuePatrol()
            }
        }
    }

    fun attackPlayer(player: PlayerCharacter) {
        // Face the player
        lookAt(player.getLocation())

        // Perform attack
        playAnimation("attack")
        player.takeDamage(attackDamage, this)
    }
}
