// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputMappingContext.h"
#include "Camera/CameraComponent.h"
#include "Enemies/UDamageInterface.h"
#include "GameFramework/Character.h"
#include "GameFramework/SpringArmComponent.h"
#define WIN32_LEAN_AND_MEAN
#include "Windows.h"
#include "OrpheeCharacter.generated.h"


UCLASS(Blueprintable)
class SOUL_KNIGHT_API AOrpheeCharacter : public ACharacter, public IUDamageInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AOrpheeCharacter();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats")
	float Health;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats")
	float MaxHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats")
	float Damage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats")
	float DefaultSpeed = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats")
	float RunningSpeed;

	// New variable for dynamic maximum speed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats")
	float DynamicMaxSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Actions")
	bool IsRunning;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Actions")
	bool IsJumping;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Actions")
    bool IsRolling;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Actions")
	bool IsAttacking;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats")
	float RollingDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Actions")
	bool CanRoll = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Actions")
	int AttackAnimationIndex;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Status")
	bool IsDead;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Environment")
	bool IsInDeathWorld;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera")
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera")
	TObjectPtr<UCameraComponent> Camera;

	//Link the mapping context
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input")
	TObjectPtr<UInputMappingContext> PlayerMappingContext;

	//Move input action
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input")
	TObjectPtr<UInputAction> MoveAction;

	//Look input action
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input")
	TObjectPtr<UInputAction> LookAction;

	//Jump input action
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input")
	TObjectPtr<UInputAction> JumpAction;

	//Run input action
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input")
	TObjectPtr<UInputAction> RunAction;
	
public:	
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	//Move function
	UFUNCTION()
	void Move(const FInputActionValue& Value);

	//Look function
	UFUNCTION()
	void Look(const FInputActionValue& InputValue);

	//Detect AZERTY or QWERTY keyboard
	UFUNCTION(BlueprintCallable, Category="Input")
	FString DetectKeyboardType();
	
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void Dodge(UPARAM() FVector Direction);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void Attack();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void StartRunning();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void EndRunning();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void Dead();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void DeathInLivingWorld();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void DeathInDeathWorld();
};
