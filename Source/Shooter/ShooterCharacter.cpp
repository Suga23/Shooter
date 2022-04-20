// Fill out your copyright notice in the Description page of Project Settings.
#include "ShooterCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Engine/SkeletalMeshSocket.h" // USkeletalMeshSocket
#include "DrawDebugHelpers.h"
#include "Particles/ParticleSystemComponent.h"
#include "Item.h"
#include "Components/WidgetComponent.h"

// Sets default values
AShooterCharacter::AShooterCharacter() : 
	BaseTurnRate(45.f), 
	BaseLookUpRate(45.f),
	bAiming(false),			// True, ��� ������������.

	// �������� �������� ���/�� ������������.
	HipTurnRate(90.f),
	HipLookUpRate(90.f),
	AimingTurnRate(20.f), // �� ����� ������������ ���������������� ����.
	AimingLookUpRate(20.f),

	// �������� ���������������� ������ ����.
	MouseHipTurnRate(1.f),
	MouseHipLookUpRate(1.f),
	MouseAimingTurnRate(0.2f),
	MouseAimingLookUpRate(0.2f),

	// �������� ���� ������ ������.
	CameraDefaultFOV(0.f),	// �� ��������� ��� ������ ����.
	CameraZoomedFOV(60.f),	// �� ��������� ����� ��������������������.
	ZoomInterpSpeed(20.f),	// �������� ������������.

	// ������� ��������������� �����������.
	CrosshairSpreadMultiplier(0.f),
	CrosshairVelocityFactor(0.f),
	CrosshairInAirFactor(0.f),
	CrosshairAimFactor(0.f),
	CrosshairShootingFactor(0.f),

	// ���������� ������� ��������.
	ShootTimeDuration(0.05f),
	bFiringBullet(false),

	// ���������� ������� ��������������� ����.
	AutomaticFireRate(0.14f), // �������� ������� ��������������� ����.
	bShouldFire(true),	// ��� ������, �� ������� �������� �� ������ ������ �������� ��� ������ ������� ������.
	bFireButtonPressed(false),
	bShouldTraceForItems(false) // ���������� ��� ������������ ���������.
{
 	/* ���������� ���� ������ ��� ������ Tick() � 
	������ �����. ����� ��� ���������, ����� �������� 
	������������������, ���� ��� �� �����. */
	PrimaryActorTick.bCanEverTick = true;

	// �������� CameraBoom(������������ � ���������, ���� ��� ������������(� ������� �� ������)).
	// ��� ������� �������� ��������, ��� �������� �������� � <> ����� �������.
	// TEXT("CameraBoom") - ����� ��� ����� ����������. 
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));

	// ������������ ������ � �������
	CameraBoom->SetupAttachment(RootComponent); // RootComponent ���������� ����������� �� Actor.

	CameraBoom->TargetArmLength = 180.f; // ���������� ������ �� �������.
	CameraBoom->bUsePawnControlRotation = true; // ����� CameraBoom ��������� �� ������������, ��� �� �� �� ��������, �� ������ ����� ��������� �� ������������.
	CameraBoom->SocketOffset = FVector(0.f, 50.f, 70.f); // �������� ��� ������ � ����� ���������� ����������, � �������� ����������� ������.


	// �������� ��������� ������.
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));

	/* ����� ���-�� ���� ������ ��������� ��� ��� ������, ������ ��������� - � ���� ������ 	�����������.
	SetupAttachment(������ ��������� ��������������� ������, �������� � ������ ������� ������ 	���� ������������ ������) */
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // ���������� ������ � �������.

	// ����� ������ �� ��������� ������������ spring arm.
	FollowCamera->bUsePawnControlRotation = false;

	// ������������� ����, ��� �������� ����� �� ��������� � ������������(�.�., ����� �������� �����, �� �������� �������������� ������ � �����).
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	// ��������� �������� ���������.

	GetCharacterMovement()->bOrientRotationToMovement = false; // �������� �������� � ����������� � ����������� �����.
	// ���������� �������� �������� ��� ���������� �� ����������� ��������
	GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f); // FRotator(Pitch, Yaw, Roll); ... �������� ������ �� ����� ������ ��������.

	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;
}

// Called when the game starts or when spawned
void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (FollowCamera)
	{
		CameraDefaultFOV = GetFollowCamera()->FieldOfView; // FieldOfView - ���������� CameraComponent ������.
		CameraCurrentFOV = CameraDefaultFOV;
	}
}



