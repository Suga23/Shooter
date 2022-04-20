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
	bAiming(false),			// True, при прицеливании.

	// Скорость поворота при/не прицеливании.
	HipTurnRate(90.f),
	HipLookUpRate(90.f),
	AimingTurnRate(20.f), // Во время прицеливания чувствительность ниже.
	AimingLookUpRate(20.f),

	// Величины чувствительности обзора мыши.
	MouseHipTurnRate(1.f),
	MouseHipLookUpRate(1.f),
	MouseAimingTurnRate(0.2f),
	MouseAimingLookUpRate(0.2f),

	// Значения поля зрения камеры.
	CameraDefaultFOV(0.f),	// По умолчанию при начале игры.
	CameraZoomedFOV(60.f),	// Со значением можно поэксперементировать.
	ZoomInterpSpeed(20.f),	// Скорость интерполяции.

	// Факторы распространения перекрестия.
	CrosshairSpreadMultiplier(0.f),
	CrosshairVelocityFactor(0.f),
	CrosshairInAirFactor(0.f),
	CrosshairAimFactor(0.f),
	CrosshairShootingFactor(0.f),

	// Переменные таймера стрельбы.
	ShootTimeDuration(0.05f),
	bFiringBullet(false),

	// Переменные ведения автоматического огня.
	AutomaticFireRate(0.14f), // Скорость ведения автоматического огня.
	bShouldFire(true),	// Нет причин, по которым персонаж не сможет начать стрельбу при первом нажатии кнопки.
	bFireButtonPressed(false),
	bShouldTraceForItems(false) // Переменная для отслеживания предметов.
{
 	/* Установите этот символ для вызова Tick() в 
	каждом кадре. Можно это отключить, чтобы улучшить 
	производительность, если это не нужно. */
	PrimaryActorTick.bCanEverTick = true;

	// Создание CameraBoom(отправляемая к персонажу, если идёт столкновение(к примеру со стеной)).
	// Эта функция получает параметр, тип которого заключёт в <> нужно создать.
	// TEXT("CameraBoom") - задаёт имя этого компонента. 
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));

	// Прикрепление камеры к объекту
	CameraBoom->SetupAttachment(RootComponent); // RootComponent переменная наследуемая от Actor.

	CameraBoom->TargetArmLength = 180.f; // Расстояние камеры до объекта.
	CameraBoom->bUsePawnControlRotation = true; // Чтобы CameraBoom следовала за контроллером, где бы он не вращался, то камера будет следовать за контроллером.
	CameraBoom->SocketOffset = FVector(0.f, 50.f, 70.f); // Смещение для гнезда в конце пружинного компонента, к которому прикреплена камера.


	// Создание следуемой камеры.
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));

	/* Здесь что-то типа первый компонент это где камера, второй компонент - к чему камера 	прикреплена.
	SetupAttachment(Второй компонент дополнительного гнезда, указание к какому объекту должна 	быть присоединена камера) */
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Прикрепить камеру к объекту.

	// Чтобы камера не вращалась относительно spring arm.
	FollowCamera->bUsePawnControlRotation = false;

	// Подтверждение того, что персонаж более не вращается с контроллером(т.е., когда вращаешь мышью, то персонаж поворачивается вместе с мышью).
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	// Настройка движения персонажа.

	GetCharacterMovement()->bOrientRotationToMovement = false; // Персонаж движется в направлении в зависимости ввода.
	// Определяет скорость вращения при ориентации на направление движения
	GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f); // FRotator(Pitch, Yaw, Roll); ... персонаж движет от этого уровня вращения.

	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;
}

// Called when the game starts or when spawned
void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (FollowCamera)
	{
		CameraDefaultFOV = GetFollowCamera()->FieldOfView; // FieldOfView - переменная CameraComponent класса.
		CameraCurrentFOV = CameraDefaultFOV;
	}
}



