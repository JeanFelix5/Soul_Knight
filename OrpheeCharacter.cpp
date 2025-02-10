// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/OrpheeCharacter.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"


// Sets default values
AOrpheeCharacter::AOrpheeCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	//Spring Arm
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Spring arm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 400.0f;
	SpringArm->bUsePawnControlRotation = true;

	//Camera
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false; 

	//This let the camera look around the player, without making the player rotate with the camera
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // rotate at this rotation rate (500 by default)
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.0f;
}

// Called when the game starts or when spawned
void AOrpheeCharacter::BeginPlay()
{
	Super::BeginPlay();

	if(const APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if(UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			if(Subsystem)
			{
				Subsystem->AddMappingContext(PlayerMappingContext, 0);
			}
		}
	}
}

// Called to bind functionality to input
void AOrpheeCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if(UEnhancedInputComponent* Input = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		//Move Action
		Input->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AOrpheeCharacter::Move);
		
		//By binding this action and not calling it from blueprint it prevent some bugs from occuring.
		//Look Action
		Input->BindAction(LookAction, ETriggerEvent::Triggered, this, &AOrpheeCharacter::Look);

		//Jump action
		Input->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		Input->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		//Run action
		Input->BindAction(RunAction, ETriggerEvent::Triggered, this, &AOrpheeCharacter::StartRunning);
		Input->BindAction(RunAction, ETriggerEvent::Completed, this, &AOrpheeCharacter::EndRunning);
	}
}

void AOrpheeCharacter::Move(const FInputActionValue& Value)
{
	if(IsRolling == true || IsDead == true)
		return;

	// Get the player input
	APlayerController* PlayerController = Cast<APlayerController>(Controller);
	if (!PlayerController)
		return;
	
	// Get the tilt values of the left analog stick
	float TiltX, TiltY;
	PlayerController->GetInputAnalogStickState(EControllerAnalogStick::CAS_LeftStick, TiltX, TiltY);

	// Calculate the magnitude of the tilt vector
	float Magnitude = FMath::Sqrt(FMath::Square(TiltX) + FMath::Square(TiltY));

	//if the magnitude and the tilt is 0. Verify if the move input is triggered by the keyboard 
	if(Magnitude == 0.0f && TiltX == 0.0f && TiltY == 0.0f)
	{
		// Check if at least one of the keys W, A, S, D, Z, or Q is pressed
		if (PlayerController->IsInputKeyDown(EKeys::W) ||
			PlayerController->IsInputKeyDown(EKeys::A) ||
			PlayerController->IsInputKeyDown(EKeys::S) ||
			PlayerController->IsInputKeyDown(EKeys::D) ||
			PlayerController->IsInputKeyDown(EKeys::Z) ||
			PlayerController->IsInputKeyDown(EKeys::Q))
		{
		
			const FVector2D MovementVector = Value.Get<FVector2D>();
			
			//Code that make the camera influence the player's walking direction
			if (Controller != nullptr)
			{
				//Find out which way is forward by getting the controller control rotation
				const FRotator Rotation = Controller->GetControlRotation();
				const FRotator YawRotation(0, Rotation.Yaw, 0);

				//Get forward vector
				const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		
				//Get right vector 
				const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

				//Add movement input   
				AddMovementInput(ForwardDirection, MovementVector.X);
				AddMovementInput(RightDirection, MovementVector.Y);
			}
		}
	}
	else
	{	//Interpolate the speed of the player depending of the magnitude of the tilt of the controller
		
		// Interpolate between zero speed and full speed based on the magnitude
		const float InterpolatedSpeed = FMath::InterpEaseInOut(0.0f, DefaultSpeed, Magnitude, 3.0f);

		// Normalize the tilt vector and scale it with the interpolated speed
		FVector2D NormalizedTilt(TiltY, TiltX);
		NormalizedTilt.Normalize();
		const FVector2D AdjustedMovement = NormalizedTilt * InterpolatedSpeed;
		
		//GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow, FString::Printf(TEXT("yellow : %f"), InterpolatedSpeed));

		// Code that makes the camera influence the player's walking direction
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// Add movement input with the adjusted vector (divide by the default speed to get a value between 0 and 1 because scale value is 0 to 1)
		AddMovementInput(ForwardDirection, AdjustedMovement.X / DefaultSpeed);
		AddMovementInput(RightDirection, AdjustedMovement.Y / DefaultSpeed);

		//GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Green, FString::Printf(TEXT("X float value is: %f"), AdjustedMovement.X / DefaultSpeed));
		//GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Green, FString::Printf(TEXT("Y float value is: %f"), AdjustedMovement.Y / DefaultSpeed));
	}
	
	/*
	 //Code that makes the player move normally regardless of the tilt magnitude of the left stick on the controller (move normally without tilt logic)
	if(IsRolling == true || IsDead == true)
		return;

	const FVector2D MovementVector = Value.Get<FVector2D>();
	
	//Code that make the camera influence the player's walking direction
	if (Controller != nullptr)
	{
		//Find out which way is forward by getting the controller control rotation
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		//Get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		//Get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		//Add movement input   
		AddMovementInput(ForwardDirection, MovementVector.X);
		AddMovementInput(RightDirection, MovementVector.Y);
	}
*/
	//Code that make the camera not influence the player's walking direction. Because I don't use the control rotation
	//Get the movement vector with the received input (X,Y) * the forward vector and the right vector of the actor
	//const UE::Math::TVector<double> MovementVector = GetActorForwardVector()*X + GetActorRightVector()*Y;

	//Add the movement input in the direction of this vector
	//AddMovementInput(MovementVector);
}

void AOrpheeCharacter::Look(const FInputActionValue& InputValue)
{
	const FVector2D InputVector = InputValue.Get<FVector2D>();

	if(Controller != nullptr)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Green, FString::Printf(TEXT("input X float value is: %f"), InputVector.X));
		//GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, FString::Printf(TEXT("input y float value is: %f"), InputVector.Y));
		AddControllerYawInput(InputVector.X);
		AddControllerPitchInput(InputVector.Y);
	}
}

FString AOrpheeCharacter::DetectKeyboardType()
{
	FString Keyboard;
	switch(PRIMARYLANGID(LOWORD(GetKeyboardLayout(0))))
	{
	case LANG_FRENCH:
		Keyboard= "AZERTY";
		if(SUBLANGID(LOWORD(GetKeyboardLayout(0)))==SUBLANG_FRENCH_CANADIAN) 
			Keyboard="QWERTY";
		break;
	case LANG_ENGLISH:
		Keyboard= "QWERTY";
		break;
	default:
		Keyboard= "unknown";
		break;
	}

	return Keyboard;
}

