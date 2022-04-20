// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Particles/ParticleSystemComponent.h" // ��� Beam->SetVectorParameter()
#include "ShooterCharacter.generated.h"

UCLASS()
class SHOOTER_API AShooterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AShooterCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// ���������� �� ������ ������ �����/�����.
	void MoveForward(float Value);

	// ���������� �� ������ �����/������.
	void MoveRight(float Value);

	/* ���������� ����� ���� � �������������� ����� �������� �������.
	@param Rate - ��� ����������������� �������, �� ���� 1.0(Rate = 1.0) ������ 100% ��������� ������ ��������. */
	void TurnAtRate(float Rate);

	/* ���������� ����� ���� � �������������� ����� �������� �������.
	@param Rate - ��� ����������������� �������, �� ���� 1.0(Rate = 1.0) ������ 100% ��������� ������ ��������. */
	void LookUpAtRate(float Rate);

	// ����������, ����� FireButton ������.
	void FireWeapon();

	bool GetBeamEndLocation(const FVector& MuzzleSocketLocation, FVector& OutBeamLocation);

	// ��������� bAiming true ��� false �� ������� ������.
	void AimingButtonPressed();

	void AimingButtonReleased();

	void CameraInterpZoom(float DeltaTime);

	/* ���������� �������� ������������ �� ��� X �����.
	@param Value - �������� �������� ��� �������� ����. */
	void Turn(float Value);

	/* ���������� �������� ������������ �� ��� Y �����.
	@param Value - �������� �������� ��� �������� ����. */
	void LookUp(float Value);

	// ���������� BaseTurnRate and BaseLookUpRate, ���������� �� ������������.
	void SetLookRates();

	// ����� ������������ ���������� ��� ���������� � ���������� ��������� �����������(�� ����� ��������, ������, �������, ����).
	void CalculateCrosshairSpread(float DeltaTime);

	void StartCrosshairBulletFire();

	UFUNCTION() // ��� ���������� ������� ��� ������������ �������.
	void FinishCrosshairBulletFire();

	void FireButtonPressed();
	void FireButtonReleased();

	void StartFireTimer(); // ������������� ������ ����� ����������.

	UFUNCTION()
	void AutoFireReset(); // ����� �������� ���������.

	bool TraceUnderCrosshairs(FHitResult& OutHitResult, FVector& OutHitLocation); // ����� ����������� ��� ��������� ��� ��������.

	void TraceForItems(); // ����������� ��� ���������, ���� OverlappedItemCount > 0.

private:
	// �� ��� ��������� �������� ������ ����������� ������� ������ � ��������� �.
	// UPROPERTY - �������� ��� ���������� ��� ������ ������, ����� ��������� ���������� ��� � blueprints.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera; // ������ ������� �� �������.

	// ������� ������� �������� �������������� � �������� �� �������(����/���). ������ ��������������� ����� ��������� ������������� ������� ��������.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float BaseTurnRate;

	// ������� ������� ������� ��������� �����/���� �������������� � �������� �� �������(����/���). ������ ��������������� ����� ��������� ������������� ������� ��������.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float BaseLookUpRate;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float HipTurnRate; // �������� �������� ��� ������������

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float HipLookUpRate; // �������� ������� ������ ��� ��������� �����/�����, ��� ������������.

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float AimingTurnRate; // �������� ��������, �� ����� ������������.

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float AimingLookUpRate; // �������� ������� ������ ��� ��������� �����/����, �� ����� ������������.

	// ��� ����������� ����� ����������, �� 0 �� 1
	/* ��������� ����������� ��� ���������������� ������ ����.
	�������� ��������(�� ��� X), ����� �������� �� �������. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float MouseHipTurnRate;

	/* ��������� ����������� ��� ���������������� ������ ����.
	�������� �������(�� ��� Y), ����� �������� �� �������. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float MouseHipLookUpRate;

	/* ��������� ����������� ��� ���������������� ������ ����.
	�������� ��������(�� ��� X), ����� �������� �������. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float MouseAimingTurnRate;

	/* ��������� ����������� ��� ���������������� ������ ����.
	�������� �������(�� ��� Y), ����� �������� �������. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float MouseAimingLookUpRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class USoundCue* FireSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class UParticleSystem* MuzzleFlash;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class UAnimMontage* HipFireMontage; // ������ ��� �������� �� ������

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UParticleSystem* ImpactParticles; // �������, ������������ ��� ��������� ����.

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UParticleSystem* BeamParticles; // ������ ���� ��� ����.

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = combat, meta = (AllowPrivateAccess = "true"))
	bool bAiming; // True, ����� �������� �������������.

	// FOV - field of view.
	float CameraDefaultFOV; // �������� ���� ������ ������ �� ���������.

	float CameraZoomedFOV; // ���� ������ ������, ��� ����������.

	float CameraCurrentFOV;	// ������� ���� ������ ������� �����.

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float ZoomInterpSpeed;	// �������� ����������, ����� �������� �������������.

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
	float CrosshairSpreadMultiplier; // ���������� ��������������� �����������.

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
	float CrosshairVelocityFactor; // ��������� �������� �������� �����������.

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
	float CrosshairInAirFactor; // ���������, �������� �����������, ���� �������� ��������� � ������� ��� �� ����� ������.

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
	float CrosshairAimFactor; // ��������� ������������ �������� �����������.

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
	float CrosshairShootingFactor; // ���������, ������������, ����� �������� ��������.

	float ShootTimeDuration; // ����������������� ��������.

	bool bFiringBullet; // �������� �� ��������.

	FTimerHandle CrosshairShootTimer; // ����� ��������. ��� ������� ��������� ���� ���������, ����� ������ ��������� ������.

	bool bFireButtonPressed; // ����� ������ ���� ��� ������ ������� �� ������� �����.

	bool bShouldFire; // True - ����� �������� ��������, False - ����� ��� ������.

	float AutomaticFireRate; // ���������������� ��������������� ������.

	FTimerHandle AutoFireTimer; // ���������� ������, ���������� ����� ����� ����������.

	bool bShouldTraceForItems; // True ���� ���������� ����������� ������ ���� ��� ���������.

	int8 OverlappedItemCount; // ������� AItem �� ������� ������� ��������������.

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "true"))
	class AItem* TraceHitItemLastFrame; // AItem ��������� ����.

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/* ��������� ��� �������� �������� ���� ����� � ��������� ������ ����� 
	������� ��������� ����������, ������������ ���� ��� ��� ������� ���������� �����. */

	/* FORCEINLINE -  �������� �������� ��������, ������ ��� � ���� ���������� 
	������. ����� ���������� ����� ���������, ����� ���� ���� ������� ����� ��������� � ���� �������. */

	// ���������� FollowCamera ���������.
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; } // ���������� ��������� CameraBoom.

	FORCEINLINE bool GetAiming() const { return bAiming; }

	UFUNCTION(BlueprintCallable)
	float GetCrosshairSpreadMultiplier() const;

	FORCEINLINE int8 GetOverllappedItemCount() const { return OverlappedItemCount; }

	// �������, ������� ����� ��������� ��� �������� �/�� OverlappedItemcount � ��������� bShouldTraceForItems.
	void IncrementOverlappedItemCount(int8 Amount); // ���� ��������� ����� 0, �� ����� ���������� ������������ ��� ���������.
};
