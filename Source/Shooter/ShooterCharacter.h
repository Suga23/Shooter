// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Particles/ParticleSystemComponent.h" // Для Beam->SetVectorParameter()
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

	// Вызывается от вводов клавиш вперёд/назад.
	void MoveForward(float Value);

	// Вызывается от вводов влево/вправо.
	void MoveRight(float Value);

	/* Вызывается через ввод и поворачивается через заданную уровень.
	@param Rate - это нормализированный уровень, то есть 1.0(Rate = 1.0) значит 100% желанного уровня вращения. */
	void TurnAtRate(float Rate);

	/* Вызывается через ввод и поворачивается через заданную уровень.
	@param Rate - это нормализированный уровень, то есть 1.0(Rate = 1.0) значит 100% желанного уровня вращения. */
	void LookUpAtRate(float Rate);

	// Вызывается, когда FireButton нажата.
	void FireWeapon();

	bool GetBeamEndLocation(const FVector& MuzzleSocketLocation, FVector& OutBeamLocation);

	// Настроить bAiming true или false по нажатию кнопки.
	void AimingButtonPressed();

	void AimingButtonReleased();

	void CameraInterpZoom(float DeltaTime);

	/* Контроллер вращения базированный на оси X мышки.
	@param Value - вводимое значение для движения мыши. */
	void Turn(float Value);

	/* Контроллер вращения базированный на оси Y мышки.
	@param Value - вводимое значение для движения мыши. */
	void LookUp(float Value);

	// Установить BaseTurnRate and BaseLookUpRate, основанный на прицеливании.
	void SetLookRates();

	// Метод определяющий увеличения или нахождения в нормальном состоянии перекрестья(во время стрельбы, прыжка, падения, бега).
	void CalculateCrosshairSpread(float DeltaTime);

	void StartCrosshairBulletFire();

	UFUNCTION() // Это вызываемая функция для определённого времени.
	void FinishCrosshairBulletFire();

	void FireButtonPressed();
	void FireButtonReleased();

	void StartFireTimer(); // Устанавливает таймер между выстрелами.

	UFUNCTION()
	void AutoFireReset(); // Когда стрельба завершена.

	bool TraceUnderCrosshairs(FHitResult& OutHitResult, FVector& OutHitLocation); // Линия трассировки для предметов под прицелом.

	void TraceForItems(); // Отслеживать для предметов, если OverlappedItemCount > 0.

private:
	// Всё это позволяет задавать камере определённую позицию камере и сохранять её.
	// UPROPERTY - помечает эту переменную для сборки мусора, также позволяет отобразить это в blueprints.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera; // Камера следует за игроком.

	// Базовый уровень вращения рассчитываемый в гардусах за секунду(град/сек). Другое масштабирование может оказывать окончательный уровень вращения.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float BaseTurnRate;

	// Базовый уровень взгляда персонажа вверх/вниз рассчитываемый в гардусах за секунду(град/сек). Другое масштабирование может оказывать окончательный уровень вращения.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float BaseLookUpRate;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float HipTurnRate; // Скорость поворота без прицеливания

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float HipLookUpRate; // Скорость подъёма камеры для просмотра вверх/внизу, без прицеливания.

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float AimingTurnRate; // Скорость вращения, во время прицеливания.

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float AimingLookUpRate; // Скорость подъёма камеры для просмотра вверх/вниз, во время прицеливания.

	// Эти коэффиценты будут ограничены, от 0 до 1
	/* Скалярный коэффициент для чувствительности обзора мыши.
	Скорость вращения(по оси X), когда персонаж не целится. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float MouseHipTurnRate;

	/* Скалярный коэффициент для чувствительности обзора мыши.
	Скорость подъёма(по оси Y), когда персонаж не целится. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float MouseHipLookUpRate;

	/* Скалярный коэффициент для чувствительности обзора мыши.
	Скорость вращения(по оси X), когда персонаж целится. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float MouseAimingTurnRate;

	/* Скалярный коэффициент для чувствительности обзора мыши.
	Скорость подъёма(по оси Y), когда персонаж целится. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float MouseAimingLookUpRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class USoundCue* FireSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class UParticleSystem* MuzzleFlash;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class UAnimMontage* HipFireMontage; // Монтаж для стрельбы из оружия

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UParticleSystem* ImpactParticles; // Частицы, образующиеся при попадании пули.

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UParticleSystem* BeamParticles; // Дымный след для пуль.

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = combat, meta = (AllowPrivateAccess = "true"))
	bool bAiming; // True, когда персонаж прицеливается.

	// FOV - field of view.
	float CameraDefaultFOV; // Значение поля зрения камеры по умолчанию.

	float CameraZoomedFOV; // Поле зрение камеры, при увеличении.

	float CameraCurrentFOV;	// Текущее поле зрения данного кадра.

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float ZoomInterpSpeed;	// Скорость увеличения, когда персонаж прицеливается.

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
	float CrosshairSpreadMultiplier; // Определяет распространение перекрестия.

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
	float CrosshairVelocityFactor; // Компонент скорости разброса перекрестья.

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
	float CrosshairInAirFactor; // Компонент, разброса перекрестия, если персонаж находится в воздухе или во время прыжка.

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
	float CrosshairAimFactor; // Компонент прицеливания разброса перекрестия.

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
	float CrosshairShootingFactor; // Компонент, используемый, когда персонаж стреляет.

	float ShootTimeDuration; // Продолжительность стрельбы.

	bool bFiringBullet; // Стреляет ли персонаж.

	FTimerHandle CrosshairShootTimer; // Время стрельбы. Эта функция назначена быть вызванной, когда таймер прекратил работу.

	bool bFireButtonPressed; // Левая кнопка мыши или правый триггер на гемпаде нажат.

	bool bShouldFire; // True - когда персонаж стреляет, False - когда ждёт таймер.

	float AutomaticFireRate; // Скорострельность автоматического оружия.

	FTimerHandle AutoFireTimer; // Установить таймер, занимающий время между выстрелами.

	bool bShouldTraceForItems; // True если необходимо отслеживать каждый кадр для предметов.

	int8 OverlappedItemCount; // Подсчёт AItem со сферами которых соприкоснулись.

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "true"))
	class AItem* TraceHitItemLastFrame; // AItem последний кадр.

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/* Поскольку был напрямую объявлен этот класс в приватной секции можно 
	создать публичный получатель, возвращаемый этот тип без прямого объявления снова. */

	/* FORCEINLINE -  является линейной функцией, потому что в коде вызывается 
	камера. Когда компиляция будет завершена, вызов этой этой функции будет перемещён в тело функции. */

	// Возвращает FollowCamera подобъект.
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; } // Возвращает подобъект CameraBoom.

	FORCEINLINE bool GetAiming() const { return bAiming; }

	UFUNCTION(BlueprintCallable)
	float GetCrosshairSpreadMultiplier() const;

	FORCEINLINE int8 GetOverllappedItemCount() const { return OverlappedItemCount; }

	// Функция, которая может добавлять или вычитать к/из OverlappedItemcount и обновляет bShouldTraceForItems.
	void IncrementOverlappedItemCount(int8 Amount); // Если перерытие равно 0, то можно прекратить отслеживание для предметов.
};