// Called every frame
void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CameraInterpZoom(DeltaTime); // Содержит интерполяцию для увеличения, когда персонаж прицеливается.

	SetLookRates(); // Менять чувствительность мыши, в зависимости от прицеливания или не прицеливания.
	
	CalculateCrosshairSpread(DeltaTime); // Рассчитать множитель разброса прицела.

	TraceForItems(); // Проверка OverlappedItemCount, затем отследить предметы.
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
			// Когда персонаж может видеть актёра, в перекрестие, необходимо переделать из актёра в Item.
			AItem* HitItem = Cast<AItem>(ItemTraceResult.Actor); // bBlockingHit = true, мы видим актёра которого задели и переделываем его в AItem.
			if (HitItem && HitItem->GetPickupWidget())
			{
				HitItem->GetPickupWidget()->SetVisibility(true); // Показать Pickup виджет для предметов.
			}
			if (TraceHitItemLastFrame) // Соприкосновение с AItem последним кадром.
			{
				if (HitItem != TraceHitItemLastFrame)
				{
					// Мы попадаем в другой AItem в этом кадре из предыдущего кадра или AItem равен null.
					TraceHitItemLastFrame->GetPickupWidget()->SetVisibility(false);
				}
			}
			TraceHitItemLastFrame = HitItem; // Хранит ссылку для HitItem следующего кадра.
		}
	}
	else if (TraceHitItemLastFrame)
	{
		// Отсутствует соприкосновение со сферой какого-либо предмета.
		TraceHitItemLastFrame->GetPickupWidget()->SetVisibility(false); // Кадр последннего предмета не должен показывать виджет.
	}
}

// Called to bind functionality to input
void AShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	check(PlayerInputComponent); // Проверка на валидность.
	
	PlayerInputComponent->BindAxis("MoveForward", this, &AShooterCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AShooterCharacter::MoveRight);
	PlayerInputComponent->BindAxis("TurnRate", this, &AShooterCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AShooterCharacter::LookUpAtRate);

	// &APawn поскольку не скалирует(увеличивает) значение ввода, то будет вызван напрямую.
	PlayerInputComponent->BindAxis("Turn", this, &AShooterCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &AShooterCharacter::LookUp);

	// IE_Pressed - потому что входное событие(input event)
	// &Character::Jump - функция прыжка наследуется от Character класса.
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);

	// Эта функция вызывается, когда кнопка отпускается.
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
	// Подлючение к гнезду BarrelSocket.
	const USkeletalMeshSocket* BarrelSocket = GetMesh()->GetSocketByName("BarrelSocket");
	if (BarrelSocket)
	{
		// FTransform принимает Location, Rotation, Scale данные. Откуда идёт вспышка и по какой оси.
		const FTransform SocketTransform = BarrelSocket->GetSocketTransform(GetMesh());

		if (MuzzleFlash)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, SocketTransform);
		}

		FVector BeamEnd;
		// True - если есть положение конечной точки луча.
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
		FHitResult FireHit; // Результат попадания.
		const FVector Start{ SocketTransform.GetLocation() }; // Возвращается вектор откуда летит пуля.
		const FQuat Rotation{ SocketTransform.GetRotation() }; // FQuat - кватерион, это математическая концепция содержащая информацию связанную(related) с вращением(вращение вокруг некоторой оси в трёхмерном пространстве).
		const FVector RotationAxis{ Rotation.GetAxisX() }; // Получение оси X, из которой вылетает пуля.
		const FVector End{ Start + RotationAxis * 50'000.f }; // FVector End{ Начало + ось вращения * дальность }; Все это даёт данные об окончательной точке линии трассировки.
		
		// Дым при столкновении с объектом.
		FVector BeamEndPoint{ End };
															  
		// LineTraceMulti попадает в конкретный объект и продолжает трассировка, задевая объект позади.
		// Когда выполняется линия трассировки обычно это передаётся в структуру FHitResult, содержащую в себе переменные, которая заполняет трассировка строки по одному каналу.
		GetWorld()->LineTraceSingleByChannel(FireHit, Start, End, ECollisionChannel::ECC_Visibility); // LineTraceSingleByChannel(Сам выстрел, начальная локация(откуда летит пуля), конечная точка, отображении столкновения) // Теперь FireHit имеет информацию хранимую нее смотря на линию трассировки, 

		// Также хранит bool, true - если попали во что-то, false - если не попали ни во что.
		if (FireHit.bBlockingHit)
		{
			// Так как более нет необходимости использовать векторы направления выстрелов и отслеживание точек столкновения, то необходимо закоментировать что ниже:
			// DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 2.f); // DrawDebugLine(мир, начальная точка, конечная точка, цвет линии трассировки, продолжении существования линии, время существования линии);
			// DrawDebugPoint(GetWorld(), FireHit.Location, 5.f, FColor::Red, false, 2.f); // DrawDebugPoint(мир, вектор выстрела, размер точки, цвет, сохранение точки, существование точки);
		
			BeamEndPoint = FireHit.Location; // Местоположение попадания. 

			if (ImpactParticles)
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, FireHit.Location); // SpawnEmitter(мир, система частиц(пули), результат попадания);
			}
		}

		if (BeamParticles)
		{
			UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BeamParticles, SocketTransform);
			if (Beam)
			{
				Beam->SetVectorParameter(("Target"), BeamEndPoint); // Beam->SetVectorParameter(Цель, конечная точка столкновения); FVector BeamEndPoint - Если ни во что не попали, BeamEndPoint = FireHit.Location; - возвращает объект, если произошло столкновение.
			}
		}
		*/
	}

	// Запуск анимации из монтажа.
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HipFireMontage)
	{
		AnimInstance->Montage_Play(HipFireMontage); // Проиграть монтаж.

		/* У экземпляра enum также есть функция, называемая монтажным переходом 
		к разделу, поэтому можно указать раздел, к которому необходимо перейти. */
		AnimInstance->Montage_JumpToSection(FName("StartFire"));
	}

	StartCrosshairBulletFire();
}