// Called every frame
void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CameraInterpZoom(DeltaTime); // �������� ������������ ��� ����������, ����� �������� �������������.

	SetLookRates(); // ������ ���������������� ����, � ����������� �� ������������ ��� �� ������������.
	
	CalculateCrosshairSpread(DeltaTime); // ���������� ��������� �������� �������.

	TraceForItems(); // �������� OverlappedItemCount, ����� ��������� ��������.
}

void AShooterCharacter::TraceForItems()
{ 
	if (bShouldTraceForItems)
	{
		FHitResult ItemTraceResult;
		FVector HitLocation;
		TraceUnderCrosshairs(ItemTraceResult, HitLocation);
		if (ItemTraceResult.bBlockingHit)
		{
			// ����� �������� ����� ������ �����, � �����������, ���������� ���������� �� ����� � Item.
			AItem* HitItem = Cast<AItem>(ItemTraceResult.Actor); // bBlockingHit = true, �� ����� ����� �������� ������ � ������������ ��� � AItem.
			if (HitItem && HitItem->GetPickupWidget())
			{
				HitItem->GetPickupWidget()->SetVisibility(true); // �������� Pickup ������ ��� ���������.
			}
			if (TraceHitItemLastFrame) // ��������������� � AItem ��������� ������.
			{
				if (HitItem != TraceHitItemLastFrame)
				{
					// �� �������� � ������ AItem � ���� ����� �� ����������� ����� ��� AItem ����� null.
					TraceHitItemLastFrame->GetPickupWidget()->SetVisibility(false);
				}
			}
			TraceHitItemLastFrame = HitItem; // ������ ������ ��� HitItem ���������� �����.
		}
	}
	else if (TraceHitItemLastFrame)
	{
		// ����������� ��������������� �� ������ ������-���� ��������.
		TraceHitItemLastFrame->GetPickupWidget()->SetVisibility(false); // ���� ����������� �������� �� ������ ���������� ������.
	}
}

// Called to bind functionality to input
void AShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	check(PlayerInputComponent); // �������� �� ����������.
	
	PlayerInputComponent->BindAxis("MoveForward", this, &AShooterCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AShooterCharacter::MoveRight);
	PlayerInputComponent->BindAxis("TurnRate", this, &AShooterCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AShooterCharacter::LookUpAtRate);

	// &APawn ��������� �� ���������(�����������) �������� �����, �� ����� ������ ��������.
	PlayerInputComponent->BindAxis("Turn", this, &AShooterCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &AShooterCharacter::LookUp);

	// IE_Pressed - ������ ��� ������� �������(input event)
	// &Character::Jump - ������� ������ ����������� �� Character ������.
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);

	// ��� ������� ����������, ����� ������ �����������.
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("FireButton", IE_Pressed, this, &AShooterCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("FireButton", IE_Released, this, &AShooterCharacter::FireButtonReleased);

	PlayerInputComponent->BindAction("AimingButton", IE_Pressed, this, &AShooterCharacter::AimingButtonPressed);
	PlayerInputComponent->BindAction("AimingButton", IE_Released, this, &AShooterCharacter::AimingButtonReleased);
}