void AShooterCharacter::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		/* Эта функция получает вращение контроллера, вне зависимости перед
		чем контроллер, здесь получается FRotator. */
		const FRotator Rotation{ Controller->GetControlRotation() }; // Определяет направление вперёд.
	
		/* YawRotation - является вращением, которое мы получаем от контроллера,
		кроме pitch и roll, они будут обнулены. Yaw-направление - если объект стоит 
		прямо и вращается по кругу и в каком бы направлении ни был, это Yaw. */
		const FRotator YawRotation{ 0, Rotation.Yaw, 0 };

		// Получение вектора соответствующему направлению.
		// GetUnitAxis(EAxis::X) - Получить прямую ось, матрицы, соответствующую вращению. Эта функция получает X-ось направление(GetUnitAxis(EAxis::X)), от вращения матрицы(FRotationMatrix{YawRotation}).
		const FVector Direction{ FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::X) }; // Это создаёт математический конструкт, названный матрицой, характеристикой Yaw-вращения.
		AddMovementInput(Direction, Value); // Получает направление объекта относительно мира.
	}
}

void AShooterCharacter::MoveRight(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// Эта функция получает вращение контроллера, вне зависимости перед чем контроллер, здесь получается FRotator.
		const FRotator Rotation{ Controller->GetControlRotation() }; // Определяет направление вправо.
	
		/* YawRotation - является вращением, которое мы получаем от контроллера,
		кроме pitch и roll,  он будут обнулены. Yaw-направление - если объект стоит 
		прямо и вращается по кругу и в каком бы направлении ни был, это Yaw. */
		const FRotator YawRotation{ 0, Rotation.Yaw, 0 };

		// Получение вектора соответствующему направлению.
		/* GetUnitAxis(EAxis::X) - Получить прямую ось, матрицы, соответствующую вращению.
		Эта функция получает X-ось направление(GetUnitAxis(EAxis::X)), 
		от вращения матрицы(FRotationMatrix{YawRotation}). */
		const FVector Direction{ FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::Y) }; // Это создаёт математический конструкт, названный матрицой, характеристикой Yaw-вращения.
		AddMovementInput(Direction, Value); // Получает направление объекта относительно мира.
	}
}

void AShooterCharacter::LookUpAtRate(float Rate)
{
	// deg/sec * sec/frame, секунды обнуляются и будут получены градусы за кадр.
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AShooterCharacter::TurnAtRate(float Rate)
{
	/* Rate - нормализированное значение между 0 и 1. Изначально значение 0, 
	если нажать на клавиатуре кнопку, то значение сразу будет 1, если использовать 
	геймпад, то в зависимости от давления на стик, значение Rate будет варьироваться от 0 до 1*/

	/* BaseTurnRate умножается на Rate, поскольку это вызывается с каждым кадром, 
	он будет умножен на GetWorld()->GetDeltaSeconds())(по-другому DeltaTime, это 
	значит сколько секунд было передано с каждым кадром). */

	// Базовый уровень вращения градусы за секунду.
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds()); // deg/sec * sec/frame, секунды обнуляются и будут получены градусы за кадр.
}