void AShooterCharacter::FireWeapon()
{
	if (FireSound)
	{
		UGameplayStatics::PlaySound2D(this, FireSound);
	}
	// ���������� � ������ BarrelSocket.
	const USkeletalMeshSocket* BarrelSocket = GetMesh()->GetSocketByName("BarrelSocket");
	if (BarrelSocket)
	{
		// FTransform ��������� Location, Rotation, Scale ������. ������ ��� ������� � �� ����� ���.
		const FTransform SocketTransform = BarrelSocket->GetSocketTransform(GetMesh());

		if (MuzzleFlash)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, SocketTransform);
		}

		FVector BeamEnd;
		// True - ���� ���� ��������� �������� ����� ����.
		bool bBeamEnd = GetBeamEndLocation(SocketTransform.GetLocation(), BeamEnd);

		if (bBeamEnd)
		{
			if (ImpactParticles)
			{
				UGameplayStatics::SpawnEmitterAtLocation(
					GetWorld(),
					ImpactParticles,
					BeamEnd);
			}

			UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(
				GetWorld(),
				BeamParticles,
				SocketTransform);

			if (Beam)
			{
				Beam->SetVectorParameter(FName("Target"), BeamEnd);
			}
		}

		/*
		FHitResult FireHit; // ��������� ���������.
		const FVector Start{ SocketTransform.GetLocation() }; // ������������ ������ ������ ����� ����.
		const FQuat Rotation{ SocketTransform.GetRotation() }; // FQuat - ���������, ��� �������������� ��������� ���������� ���������� ���������(related) � ���������(�������� ������ ��������� ��� � ��������� ������������).
		const FVector RotationAxis{ Rotation.GetAxisX() }; // ��������� ��� X, �� ������� �������� ����.
		const FVector End{ Start + RotationAxis * 50'000.f }; // FVector End{ ������ + ��� �������� * ��������� }; ��� ��� ��� ������ �� ������������� ����� ����� �����������.
		
		// ��� ��� ������������ � ��������.
		FVector BeamEndPoint{ End };
															  
		// LineTraceMulti �������� � ���������� ������ � ���������� �����������, ������� ������ ������.
		// ����� ����������� ����� ����������� ������ ��� ��������� � ��������� FHitResult, ���������� � ���� ����������, ������� ��������� ����������� ������ �� ������ ������.
		GetWorld()->LineTraceSingleByChannel(FireHit, Start, End, ECollisionChannel::ECC_Visibility); // LineTraceSingleByChannel(��� �������, ��������� �������(������ ����� ����), �������� �����, ����������� ������������) // ������ FireHit ����� ���������� �������� ��� ������ �� ����� �����������, 

		// ����� ������ bool, true - ���� ������ �� ���-��, false - ���� �� ������ �� �� ���.
		if (FireHit.bBlockingHit)
		{
			// ��� ��� ����� ��� ������������� ������������ ������� ����������� ��������� � ������������ ����� ������������, �� ���������� ��������������� ��� ����:
			// DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 2.f); // DrawDebugLine(���, ��������� �����, �������� �����, ���� ����� �����������, ����������� ������������� �����, ����� ������������� �����);
			// DrawDebugPoint(GetWorld(), FireHit.Location, 5.f, FColor::Red, false, 2.f); // DrawDebugPoint(���, ������ ��������, ������ �����, ����, ���������� �����, ������������� �����);
		
			BeamEndPoint = FireHit.Location; // �������������� ���������. 

			if (ImpactParticles)
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, FireHit.Location); // SpawnEmitter(���, ������� ������(����), ��������� ���������);
			}
		}

		if (BeamParticles)
		{
			UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BeamParticles, SocketTransform);
			if (Beam)
			{
				Beam->SetVectorParameter(("Target"), BeamEndPoint); // Beam->SetVectorParameter(����, �������� ����� ������������); FVector BeamEndPoint - ���� �� �� ��� �� ������, BeamEndPoint = FireHit.Location; - ���������� ������, ���� ��������� ������������.
			}
		}
		*/
	}

	// ������ �������� �� �������.
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HipFireMontage)
	{
		AnimInstance->Montage_Play(HipFireMontage); // ��������� ������.

		/* � ���������� enum ����� ���� �������, ���������� ��������� ��������� 
		� �������, ������� ����� ������� ������, � �������� ���������� �������. */
		AnimInstance->Montage_JumpToSection(FName("StartFire"));
	}

	StartCrosshairBulletFire();
}

void AShooterCharacter::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		/* ��� ������� �������� �������� �����������, ��� ����������� �����
		��� ����������, ����� ���������� FRotator. */
		const FRotator Rotation{ Controller->GetControlRotation() }; // ���������� ����������� �����.
	
		/* YawRotation - �������� ���������, ������� �� �������� �� �����������,
		����� pitch � roll, ��� ����� ��������. Yaw-����������� - ���� ������ ����� 
		����� � ��������� �� ����� � � ����� �� ����������� �� ���, ��� Yaw. */
		const FRotator YawRotation{ 0, Rotation.Yaw, 0 };

		// ��������� ������� ���������������� �����������.
		// GetUnitAxis(EAxis::X) - �������� ������ ���, �������, ��������������� ��������. ��� ������� �������� X-��� �����������(GetUnitAxis(EAxis::X)), �� �������� �������(FRotationMatrix{YawRotation}).
		const FVector Direction{ FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::X) }; // ��� ������ �������������� ���������, ��������� ��������, ��������������� Yaw-��������.
		AddMovementInput(Direction, Value); // �������� ����������� ������� ������������ ����.
	}
}

void AShooterCharacter::MoveRight(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// ��� ������� �������� �������� �����������, ��� ����������� ����� ��� ����������, ����� ���������� FRotator.
		const FRotator Rotation{ Controller->GetControlRotation() }; // ���������� ����������� ������.
	
		/* YawRotation - �������� ���������, ������� �� �������� �� �����������,
		����� pitch � roll,  �� ����� ��������. Yaw-����������� - ���� ������ ����� 
		����� � ��������� �� ����� � � ����� �� ����������� �� ���, ��� Yaw. */
		const FRotator YawRotation{ 0, Rotation.Yaw, 0 };

		// ��������� ������� ���������������� �����������.
		/* GetUnitAxis(EAxis::X) - �������� ������ ���, �������, ��������������� ��������.
		��� ������� �������� X-��� �����������(GetUnitAxis(EAxis::X)), 
		�� �������� �������(FRotationMatrix{YawRotation}). */
		const FVector Direction{ FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::Y) }; // ��� ������ �������������� ���������, ��������� ��������, ��������������� Yaw-��������.
		AddMovementInput(Direction, Value); // �������� ����������� ������� ������������ ����.
	}
}

void AShooterCharacter::LookUpAtRate(float Rate)
{
	// deg/sec * sec/frame, ������� ���������� � ����� �������� ������� �� ����.
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AShooterCharacter::TurnAtRate(float Rate)
{
	/* Rate - ����������������� �������� ����� 0 � 1. ���������� �������� 0, 
	���� ������ �� ���������� ������, �� �������� ����� ����� 1, ���� ������������ 
	�������, �� � ����������� �� �������� �� ����, �������� Rate ����� ������������� �� 0 �� 1*/

	/* BaseTurnRate ���������� �� Rate, ��������� ��� ���������� � ������ ������, 
	�� ����� ������� �� GetWorld()->GetDeltaSeconds())(��-������� DeltaTime, ��� 
	������ ������� ������ ���� �������� � ������ ������). */

	// ������� ������� �������� ������� �� �������.
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds()); // deg/sec * sec/frame, ������� ���������� � ����� �������� ������� �� ����.
}

void AShooterCharacter::CalculateCrosshairSpread(float DeltaTime)
{
	// ��������� ������� ��������, ��� ����������� ��������� �� ����, �� ������������ �������� ������(����������� � ��������, ����� ��� ��������������� ����� 0 � 1(� ������� �������� 600, �.�. 300 == 0.5, 450 == 0.75).
	FVector2D WalkSpeedRange{ 0.f, 600.f }; // 2D ������ ��������������� ��������� �������� ���������.
	FVector2D VelocityMultiplierRange{ 0.f, 1.f };
	FVector Velocity{ GetVelocity() };
	Velocity.Z = 0.f; // ����� ����, ��� ���, ������ ������������ � 2D.

	// GetMappedRangeValueClamped - ���������� ��������������� ���������, �� ��������� ��������.
	CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(
		WalkSpeedRange, // �������� �������� �� 0 �� 600.
		VelocityMultiplierRange, // ��������� ��������, ��������� �������� 600, �� � ��������� �� 0 �� 1.
		Velocity.Size());

	// ���������, ����� �� ����������� ����������� ���� �������� � �������.
	if (GetCharacterMovement()->IsFalling()) // �������� � �������?
	{
		// ���������� �����������(��������), ���� �������� � �������.
		CrosshairInAirFactor = FMath::FInterpTo(
			CrosshairInAirFactor,
			2.25f, // ����������� ��������, �� �������� ��������������� �����������.
			DeltaTime,
			2.25f); // �������� ����������(���������������) �����������.		
	}
	else // �������� �� �����.
	{
		// ������� ���������� �����������, ���� �������� �� �����.
		CrosshairInAirFactor = FMath::FInterpTo(
			CrosshairInAirFactor,
			0.f, // ����������� ��������, �� �������� ��������������� �����������.
			DeltaTime,
			30.f); // �������� ����������(���������������) �����������.	
	}

	// ���������� ������ ������������, ��� �����������.
	if (bAiming) // �������� �������������?
	{
		// ������� ���������� ����������� �� ������� ��������.
		CrosshairAimFactor = FMath::FInterpTo(
			CrosshairAimFactor,
			0.6f,
			DeltaTime,
			30.f);
	}
	else // �� �������������.
	{
		// ������� ����������� ����������� �� �������� ��������.
		CrosshairAimFactor = FMath::FInterpTo(
			CrosshairAimFactor,
			0.f,
			DeltaTime,
			30.f);
	}

	// True 0.05 ������ ����� ������ ��������.
	if (bFiringBullet)
	{
		CrosshairShootingFactor = FMath::FInterpTo(
			CrosshairShootingFactor,
			0.3f,
			DeltaTime,
			60.f);
	}
	else
	{
		CrosshairShootingFactor = FMath::FInterpTo(
			CrosshairShootingFactor,
			0.f,
			DeltaTime,
			60.f);
	}

	// ���� �������� �� ������������, �� ��� ����� 0, � �������, ���� �������� �� �������������, �� CrooshairAimFactor = 0, � ������ ���������� ����� ����� ������������ ��� ����������.
	CrosshairSpreadMultiplier = 0.5f + CrosshairVelocityFactor + CrosshairInAirFactor - CrosshairAimFactor + CrosshairShootingFactor; // CrosshairVelocityFactor ��������� �� ������ ��������, ����� �������� ��� �������� � �� �������, ����� �����.
}