void AShooterCharacter::CalculateCrosshairSpread(float DeltaTime)
{
	// Получение текущий скорости, для определения диапазона от нуля, до максимальной скорости ходьбы(отображение её значения, чтобы она соответствовала между 0 и 1(к примеру скорость 600, т.е. 300 == 0.5, 450 == 0.75).
	FVector2D WalkSpeedRange{ 0.f, 600.f }; // 2D вектор соответствующий диапазону скорости персонажа.
	FVector2D VelocityMultiplierRange{ 0.f, 1.f };
	FVector Velocity{ GetVelocity() };
	Velocity.Z = 0.f; // Равно нулю, так как, прицел отображается в 2D.

	// GetMappedRangeValueClamped - возвращает соответствующий процентаж, от заданного значения.
	CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(
		WalkSpeedRange, // Вводимое значение от 0 до 600.
		VelocityMultiplierRange, // Выводимое значение, получение скорости 600, но в диапазоне от 0 до 1.
		Velocity.Size());

	// Расчитать, будет ли расширяться перекрестье если персонаж в воздухе.
	if (GetCharacterMovement()->IsFalling()) // Персонаж в воздухе?
	{
		// Расширение перекрестья(медленно), пока персонаж в воздухе.
		CrosshairInAirFactor = FMath::FInterpTo(
			CrosshairInAirFactor,
			2.25f, // Определённое значение, до которого интерполируется перекрестие.
			DeltaTime,
			2.25f); // Скорость расширения(распространения) перекрестья.		
	}
	else // Персонаж на земле.
	{
		// Быстрое скоращение перекрестия, пока персонаж на земле.
		CrosshairInAirFactor = FMath::FInterpTo(
			CrosshairInAirFactor,
			0.f, // Определённое значение, до которого интерполируется перекрестие.
			DeltaTime,
			30.f); // Скорость расширения(распространения) перекрестья.	
	}

	// Рассчитать фактор прицеливания, для перекрестия.
	if (bAiming) // Персонаж прицеливается?
	{
		// Быстрое сокращение перекрестия до меньших размеров.
		CrosshairAimFactor = FMath::FInterpTo(
			CrosshairAimFactor,
			0.6f,
			DeltaTime,
			30.f);
	}
	else // Не прицеливается.
	{
		// Быстрое возвращение перекрестия до исходных размеров.
		CrosshairAimFactor = FMath::FInterpTo(
			CrosshairAimFactor,
			0.f,
			DeltaTime,
			30.f);
	}

	// True 0.05 секунд после начала стрельбы.
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

	// Если значение не используется, то оно равно 0, к примеру, если персонаж не прицеливается, то CrooshairAimFactor = 0, а другие переменные между собой складываются или вычитаются.
	CrosshairSpreadMultiplier = 0.5f + CrosshairVelocityFactor + CrosshairInAirFactor - CrosshairAimFactor + CrosshairShootingFactor; // CrosshairVelocityFactor находится на низком значении, когда персонаж идёт медленно и на высоком, когда бежит.
}