bool AShooterCharacter::GetBeamEndLocation(const FVector& MuzzleSocketLocation, FVector& OutBeamLocation)
{
	FHitResult CrosshairHitResult;
	bool bCrosshairHit = TraceUnderCrosshairs(CrosshairHitResult, OutBeamLocation); // ��������� ���� ��������� ��� �����������.
	
	if (bCrosshairHit)
	{
		OutBeamLocation = CrosshairHitResult.Location; // ��������������� ������������ ����(�� ��� ���������� ���������� �� �������).
	}
	else // ���������� ���� �� ���� �����������.
	{
		// OutBeamLocation - �������� �������������� ����������� ����. ���� ��������� �� ����, �������������� ���� ����� ��������� �� ����� ���� ������ ������(����� �� ������� �� 50'000 ������).
	}

	// ���������� ������ �����������(������ ������)
	FHitResult WeaponTraceHit;
	const FVector WeaponTraceStart{ MuzzleSocketLocation };
	const FVector StartToEnd{ OutBeamLocation - WeaponTraceStart };
	// ��� ��� ������ �������, ������ ��� 50'000 ������, � ���� ����� �� ������� �� �����.
	const FVector WeaponTraceEnd{ MuzzleSocketLocation + StartToEnd * 1.25f }; // ������ ����� ����������� �� 25% �������, ������ ���� ����� ��������������� �� ���������� ���������, ���� �������� ������.
	
	GetWorld()->LineTraceSingleByChannel(
		WeaponTraceHit, // ��������� ��������.
		WeaponTraceStart, // ������(������� ������).
		WeaponTraceEnd,	// �����(�������� ����� ����).
		ECollisionChannel::ECC_Visibility); // ��������� ������������.

	if (WeaponTraceHit.bBlockingHit) // ������ ����� ������� � �������� ������ ����(BeamEndPoint) ?
	{
		OutBeamLocation = WeaponTraceHit.Location; // �������, ������� ���������� ���������� �������� ����� ����(BeamEndPoint).
		return true;
	}
	return false;
}

void AShooterCharacter::AimingButtonPressed()
{
	bAiming = true;
}

void AShooterCharacter::AimingButtonReleased()
{
	bAiming = false;
}

void AShooterCharacter::CameraInterpZoom(float DeltaTime)
{
	// ������ �� ������ ������������?
	// ���������� ������� ���� ������ ������.
	if (bAiming)
	{
		// ��������� ���� ������.
		CameraCurrentFOV = FMath::FInterpTo(
			CameraCurrentFOV, // ������� ��������.
			CameraZoomedFOV, // ������� ��������
			DeltaTime, // ������ ����,
			ZoomInterpSpeed); // �������� ����������.
	}
	else
	{
		// ���� ������ �� ���������.
		CameraCurrentFOV = FMath::FInterpTo(
			CameraCurrentFOV, // ������� ��������.
			CameraDefaultFOV, // ������� ��������
			DeltaTime, // ������ ����,
			ZoomInterpSpeed); // �������� ����������.
	}
	// ���������� ������� �������� ��������� ������.
	GetFollowCamera()->SetFieldOfView(CameraCurrentFOV);
}

// Yaw - ��������(��� X).
void AShooterCharacter::Turn(float Value)
{
	float TurnScaleFactor{};
	if (bAiming)
	{
		TurnScaleFactor = MouseAimingTurnRate;
	}
	else
	{
		TurnScaleFactor = MouseHipTurnRate;
	}
	AddControllerYawInput(Value * TurnScaleFactor);
}