bool AShooterCharacter::GetBeamEndLocation(const FVector& MuzzleSocketLocation, FVector& OutBeamLocation)
{
	FHitResult CrosshairHitResult;
	bool bCrosshairHit = TraceUnderCrosshairs(CrosshairHitResult, OutBeamLocation); // Проверить путь попадания для перекрестия.
	
	if (bCrosshairHit)
	{
		OutBeamLocation = CrosshairHitResult.Location; // Ориентировочное расположение луча(всё ещё необходимо проследить за оружием).
	}
	else // Отсутствие цели на пути перекрестья.
	{
		// OutBeamLocation - конечное местоположения трассировки луча. Если попадания не было, местоположение луча будет обновлено до конца этой первой трассы(прямо из прицела на 50'000 единиц).
	}

	// Выполнение второй трассировки(ствола орудия)
	FHitResult WeaponTraceHit;
	const FVector WeaponTraceStart{ MuzzleSocketLocation };
	const FVector StartToEnd{ OutBeamLocation - WeaponTraceStart };
	// Так как второй выстрел, меньше чем 50'000 единиц, а след линии не доходит до стены.
	const FVector WeaponTraceEnd{ MuzzleSocketLocation + StartToEnd * 1.25f }; // Делает линию трассировки на 25% длиннее, вместо того чтобы останавливаться на результате попадания, пуля проходит дальше.
	
	GetWorld()->LineTraceSingleByChannel(
		WeaponTraceHit, // Результат выстрела.
		WeaponTraceStart, // Начало(локация гнезда).
		WeaponTraceEnd,	// Конец(конечная точка луча).
		ECollisionChannel::ECC_Visibility); // Видимость столкновения.

	if (WeaponTraceHit.bBlockingHit) // Объект между стволом и конечной точкой луча(BeamEndPoint) ?
	{
		OutBeamLocation = WeaponTraceHit.Location; // Снаряды, которые появляются используют конечную точку луча(BeamEndPoint).
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
	// Нажата ли кнопка прицеливания?
	// Установить текущее поле зрения камеры.
	if (bAiming)
	{
		// Увеличить поле зрения.
		CameraCurrentFOV = FMath::FInterpTo(
			CameraCurrentFOV, // Текущее значение.
			CameraZoomedFOV, // Целевое значение
			DeltaTime, // Дельта тайм,
			ZoomInterpSpeed); // Скорость увеличения.
	}
	else
	{
		// Поле зрения по умолчанию.
		CameraCurrentFOV = FMath::FInterpTo(
			CameraCurrentFOV, // Текущее значение.
			CameraDefaultFOV, // Целевое значение
			DeltaTime, // Дельта тайм,
			ZoomInterpSpeed); // Скорость увеличения.
	}
	// Установить текущее значение следуемой камеры.
	GetFollowCamera()->SetFieldOfView(CameraCurrentFOV);
}

// Yaw - вращение(ось X).
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

// Pitch - вверх/вниз(ось Y)
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

// Каждый раз, когда происходит стрельба из оружия, вызывается эта функция, которая запускает таймер, по истечении которой, вызывается функция FinishCrosshairBulletFire.
void AShooterCharacter::StartCrosshairBulletFire()
{
	bFiringBullet = true; // True, когда персонаж стреляет.

	GetWorldTimerManager().SetTimer(
		CrosshairShootTimer,
		this, // Класс пользователя.
		&AShooterCharacter::FinishCrosshairBulletFire, // Адрес функции.
		ShootTimeDuration);// Продолжительность стрельбы.
		// Последний параметр не используется, если пользователь хочет зациклить таймер.	
}

void AShooterCharacter::FinishCrosshairBulletFire()
{
	bFiringBullet = false; // Cтановится false, по истетечении 0.5, после прекращения стрельбы.
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
			AutoFireTimer,	// Объект.
			this,		// Вызывающий класс.
			&AShooterCharacter::AutoFireReset, // Функция, которая будет использоваться.
			AutomaticFireRate);	//Время задержки.
	}
}

void AShooterCharacter::AutoFireReset()
{
	bShouldFire = true;
	if (bFireButtonPressed) // Проверяет, продолжает ли персонаж стрелять.
	{
		StartFireTimer();
	}
}

bool AShooterCharacter::TraceUnderCrosshairs(FHitResult& OutHitResult, FVector& OutHitLocation)
{
	// Получить размер поля зрения.
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	// Получение местоположения Crosshair на экране.
	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f); // Местоположение перекрестия, инициализируется его X и Y, будут размером окна просмотра, которое X, Y делится на 2.f.
	FVector CrosshairWorldPosition; // Получение позиции.
	FVector CrosshairWorldDirection; // Получение направления.

	// Если внутри этого bool всё получено и выполнено успешно.
	// Получение мировой позиции перекрестья.
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0), // GetPlayerController - возвращает контроллер для Pawn(объект вызывающий, индекс игрока(т.к. один игрок в мире)
		CrosshairLocation, // Местоположение перекрестья
		CrosshairWorldPosition,
		CrosshairWorldDirection);

	if (bScreenToWorld)
	{
		// След от местоположения мира перекрестия перенаружу.
		const FVector Start{ CrosshairWorldPosition };
		const FVector End{ Start + CrosshairWorldDirection * 50'000.f };
		OutHitLocation = End;
		GetWorld()->LineTraceSingleByChannel(
			OutHitResult, // Результат true/false, содержит информацию зоне видимости(видит ли объект).
			Start,
			End,
			ECollisionChannel::ECC_Visibility); // Видимость.
		if (OutHitResult.bBlockingHit)
		{
			// Вне зависимости будет ли попадание или нет, это данные об этом будут чем-то заполнены, ччтобы использовать для луча и местоположения.
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