// Pitch - �����/����(��� Y)
void AShooterCharacter::LookUp(float Value)
{
	float LookUpScaleFactor{};
	if (bAiming)
	{
		LookUpScaleFactor = MouseAimingLookUpRate;
	}
	else
	{
		LookUpScaleFactor = MouseHipLookUpRate;
	}
	AddControllerPitchInput(Value * LookUpScaleFactor);
}

void AShooterCharacter::SetLookRates()
{
	if (bAiming)
	{
		BaseTurnRate = AimingTurnRate;
		BaseTurnRate = AimingLookUpRate;
	}
	else
	{
		BaseTurnRate = HipTurnRate;
		BaseTurnRate = HipLookUpRate;
	}
}

float AShooterCharacter::GetCrosshairSpreadMultiplier() const
{
	return CrosshairSpreadMultiplier;
}

// ������ ���, ����� ���������� �������� �� ������, ���������� ��� �������, ������� ��������� ������, �� ��������� �������, ���������� ������� FinishCrosshairBulletFire.
void AShooterCharacter::StartCrosshairBulletFire()
{
	bFiringBullet = true; // True, ����� �������� ��������.

	GetWorldTimerManager().SetTimer(
		CrosshairShootTimer,
		this, // ����� ������������.
		&AShooterCharacter::FinishCrosshairBulletFire, // ����� �������.
		ShootTimeDuration);// ����������������� ��������.
		// ��������� �������� �� ������������, ���� ������������ ����� ��������� ������.	
}

void AShooterCharacter::FinishCrosshairBulletFire()
{
	bFiringBullet = false; // C��������� false, �� ����������� 0.5, ����� ����������� ��������.
}

void AShooterCharacter::FireButtonPressed()
{
	bFireButtonPressed = true;
	StartFireTimer();
}

void AShooterCharacter::FireButtonReleased()
{
	bFireButtonPressed = false;
}

void AShooterCharacter::StartFireTimer()
{
	if (bShouldFire)
	{
		FireWeapon();
		bShouldFire = false;
		GetWorldTimerManager().SetTimer(
			AutoFireTimer,	// ������.
			this,		// ���������� �����.
			&AShooterCharacter::AutoFireReset, // �������, ������� ����� ��������������.
			AutomaticFireRate);	//����� ��������.
	}
}

void AShooterCharacter::AutoFireReset()
{
	bShouldFire = true;
	if (bFireButtonPressed) // ���������, ���������� �� �������� ��������.
	{
		StartFireTimer();
	}
}

bool AShooterCharacter::TraceUnderCrosshairs(FHitResult& OutHitResult, FVector& OutHitLocation)
{
	// �������� ������ ���� ������.
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	// ��������� �������������� Crosshair �� ������.
	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f); // �������������� �����������, ���������������� ��� X � Y, ����� �������� ���� ���������, ������� X, Y ������� �� 2.f.
	FVector CrosshairWorldPosition; // ��������� �������.
	FVector CrosshairWorldDirection; // ��������� �����������.

	// ���� ������ ����� bool �� �������� � ��������� �������.
	// ��������� ������� ������� �����������.
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0), // GetPlayerController - ���������� ���������� ��� Pawn(������ ����������, ������ ������(�.�. ���� ����� � ����)
		CrosshairLocation, // �������������� �����������
		CrosshairWorldPosition,
		CrosshairWorldDirection);

	if (bScreenToWorld)
	{
		// ���� �� �������������� ���� ����������� ����������.
		const FVector Start{ CrosshairWorldPosition };
		const FVector End{ Start + CrosshairWorldDirection * 50'000.f };
		OutHitLocation = End;
		GetWorld()->LineTraceSingleByChannel(
			OutHitResult, // ��������� true/false, �������� ���������� ���� ���������(����� �� ������).
			Start,
			End,
			ECollisionChannel::ECC_Visibility); // ���������.
		if (OutHitResult.bBlockingHit)
		{
			// ��� ����������� ����� �� ��������� ��� ���, ��� ������ �� ���� ����� ���-�� ���������, ������ ������������ ��� ���� � ��������������.
			OutHitLocation = OutHitResult.Location;
			return true;
		}
	}
	return false;
}

void AShooterCharacter::IncrementOverlappedItemCount(int8 Amount)
{
	if (OverlappedItemCount + Amount <= 0)
	{
		OverlappedItemCount = 0;
		bShouldTraceForItems = false;
	}
	else
	{
		OverlappedItemCount += Amount;
		bShouldTraceForItems = true;
	}
}
